/*
 * rawio.c - plagiarised from ../../bootblocks/trk_buf.c 
 */

#ifdef DEBUG
#include <stdio.h>
#endif

#include <bios.h>
#include <ctype.h>
#include <malloc.h>
#include "rawio.h"

int rawio_disk_drive = 0;
int rawio_disk_spt   = 7;
int rawio_disk_heads = 2;
int rawio_disk_cyls  = 0;

static int    last_drive = 0;
static int    data_len = 0;
static long   data_trk1 = 0;
static char * data_buf1 = 0;
static long   data_trk2 = 0;
static char * data_buf2 = 0;

static long   bad_track = -1;	/* Track number of last unsuccesful read */

static long   rawio_get_dpt();

void rawio_reset_disk()
{
   if( data_buf1 ) free(data_buf1);
   if( data_buf2 ) free(data_buf2);
   data_buf1 = data_buf2 = 0;
   last_drive = rawio_disk_drive;

   if( !(rawio_disk_drive & 0x80 ) )
   {
      rawio_disk_spt   = 7;	/* Defaults for reading Boot area. */
      rawio_disk_heads = 2;
      rawio_disk_cyls  = 0;
#ifdef DEBUG
      fprintf(stderr, "reset_disk (hard): spt = %d, heads = %d, cyls = %d\n",
           rawio_disk_spt,
           rawio_disk_heads,
           rawio_disk_cyls);
#endif
   }
#if defined(__MSDOS__) || defined(__STANDALONE__)
   else
   {
      /* Hard disk, get parameters from bios */
      long dpt;
      int  v;

      rawio_disk_spt   = 17;	/* Defaults for reading Boot area. */
      rawio_disk_heads = 1;
      rawio_disk_cyls  = 0;

      dpt = rawio_get_dpt(rawio_disk_drive);
      v = ((dpt>>16) & 0xFF);
      if( v == 0xFF || v <= (rawio_disk_drive&0x7F) ) return; /* Bad dpt */

      rawio_disk_spt   = (dpt & 0x3F); 		/* Max sector number 1-63 */
      if( rawio_disk_spt == 0 ) rawio_disk_spt = 64;	/* 1-64 ? */
      rawio_disk_heads = ((dpt>>24) & 0xFF) + 1;	/* Head count 1-256 */
      rawio_disk_cyls  = ((dpt>>8) & 0xFF) + ((dpt<<2) & 0x300) + 1;

      /* Cyls count, unchecked, only needs != 0, if AMI 386 bios can be
       * upto 4096 cylinder, otherwise BIOS limit is 1024 cyl.
       */
#ifdef DEBUG
      fprintf(stderr, "reset_disk (soft): spt = %d, heads = %d, cyls = %d\n",
           rawio_disk_spt,
           rawio_disk_heads,
           rawio_disk_cyls);
#endif
   }
#endif
}

int rawio_read_lsector(sectno, buffer)
long sectno;
char* buffer;
{
   int tries = 6;
   int rv;

   int phy_s = 1;
   int phy_h = 0;
   int phy_c = 0;

   if( rawio_disk_drive != last_drive ) rawio_reset_disk();

   if( rawio_disk_spt < 0 || rawio_disk_spt > 63 || rawio_disk_heads < 1 )
   {
      phy_s = sectno;
      rawio_reset_disk();

#ifdef DEBUG
      fprintf(stderr, "read_sector(%ld = %d,%d,%d)\n",
                                   sectno, phy_c, phy_h, phy_s+1);
#endif
   }
   else
   {
      phy_s = sectno%rawio_disk_spt;
      phy_h = sectno/rawio_disk_spt%rawio_disk_heads;
      phy_c = sectno/rawio_disk_spt/rawio_disk_heads;

#ifdef DEBUG
      fprintf(stderr, "read_sector(%ld = %d,%d,%d)\n",
                                   sectno, phy_c, phy_h, phy_s+1);
#endif
      if( rawio_fetch_track_buf(phy_c, phy_h, phy_s) >= 0 )
      {
#ifdef DEBUG
         fprintf(stderr, "read_sector: ftb worked\n");
#endif
         memcpy(buffer, data_buf1 + (phy_s % data_len) * 512, 512);
         return 512;
      }
   }

   data_len = -1;	/* Zap the cache */
   if( data_buf1 == 0 )
      data_buf1 = malloc(512);
   if( data_buf1 == 0 )
   {
#ifdef DEBUG
      fprintf(stderr, "Cannot allocate memory for disk read!!!\n");
#endif
      return 0;
   }

#ifdef DEBUG
   fprintf(stderr, "WARNING: Single sector read\n");
#endif

   do
   {
     rv = rawio_phy_read(rawio_disk_drive, phy_c, phy_h, phy_s+1, 1, data_buf1);
     tries--;
#ifdef DEBUG
     if( rv ) fprintf(stderr, "Error in phy_read(%d,%d,%d,%d,%d,%d);\n",
		      rawio_disk_drive, phy_c, phy_h, phy_s+1, 1, data_buf1);
#endif
   }
   while(rv && tries > 0);

   if(rv)
   {
#ifdef DEBUG
      fprintf(stderr, "rawio failed\n");
#endif
      return 0;
   }
   else
   {
#ifdef DEBUG
      fprintf(stderr, "rawio worked\n");
#endif
      memcpy(buffer, data_buf1, 512);
      return 512;
   }
}

rawio_fetch_track_buf(phy_c, phy_h, phy_s)
int phy_c, phy_h, phy_s;
{
   long trk_no, t;
   char * p;
   int tries = 3;
   int rv, nlen;

   /* Big tracks get us short of memory so limit it. */
   nlen = (rawio_disk_spt-1)/22;
   nlen = (rawio_disk_spt+nlen)/(nlen+1);
   trk_no = (long)phy_c*rawio_disk_heads*4+phy_h*4+phy_s/nlen+1;

   if( data_len != nlen )
   {
      if( data_buf1 ) free(data_buf1);
      if( data_buf2 ) free(data_buf2);
      data_buf1 = data_buf2 = 0;
      data_len = rawio_disk_spt;
   }
#ifdef DEBUG
   fprintf(stderr,
           "ftb: trk_no=%ld, data_trk1=%ld, data_buf1=%x\n",
           trk_no,
           data_trk1,
           data_buf1);
#endif
   if( trk_no == bad_track ) return -1;

   if( data_buf1 && trk_no == data_trk1 ) return 0;

   /* Two cases:
    * 1) buffer2 has the one we want, need to swap to make it most recent 
    * 2) Neither has it, need to swap to overwrite least recent.
    */

   /* So we always swap */
   p = data_buf1; data_buf1 = data_buf2; data_buf2 = p;
   t = data_trk1; data_trk1 = data_trk2; data_trk2 = t;

#ifdef DEBUG
   fprintf(stderr, "ftb swap: trk_no=%ld, data_trk1=%ld, data_buf1=%x\n",
           trk_no,
           data_trk1,
           data_buf1);
#endif
   /* The other one right ? */
   if( data_buf1 && trk_no == data_trk1 ) return 0;

   /* If we get here we have to do a physical read ... */
   /*   into data_buf1. */

   if( data_buf1 == 0 )
   {
      data_buf1 = malloc(rawio_disk_spt*512);

#ifdef __ELKS__
      fprintf(stderr, "Allocated buffer to %d\n", data_buf1);
#endif
   }
   if( data_buf1 == 0 )
   {
      /* Is buf2 allocated ? Yes take it! */
      data_buf1 = data_buf2; data_buf2 = 0; data_trk2 = -1;
   }

   bad_track = -1;
   data_trk1 = -1;

#ifdef DEBUG
   fprintf(stderr, "ftb buf: trk_no=%ld, data_trk1=%ld, data_buf1=%x\n",
           trk_no,
           data_trk1,
           data_buf1);
#endif
   /* Not enough memory for track read. */
   if( data_buf1 == 0 ) return -1;

   do /* the physical read */
   {
     rv = rawio_phy_read(rawio_disk_drive, phy_c, phy_h, phy_s/data_len+1, data_len,
                   data_buf1);
     tries--;
#ifdef DEBUG
     if( rv ) fprintf(stderr, "Error in phy_read(%d,%d,%d,%d,%d,%d);\n",
	     rawio_disk_drive, phy_c, phy_h, phy_s/data_len+1, data_len, data_buf1);
#endif
   }
   while(rv && tries > 0);

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
rawio_phy_read(drive, cyl, head, sect, length, buffer)
{
#asm
  push	bp
  mov	bp,sp

  push	es
  push	ds
  pop	es

  mov	dl,[bp+2+_rawio_phy_read.drive]
  mov	ch,[bp+2+_rawio_phy_read.cyl]
  mov	dh,[bp+2+_rawio_phy_read.head]
  mov	bx,[bp+2+_rawio_phy_read.buffer]

  mov	ax,[bp+2+_rawio_phy_read.cyl]		! Bits 10-11 of cylinder, AMI BIOS.
  mov	cl,#4
  sar	ax,cl
  and	al,#$C0
  xor	dh,al

  mov	cl,[bp+2+_rawio_phy_read.sect]
  and	cl,#$3F
  mov	ax,[bp+2+_rawio_phy_read.cyl]		! Bits 8-9 of cylinder.
  sar	ax,#1
  sar	ax,#1
  and	al,#$C0
  or	cl,al

  mov	al,[bp+2+_rawio_phy_read.length]
  mov	ah,#$02
  int	$13
  jc	read_err
  mov	ax,#0
read_err:

  pop	es
  pop	bp
#endasm
}

long
rawio_get_dpt(drive)
{
#asm
  push	bp
  mov	bp,sp

  push	di
  push	es

  mov	dl,[bp+2+_rawio_get_dpt.drive]

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
#endif

