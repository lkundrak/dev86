
#include "monitor.h"

#ifndef MAXTRK
#define MAXTRK	18
#endif

int disk_drive = 0;
int disk_spt   = 7;
int disk_heads = 0;
int disk_cyls  = 0;
long disk_partition_offset = 0;

static long bad_start = -1;
static long buf_start = -1;
static int  buf_sec = 0;
static int  buf_len = 0;
static char buffer[MAXTRK*512];	/* WARNING: This must be DMAable */

void reset_disk()
{
   disk_spt   = 7;	/* Defaults for reading floppy boot area. */
   disk_heads = 0;
   disk_cyls  = 0;
   bad_start  = -1;
   disk_partition_offset = 0;

#ifdef __STANDALONE__
   if( disk_drive == __argr.h.dl && __argr.x.si >= 9 && __argr.x.si <= 63 )
   {
      disk_spt = __argr.x.si;
      disk_heads = 2;
      disk_cyls = 80;
   }
   if( disk_drive & 0x80 )
   {
      /* Hard disk, get parameters from bios */
      long dpt;
      int  v;

      if( disk_drive == __argr.h.dl )
         disk_partition_offset = __argr.x.cx + ((long)__argr.h.dh<<16);

      dpt = _bios_get_dpt(disk_drive);
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
}

char * read_lsector(sectno)
long sectno;
{
   int tries;
   int rv = 0;

   int phy_s = 1;
   int phy_h = 0;
   int phy_c = 0; 

   long bstart;

   if( sectno == 0 || disk_heads == 0 ) reset_disk();
   if( disk_partition_offset > 0 ) sectno += disk_partition_offset;

   if( disk_spt < 1 || disk_heads < 1 )
      phy_s = sectno;
   else
   {
      phy_s = sectno%disk_spt;
      phy_h = sectno/disk_spt%disk_heads;
      phy_c = sectno/disk_spt/disk_heads;

      bstart = (long)phy_c*disk_heads+phy_h;
      if (disk_spt > MAXTRK) {
	 bstart = bstart * (disk_spt+MAXTRK-1)/MAXTRK;
	 buf_sec = phy_s/MAXTRK;
	 bstart = bstart + buf_sec;
	 buf_sec *= MAXTRK;

	 if (disk_spt > buf_sec+MAXTRK) buf_len = MAXTRK;
	 else                           buf_len = disk_spt-buf_sec;
      } else {
	 buf_sec = 0;
	 buf_len = disk_spt;
      }

      if( bstart != buf_start && bstart != bad_start && buf_len > 1 )
      {
	 rv = _bios_disk_read(disk_drive,phy_c,phy_h,buf_sec+1,buf_len,buffer);
	 if( rv == 0 )
	    buf_start = bstart;
	 else {
	    bad_start = bstart;
	    buf_start = -1;
	 }
#ifdef DEBUG
	 printf("Track read %d,%d,%d,%d,%d,%d -> %d\n",
		disk_drive,phy_c,phy_h,buf_sec+1,buf_len,buffer, rv);
#endif
      }

      if( bstart == buf_start )
	 return buffer + (phy_s-buf_sec) * 512;
   }

   tries = 6;
   do
   {
      if( rv && tries<0) {
	 int v;
	 printf("Disk error 0x%02x %d:%d:%d:%d[%2d] -> 0x%04x[]\n",
	               rv, disk_drive, phy_c, phy_h, phy_s+1, 1, buffer);
	 printf("Retry ?"); v = (getch()&0x7F); printf("\n");
	 if (v == 3 || v == 27 || v == 'n' || v == 'N')
	    return 0;
	 tries = 6;
      }

      if (tries == 3) _bios_disk_reset(disk_drive);

      rv = _bios_disk_read(disk_drive, phy_c, phy_h, phy_s+1, 1, buffer);
      tries--;
   }
   while(rv);
#ifdef DEBUG
   printf("Sector read %d,%d,%d,%d,%d,%d -> %d\n",
	   disk_drive,phy_c,phy_h,phy_s+1,1,buffer, rv);
#endif

   if(rv) return 0; else return buffer;
}
