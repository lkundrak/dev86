
#include "monitor.h"

#ifndef MINI_BUF

/* #define DEBUG 1 /**/ 

int disk_drive = 0;
int disk_spt   = 7;
int disk_heads = 0;
int disk_cyls  = 0;
long disk_partition_offset = 0;

int check_motor = 1;

static int    last_drive = -1;
static int    data_len = 0;
static long   data_trk1 = 0;
static char * data_buf1 = 0;
static long   data_trk2 = 0;
static char * data_buf2 = 0;

static long   bad_track = -1;	/* Track number of last unsuccesful read */

static long   get_dpt();

void reset_disk()
{
#ifdef DEBUG
   fprintf(stderr, "Reset Disk: ");
#endif
   if( data_buf1 ) free(data_buf1);
   if( data_buf2 ) free(data_buf2);
   data_buf1 = data_buf2 = 0;
   last_drive = disk_drive;
   bad_track = -1;

   disk_spt   = 7;	/* Defaults for reading floppy boot area. */
   disk_heads = 0;
   disk_cyls  = 0;

   if( !(disk_drive & 0x80 ) )
   {
#ifdef __STANDALONE__
      if( disk_drive == __argr.h.dl && __argr.x.si >= 9 && __argr.x.si <= 63 )
      {
	 disk_spt = __argr.x.si;
	 disk_heads = 2;
	 disk_cyls = 80;
      }
#endif

      /* Floppy -> no partitions */
      /* Even if there were we couldn't deal with it anyway! */
      disk_partition_offset = 0;
   }
#if defined(__MSDOS__) || defined(__STANDALONE__)
   else
   {
      /* Hard disk, get parameters from bios */
      long dpt;
      int  v;

#ifdef __STANDALONE__
      if( disk_partition_offset == 0 && disk_drive == __argr.h.dl )
      {
         disk_partition_offset = __argr.x.cx + ((long)__argr.h.dh<<16);
      }
#endif

      dpt = get_dpt(disk_drive);
      v = ((dpt>>16) & 0xFF);
      if( v != 0xFF && v > (disk_drive&0x7F) )
      {
         disk_spt   = (dpt & 0x3F); 		/* Max sector number 1-63 */
         if( disk_spt == 0 ) disk_spt = 64;	/* 1-64 ? */
         disk_heads = ((dpt>>24) & 0xFF) + 1;	/* Head count 1-256 */
         disk_cyls  = ((dpt>>8) & 0xFF) + ((dpt<<2) & 0x300) + 1;

         /* Cyls count, unchecked, only needs != 0, if AMI 386 bios can be
          * upto 4096 cylinder, otherwise BIOS limit is 1024 cyl.
          */
      }
   }
#endif

#ifdef DEBUG
   fprintf(stderr, "%d/%d/%d\n", disk_spt, disk_heads, disk_cyls);
#endif
}

char * read_lsector(sectno)
long sectno;
{
   int rv;

   int phy_s = 1;
   int phy_h = 0;
   int phy_c = 0;

   rv = 0;
   if( disk_drive != last_drive )  rv = 1;
   else if( check_motor )          rv = ( motor_running() == 0 );
   else if( sectno == 0 )          rv = 1;
   if( rv ) reset_disk();

   if( disk_partition_offset > 0 )
      sectno += disk_partition_offset;

   if( disk_spt < 0 || disk_spt > 63 || disk_heads < 1 )
   {
      phy_s = sectno;
#if DEBUG > 1
      fprintf(stderr, "read_sector(%ld = %d,%d,%d)\n",
                                   sectno, phy_c, phy_h, phy_s+1);
#endif
   }
   else
   {
      phy_s = sectno%disk_spt;
      phy_h = sectno/disk_spt%disk_heads;
      phy_c = sectno/disk_spt/disk_heads;

#if DEBUG > 1
      fprintf(stderr, "read_sector(%ld = %d,%d,%d)\n",
                                   sectno, phy_c, phy_h, phy_s+1);
#endif

      if( fetch_track_buf(phy_c, phy_h, phy_s) >= 0 )
         return data_buf1 + (phy_s % data_len) * 512;
   }

   data_len = -1;	/* Zap the cache */
   if( data_buf1 == 0 )
      data_buf1 = malloc(512);
   if( data_buf1 == 0 )
   {
      printf("Cannot allocate memory for disk read!!!\n");
      return 0;
   }

#ifdef DEBUG
     fprintf(stderr, "phy_read(%d,%d,%d,%d,%d,0x%x)\n",
	   disk_drive, phy_c, phy_h, phy_s+1, 1, data_buf1);
#endif

   do
   {
     int v,tries = 6;
     do
     {
       rv = phy_read(disk_drive, phy_c, phy_h, phy_s+1, 1, data_buf1);
       tries--;
     }
     while(rv && tries > 0);
     if( rv )
     {
       printf("Disk error 0x%02x %d:%d:%d:%d[%2d] -> 0x%04x[]\nRetry:",
		      rv, disk_drive, phy_c, phy_h, phy_s+1, 1, data_buf1);
       fflush(stdout);

       v = phy_reset(disk_drive);
       v = (getch()&0x7F);
       printf("\n");
       if( v == 3 || v == 27 ) break;
     }
   }
   while(rv);

   check_motor = motor_running();

   if(rv) return 0; else return data_buf1;
}

fetch_track_buf(phy_c, phy_h, phy_s)
int phy_c, phy_h, phy_s;
{
   long trk_no, t;
   char * p;
   int tries = 6;
   int rv, nlen;

   /* Big tracks get us short of memory so limit it. */
   nlen = (disk_spt-1)/24;
   nlen = (disk_spt+nlen)/(nlen+1);
   /*
   trk_no = (long)phy_c*disk_heads*4+phy_h*4+phy_s/nlen+1;
   */

   trk_no = (long)(phy_c*disk_heads+phy_h)*((disk_spt+4)/nlen)+phy_s/nlen+1;

#if DEBUG > 2
   fprintf(stderr, "Info len=%d,%d trk=%ld,%ld,%ld\n", 
	 data_len,nlen, trk_no,data_trk1,data_trk2);
#endif

   if( data_len != nlen )
   {
      if( data_buf1 ) free(data_buf1);
      if( data_buf2 ) free(data_buf2);
      data_buf1 = data_buf2 = 0;
      data_len = nlen;
   }
   if( trk_no == bad_track ) return -1;

   if( data_buf1 && trk_no == data_trk1 ) return 0;

   /* Two cases:
    * 1) buffer2 has the one we want, need to swap to make it most recent 
    * 2) Neither has it, need to swap to overwrite least recent.
    */

   /* But sequential reads may spoil this so don't swap if we are shifting
    * to the next track.
    */
   
   if( trk_no == data_trk2 || trk_no != data_trk1 + 1 )
   {
      p = data_buf1; data_buf1 = data_buf2; data_buf2 = p;
      t = data_trk1; data_trk1 = data_trk2; data_trk2 = t;
   }

   /* The other one right ? */
   if( data_buf1 && trk_no == data_trk1 ) return 0;

   /* If we get here we have to do a physical read ... */
   /*   into data_buf1. */

   if( data_buf1 == 0 )
   {
      data_buf1 = malloc(disk_spt*512);

#ifdef DEBUG
      if( data_buf1 )
         fprintf(stderr, "Allocated buffer to %d\n", data_buf1);
      else
         fprintf(stderr, "Failed to allocated buffer.\n");
#endif
   }
   if( data_buf1 == 0 )
   {
      /* Is buf2 allocated ? Yes take it! */
      data_buf1 = data_buf2; data_buf2 = 0; data_trk2 = -1;
   }

   data_trk1 = -1;

   /* Not enough memory for track read. */
   if( data_buf1 == 0 ) return -1;

#ifdef DEBUG
     fprintf(stderr, "phy_read(%d,%d,%d,%d,%d,0x%x)\n",
	   disk_drive, phy_c, phy_h, phy_s/data_len*data_len+1, data_len, data_buf1);
#endif

   do /* the physical read */
   {
     rv = phy_read(disk_drive, phy_c, phy_h, phy_s/data_len*data_len+1, data_len,
                   data_buf1);
     tries--;
   }
   while(rv && tries > 0);
   if( rv ) printf("Disk error 0x%02x %d:%d:%d:%d[%2d] -> 0x%04x[]\n",
	  rv, disk_drive, phy_c, phy_h, phy_s/data_len+1, data_len, data_buf1);

   check_motor = motor_running();

   /* Disk error, it'll try one at a time, _very_ slowly! */
   if(rv)
   {
      bad_track = trk_no;
      return -1;
   }

   /* Yes! */
   data_trk1 = trk_no;
   return 0;
}

#if defined(__MSDOS__) || defined(__STANDALONE__)
phy_read(drive, cyl, head, sect, length, buffer)
{
#asm
  push	bp
  mov	bp,sp

  push	es
  push	ds
  pop	es

  mov	dl,[bp+2+_phy_read.drive]
  mov	ch,[bp+2+_phy_read.cyl]
  mov	dh,[bp+2+_phy_read.head]
  mov	bx,[bp+2+_phy_read.buffer]

  mov	ax,[bp+2+_phy_read.cyl]		! Bits 10-11 of cylinder, AMI BIOS.
  mov	cl,#4
  sar	ax,cl
  and	al,#$C0
  xor	dh,al

  mov	cl,[bp+2+_phy_read.sect]
  and	cl,#$3F
  mov	ax,[bp+2+_phy_read.cyl]		! Bits 8-9 of cylinder.
  sar	ax,#1
  sar	ax,#1
  and	al,#$C0
  or	cl,al

  mov	al,[bp+2+_phy_read.length]
  mov	ah,#$02
  int	$13
  jc	read_err
  mov	ax,#0
read_err:
  xchg	ah,al
  xor	ah,ah

  pop	es
  pop	bp
#endasm
}

long
get_dpt(drive)
{
#asm
  push	bp
  mov	bp,sp

  push	di
  push	es

  mov	dl,[bp+2+_get_dpt.drive]

  mov	ah,#$08
  int	$13
  jnc	func_ok
  mov	cx,ax
  mov	dx,#-1
func_ok:
  mov	ax,cx

  pop	es
  pop	di
  pop	bp
#endasm
}

phy_reset(drive)
{
#asm
  push	bp
  mov	bp,sp

  push	di
  push	es

  mov	dl,[bp+2+_phy_reset.drive]

  mov	ah,#$08
  int	$13
  jnc	reset_ok
  mov	cx,ax
  mov	dx,#-1
reset_ok:
  mov	ax,cx

  pop	es
  pop	di
  pop	bp
#endasm
}

motor_running()
{
#asm
  push	es
  mov	ax,#$40
  mov	es,ax
  seg	es
  mov	al,[$3f]
  xor	ah,ah
  and	al,#$0F
  pop	es
#endasm
}
#endif

#endif
