
#ifdef __STANDALONE__

#include <ctype.h>
#include <malloc.h>
#include "readfs.h"

#define DOS_SECT(P)        get_uint(P,0x0B)
#define DOS_CLUST(P)       get_byte(P,0x0D)
#define DOS_RESV(P)        get_uint(P,0x0E)
#define DOS_NFAT(P)        get_byte(P,0x10)
#define DOS_NROOT(P)       get_uint(P,0x11)
#define DOS_MAXSECT(P)     get_uint(P,0x13)
#define DOS_MEDIA(P)       get_byte(P,0x15)
#define DOS_FATLEN(P)      get_uint(P,0x16)
#define DOS_SPT(P)         get_uint(P,0x18)
#define DOS_HEADS(P)       get_uint(P,0x1A)
#define DOS_HIDDEN(P)      get_long(P,0x1C)
#define DOS4_MAXSECT(P)    get_long(P,0x20)
#define DOS4_PHY_DRIVE(P)  get_byte(P,0x24)
#define DOS4_SERIAL(P)     get_long(P,0x27)

/* These assume alignment is not a problem */
#define get_byte(P,Off)	   *((unsigned char*)((char*)(P)+(Off)))
#define get_uint(P,Off)	   *((unsigned short*)((char*)(P)+(Off)))
#define get_long(P,Off)	   *((long*)((char*)(P)+(Off)))

static int alloc_trackbuf();
static char * read_sector();
static int read_bootblock();

int disk_drive = 0;

static int track_number = -1;
static char * track_buffer = 0;
static int track_len = 0;
static int disk_spt = -1;
static int disk_heads = 2;

static int dir_nentry, dir_sect;
static int dos_clust0, dos_spc;

static char * fat_buf = 0;

struct filestatus {
   char fname[12];
   unsigned short first_cluster;
   unsigned short cur_cluster;
   unsigned short sector_no;
   long file_length;
}
   cur_file = { "", 0, 0, 0 };

open_file(fname)
char * fname;
{
   extern union REGS __argr;
   char conv_name[12];
   char *s, *d;
   int i;
   int dodir = 0;

   if(strcmp(fname, ".") == 0) dodir = 1;
   else
   {
      /* Convert the name to MSDOS directory format */
      strcpy(conv_name, "        ");
      for(s=fname, d=conv_name; *s && *d && *s != '.' && *s != ' '; s++)
      {
	 if( islower(*s) ) *d++ = toupper(*s);
	 else              *d++ = *s;
      }
      while( *s && *s != '.' ) s++;
      strcpy(d=(conv_name+8), "   ");
      if( *s == '.' )
      {
	 for(s++; *s && *d; s++)
	 {
	    if( islower(*s) ) *d++ = toupper(*s);
	    else              *d++ = *s;
	 }
      }

      /* Already opened ? Then just rewind it */
      if( cur_file.first_cluster && strcmp(cur_file.fname, conv_name) == 0 )
	 return rewind_file();
   }

   memset(&cur_file, '\0', sizeof(cur_file));

   /* Get the superblock */
   s = read_sector(0);
   if( s == 0 ) return -1;

   /* Collect important data */
   dir_sect = DOS_RESV(s) + DOS_NFAT(s)*DOS_FATLEN(s);
   dir_nentry = DOS_NROOT(s);

   dos_spc = DOS_CLUST(s);
   if( dos_spc < 1 ) dos_spc = 1;
   dos_clust0 = dir_sect + (dir_nentry+15)/16 - 2*dos_spc;

   if( !dodir )
   {
      /* Read in and buffer the FAT */
      if( fat_buf ) free(fat_buf);
      fat_buf = malloc(DOS_FATLEN(s) * 512);
      if( fat_buf == 0 ) return -1;
      else
      {
	 int fatsec = DOS_RESV(s);
	 int nsec = DOS_FATLEN(s);

	 for(i=0; i<nsec; i++)
	 {
	    s = read_sector(fatsec+i);
	    if(s == 0) return -1;
	    memcpy(fat_buf+i*512, s, 512);
	 }
      }
   }

   /* Scan the root directory for the file */
   for(i=0; i<dir_nentry; i++)
   {
      s = read_sector(dir_sect+i/16);
      if( s == 0 ) return -1;
      d = s + (i%16)*32;
      if( dodir )
      {
         char dtime[20];
	 sprintf(dtime, " %02d/%02d/%04d %02d:%02d",
	           (get_uint(d,24)&0x1F),
	           ((get_uint(d,24)>>5)&0xF),
	           ((get_uint(d,24)>>9)&0x7F)+1980,
	           ((get_uint(d,22)>>11)&0x1F),
	           ((get_uint(d,22)>>5)&0x3F)
	        );
	 if( *d > ' ' && *d <= '~' ) switch(d[11]&0x18)
	 {
	 case 0:
            printf("%-8.8s %-3.3s %10ld%s\n", d, d+8, get_long(d,28), dtime);
	    break;
	 case 0x10:
            printf("%-8.8s %-3.3s <DIR>     %s\n", d, d+8, dtime);
	    break;
	 case 8:
            if( (d[11] & 7) == 0 )
	       printf("%-11.11s  <LBL>     %s\n", d, dtime);
	    break;
	 }
      }
      else if( memcmp(d, conv_name, 11) == 0 && (d[11]&0x18) == 0 )
      { /* Name matches and is normal file */

         strcpy(cur_file.fname, conv_name);
	 cur_file.first_cluster = get_uint(d,26);
	 cur_file.file_length =   get_long(d,28);

	 cur_file.cur_cluster = cur_file.first_cluster;
	 cur_file.sector_no = 0;

	 return 0;
      }
   }
   return -1;
}

rewind_file()
{
   /* Is there an opened file ? */
   if( cur_file.fname[0] == 0 ) return -1;

   cur_file.sector_no = 0;
   cur_file.cur_cluster = cur_file.first_cluster;
   return 0;
}

close_file()
{
   if( fat_buf ) free(fat_buf);
   if( track_buffer ) free(track_buffer);
   memset(&cur_file, '\0', sizeof(cur_file));
   fat_buf = 0;
   track_buffer = 0;
   track_len = 0;
   track_number = -1;
   disk_spt = -1;
   disk_heads = 2;
   return 0;
}

long
file_length()
{
   /* Is there an opened file ? */
   if( cur_file.fname[0] == 0 ) return -1;
   
   return cur_file.file_length;
}

read_block(buffer)
char * buffer;
{
   int s;
   char * ptr;

   /* Is there an opened file ? */
   if( cur_file.fname[0] == 0 ) return -1;

   /* Are we before the EOF ? NB: FAT12 ONLY! */
   if( cur_file.cur_cluster >= 0xFF0 || cur_file.cur_cluster < 2 ) return -1;

   for(s=0; s<2; s++)
   {
      unsigned int sectno;

      if( cur_file.cur_cluster >= 0xFF0 || cur_file.cur_cluster < 2 )
      {
         memset(buffer, '\0', 512);
	 buffer += 512;
	 continue;
      }

      sectno = dos_clust0
             + cur_file.cur_cluster * dos_spc
	     + cur_file.sector_no % dos_spc;

      ptr = read_sector(sectno);
      if( ptr == 0 ) return -1;
      memcpy(buffer, ptr, 512);

      cur_file.sector_no++;
      if( cur_file.sector_no % dos_spc == 0 )
      {
         int odd = (cur_file.cur_cluster&1);
	 unsigned int val;

	 val = cur_file.cur_cluster + (cur_file.cur_cluster>>1);
	 val = get_uint(fat_buf, val);

	 if( odd ) val>>=4;

	 val &= 0xFFF;

         cur_file.cur_cluster = val;
      }

      buffer += 512;
   }

   return 0;
}

static char * read_sector(sectno)
int sectno;
{
   int track_no, track_off, track_start, linsect;
   if( disk_spt == -1 && read_bootblock() < 0 ) return 0;

   track_no = sectno / track_len;
   track_off= sectno % track_len;

   if( track_no != track_number )
   {
      track_number = -1;
      track_start = track_no * track_len;

      for(linsect=0; linsect<track_len; linsect++)
      {
         if( raw_read(disk_drive, track_start+linsect,
	                          track_buffer+linsect*512) <0 )
	    return 0;
      }
      if( raw_read(disk_drive, 0, 0) <0 )
         return 0;

      track_number = track_no;
   }
   return track_buffer + 512*track_off;
}

static int raw_read(drive, linsect, buffer)
int drive, linsect;
char * buffer;
{
static char * pend_buf = 0, *buf_start = 0;
static int pend_s, pend_h, pend_c, pend_len = 0;

   int phy_s = linsect%disk_spt;
   int phy_h = linsect/disk_spt%disk_heads;
   int phy_c = linsect/disk_spt/disk_heads;
   int tries = 5;
   int rv = 0;

   if( buffer != pend_buf || 
       pend_s+pend_len != phy_s ||
       pend_h != phy_h ||
       pend_c != phy_c )
   {
      if( buf_start ) do
      {
        rv = phy_read(drive, pend_c, pend_h, pend_s+1, pend_len, buf_start);
        tries--;
        if( rv ) printf("Error in phy_read(%d,%d,%d,%d,%d,%d);\n",
                         drive, pend_c, pend_h, pend_s+1, pend_len, buf_start);
      }
      while(rv && tries > 0);

      pend_c = phy_c;
      pend_h = phy_h;
      pend_s = phy_s;
      pend_len = 0;
      pend_buf = buf_start = buffer;
   }

   pend_len++;
   pend_buf += 512;

   return rv;
}

static int read_bootblock()
{
  char * sptr;
  int rv, media_byte = 0;
  if( alloc_trackbuf(2) ) return -1;

  disk_spt = 2;
  sptr = read_sector(1);
  disk_spt = -1;
  if( sptr == 0 ) return -1;
  media_byte = *(unsigned char*)sptr;

  /* Valid media byte ? */
  if( (media_byte & 0xF0) != 0xF0 ) return -1;
  disk_spt = 2;
  sptr = read_sector(0);
  disk_spt = -1;
  if( sptr == 0 ) return -1;

  if( DOS_MEDIA(sptr) != media_byte ) return -1;
  if( DOS_SPT(sptr) > 63 ) return -1;
  if( DOS_SECT(sptr) != 512 ) return -1;

  disk_spt   = DOS_SPT(sptr);
  disk_heads = DOS_HEADS(sptr);

  rv = alloc_trackbuf(disk_spt*disk_heads);	/* Cylinder buffer */
  if( rv < 0 ) rv = alloc_trackbuf(disk_spt);	/* Track buffer */
  if( rv < 0 ) rv = alloc_trackbuf(disk_spt/2);	/* 1/2 Track buffer */
  if( rv < 0 ) rv = alloc_trackbuf(2);		/* Block buffer */
  if( rv < 0 ) disk_spt = -1;
  return rv;
}

static int alloc_trackbuf(sectors)
int sectors;
{
   char * new_track;
   int seg_start, seg_end;
   if( sectors <= track_len ) return 0;

   if( track_buffer ) free(track_buffer);
   track_buffer = 0;
   track_len = 0;
   track_number = -1;

   if( sectors < 1 || sectors > 63 ) /* WTF! */ return -1;

   new_track = malloc(sectors*512);
   if( new_track == 0 ) return -1;

   seg_start = __get_ds() + (unsigned int)new_track / 16;
   seg_end   = __get_ds() + (unsigned int)(new_track+sectors*512-1) / 16;

   if( (seg_start&0xF000) != (seg_end&0xF000) ) /* Bugger */
   {
      int rv = alloc_trackbuf(sectors);
      free(new_track);
      return rv;
   }

   track_len = sectors;
   track_buffer = new_track;
   return 0;
}

#endif

#if defined(__MSDOS__) || defined(__STANDALONE__)
phy_read(drive, cyl, head, sect, length, buffer)
{
#asm
  push	bp
  mov	bp,sp

  push	ds
  pop	es

  mov	dl,[bp+2+_phy_read.drive]
  mov	ch,[bp+2+_phy_read.cyl]
  mov	dh,[bp+2+_phy_read.head]
  mov	cl,[bp+2+_phy_read.sect]
  mov	al,[bp+2+_phy_read.length]
  mov	bx,[bp+2+_phy_read.buffer]

  mov	ah,#$02
  int	$13
  jc	read_err
  mov	ax,#0
read_err:

  pop	bp
#endasm
}
#endif

