
#include "monitor.h"

#ifdef BUFFER_FAT
#define read_fat_sector read_sector
#endif

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
#define DOS4_FATTYPE(P)    get_uint(P,0x39)

/* These assume alignment is not a problem */
#define get_byte(P,Off)	   *((unsigned char*)((char*)(P)+(Off)))
#define get_uint(P,Off)	   *((unsigned short*)((char*)(P)+(Off)))
#define get_long(P,Off)	   *((long*)((char*)(P)+(Off)))

typedef long Tsect;

static int read_bootblock();
static int dir_nentry, dir_sect;
static Tsect dos_clust0, dos_spc;
static int dos_fatpos, dos_fatlen, dos_fattype;
static int last_serial = 0;

#ifndef BUFFER_FAT
static char * read_fat_sector();
static int sector_no = 0;
static char sector_buf[512];
#endif

#ifdef BUFFER_FAT
static char * fat_buf = 0;
int           use_fatbuf = 0;
#endif

static
struct filestatus {
   char fname[12];
   unsigned short first_cluster;
   unsigned short cur_cluster;
   unsigned short sector_no;		/* Max filesize = 32M */
   long file_length;
}
   cur_file = { "", 0, 0, 0 };

dos_open_file(fname)
char * fname;
{
   char conv_name[12];
   char *s, *d;
   int i;
   int dodir = 0;

   /* Get the superblock */
   if( read_bootblock() < 0 ) return -1;

   if(strcmp(fname, ".") == 0)
      dodir = 1;
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
	 return dos_rewind_file();
   }

   memset(&cur_file, '\0', sizeof(cur_file));

#ifdef BUFFER_FAT
   if( !dodir && dos_fattype == 12 )
   {
      /* Read in and buffer the FAT */
      if( fat_buf ) free(fat_buf);
      fat_buf = malloc(dos_fatlen * 512);
      if( fat_buf == 0 ) 
	 use_fatbuf = 0;
      else
      {
	 use_fatbuf = 1;
	 for(i=0; i<dos_fatlen; i++)
	 {
	    s = read_sector(dos_fatpos+i);
	    if(s == 0)
	    {
	       use_fatbuf = 0;
	       free(fat_buf);
	       fat_buf = 0;
	    }
	    memcpy(fat_buf+i*512, s, 512);
	 }
      }
   }
   else
      use_fatbuf = 0;
#endif

#ifndef NOCOMMAND
   more_char(-1);
#endif

   /* Scan the root directory for the file */
   for(i=0; i<dir_nentry; i++)
   {
      s = read_sector(dir_sect+i/16);
      if( s == 0 ) return -1;
      d = s + (i%16)*32;
      if( dodir )
      {
#ifdef NOCOMMAND
         break;
#else
	 int dtime = 0;

	 if( *d > ' ' && *d <= '~' ) switch(d[11]&0x18)
	 {
	 case 0:
            printf("%-8.8s %-3.3s %10ld", d, d+8, get_long(d,28));
	    dtime=1;
	    break;
	 case 0x10:
            printf("%-8.8s %-3.3s <DIR>     ", d, d+8);
	    dtime=1;
	    break;
	 case 8:
            if( (d[11] & 7) == 0 )
	    {
	       printf("%-11.11s  <LBL>     ", d);
	       dtime=1;
	    }
	    break;
	 }
	 if( dtime )
	 {
	    printf(" %02d/%02d/%04d %02d:%02d",
	           (get_uint(d,24)&0x1F),
	           ((get_uint(d,24)>>5)&0xF),
	           ((get_uint(d,24)>>9)&0x7F)+1980,
	           ((get_uint(d,22)>>11)&0x1F),
	           ((get_uint(d,22)>>5)&0x3F)
	        );
	    if( more_char('\n') < 0 ) break;
	 }
#endif
      }
      else if( memcmp(d, conv_name, 11) == 0 && (d[11]&0x18) == 0 )
      { /* Name matches and is normal file */

         strcpy(cur_file.fname, conv_name);
	 cur_file.first_cluster = get_uint(d,26);
	 cur_file.file_length =   get_long(d,28);

	 cur_file.cur_cluster = cur_file.first_cluster;
	 cur_file.sector_no = 0;
#ifdef DEBUG
         fprintf(stderr, "Opened first cluster %d, len %ld\n",
	                  cur_file.first_cluster,
	                  cur_file.file_length
	        );
#endif

	 return 0;
      }
   }
   return -1;
}

dos_rewind_file()
{
   /* Is there an opened file ? */
   if( cur_file.fname[0] == 0 ) return -1;

   cur_file.sector_no = 0;
   cur_file.cur_cluster = cur_file.first_cluster;
   return 0;
}

dos_close_file()
{
#ifdef BUFFER_FAT
   if( fat_buf ) free(fat_buf);
   fat_buf = 0;
#endif
   memset(&cur_file, '\0', sizeof(cur_file));

   reset_disk();
   return 0;
}

long
dos_file_length()
{
   /* Is there an opened file ? */
   if( cur_file.fname[0] == 0 ) return -1;
   
   return cur_file.file_length;
}

dos_read_block(buffer)
char * buffer;
{
   int s;
   char * ptr;

   /* Is there an opened file ? */
   if( cur_file.fname[0] == 0 )
   {
#ifdef DEBUG
      fprintf(stderr, "File is not currently open!\n");
#endif
      return -1;
   }

   /* Are we before the EOF ? */
   if( ( dos_fattype == 12 &&  cur_file.cur_cluster >= 0xFF0 ) ||
       ( dos_fattype == 16 && (cur_file.cur_cluster&0xFFF0) == 0xFFF0 ) ||
	 cur_file.cur_cluster < 2 )
   {
#ifdef DEBUG
      fprintf(stderr, "Hit end of file; cluster 0x%03x\n",
                       cur_file.cur_cluster);
#endif
      return -1;
   }

   for(s=0; s<2; s++)
   {
      Tsect sectno;

      if( ( dos_fattype == 12 &&  cur_file.cur_cluster >= 0xFF0 ) ||
          ( dos_fattype == 16 && (cur_file.cur_cluster&0xFFF0) == 0xFFF0 ) ||
	    cur_file.cur_cluster < 2 )
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
	 if( dos_fattype == 12 )
	 {
	    int odd = (cur_file.cur_cluster&1);
	    unsigned int val, val2;

	    val = cur_file.cur_cluster + (cur_file.cur_cluster>>1);
#ifdef BUFFER_FAT
	    if( use_fatbuf )
	       val2 = get_uint(fat_buf, val);
	    else
#endif
	    {
	       ptr = read_fat_sector(dos_fatpos+(val/512));
	       if( ptr == 0 ) return -1;
	       if( val%512 == 511 )
	       {
		  val2 = (ptr[511]&0xFF);
		  ptr = read_fat_sector(dos_fatpos+(val/512)+1);
		  if( ptr == 0 ) return -1;
		  val2 |= (ptr[0]<<8);
	       }
	       else
		  val2 = get_uint(ptr, (val%512));
	    }

	    if( odd ) val2>>=4;

	    val2 &= 0xFFF;

	    cur_file.cur_cluster = val2;
	 }
	 else
	 {
	    ptr = read_fat_sector(dos_fatpos+(cur_file.cur_cluster/256));
	    if( ptr == 0 ) return -1;
	    cur_file.cur_cluster = get_uint(ptr, (cur_file.cur_cluster%256*2));
	 }
      }

      buffer += 512;
   }

   return 0;
}

static int read_bootblock()
{
   char * sptr;
   int rv, media_byte = 0;

#ifndef BUFFER_FAT
   sector_no = 0;
#endif

   sptr = read_sector(1);
   if( sptr == 0 ) return -1;
   media_byte = *(unsigned char*)sptr;

   /* Valid media byte ? */
   if( (media_byte & 0xF0) != 0xF0 ) return -1;
   sptr = read_sector(0);
   if( sptr == 0 ) return -1;

   if( DOS_MEDIA(sptr) != media_byte ) return -1;
   if( DOS_SPT(sptr) > 63 ) return -1;
   if( DOS_SECT(sptr) != 512 ) return -1;

   if( last_serial != DOS4_SERIAL(sptr) ) dos_close_file();
   last_serial = DOS4_SERIAL(sptr);

   /* Collect important data */
   dir_sect = DOS_RESV(sptr) + DOS_NFAT(sptr)*DOS_FATLEN(sptr);
   dir_nentry = DOS_NROOT(sptr);

   dos_fatpos = DOS_RESV(sptr);
   dos_fatlen = DOS_FATLEN(sptr);
   dos_spc = DOS_CLUST(sptr);
   dos_fattype = DOS4_FATTYPE(sptr);
   switch(dos_fattype)
   {
      case '1'+'6'*256: dos_fattype = 16; break;
      case '1'+'2'*256:
      default:          dos_fattype = 12; break;
   }
   if( dos_spc < 1 ) dos_spc = 1;
   dos_clust0 = dir_sect + (dir_nentry+15)/16 - 2*dos_spc;

   if( disk_cyls == 0 )
   {
      disk_spt   = DOS_SPT(sptr);
      disk_heads = DOS_HEADS(sptr);
   }

   return 0;
}

#ifndef BUFFER_FAT
static char *
read_fat_sector(sector)
int sector;
{
   char * p;
   if( sector == sector_no ) return sector_buf;
   p = read_sector(sector);
   if(p)
   {
      memcpy(sector_buf, p, 512);
      sector_no = sector;
      return sector_buf;
   }
   else
      return 0;
}
#endif

