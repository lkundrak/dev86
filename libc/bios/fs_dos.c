
#ifdef DEBUG
#include <stdio.h>
#endif

#include <ctype.h>
#include <malloc.h>
#include <errno.h>
#include "io.h"
#include "rawio.h"

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

static int read_bootblock();
static int dir_nentry, dir_sect;
static int dos_clust0, dos_spc, dos_fatpos;
static int last_serial = 0;

struct filestatus {
   char fname[12];
   unsigned short first_cluster;
   unsigned short cur_cluster;
   unsigned short sector_no;		/* Max filesize = 32M */
   long file_length;
};

static int fsdos_read_block();
static int fsdos_close_file();

/* buffer to read into */
static char sect[512];

fsdos_open_file(iob, fname, flags, mode)
ioblock* iob;
char * fname;
int flags;
int mode;
{
   extern union REGS __argr;
   char conv_name[12];
   char *d, *s;
   int i;
   struct filestatus* cur_file;
   
#ifdef DEBUG
   fprintf(stderr, "fsdos_open_file(%x, %s, %d, %d, %d)\n",
           iob, fname, flags, mode, sizeof(iob));
#endif
   iob->block_read = fsdos_read_block;
   iob->close = fsdos_close_file;

   /* Get the superblock */
   if( read_bootblock() < 0 ) return -1;

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
   }
#ifdef DEBUG
   fprintf(stderr, "fsdos_open_file: converted filename=<%s>\n", conv_name);
#endif

   /* Scan the root directory for the file */
   s = sect;
   for(i=0; i<dir_nentry; i++)
   {
      if (rawio_read_sector(dir_sect+i/16, sect) == 0)
      {
         errno = EIO;
         return -1;
      }
      d = s + (i%16)*32;
      if( memcmp(d, conv_name, 11) == 0 && (d[11]&0x18) == 0 )
      { /* Name matches and is normal file */

#ifdef DEBUG
         fprintf(stderr, "dos_open_file: %s worked\n", fname);
#endif
         cur_file = malloc(sizeof(*cur_file));

         iob->context = cur_file;
         if (iob->context == NULL)
         {
            errno = ENOMEM;
            return -1;
         }
         memset(cur_file, '\0', sizeof(*cur_file));
         strcpy(cur_file->fname, conv_name);
	 cur_file->first_cluster = get_uint(d,26);
	 cur_file->file_length =   get_long(d,28);

	 cur_file->cur_cluster = cur_file->first_cluster;
	 cur_file->sector_no = 0;

	 return 0;
      }
   }
   return -1;
}

fsdos_rewind_file(iob)
ioblock* iob;
{
   struct filestatus* cur_file;
   
   if (iob == NULL || iob->context == NULL)
   {
       errno = EBADF;
       return -1;
   }

   cur_file = (struct filestatus*) iob->context;
   /* Is there an opened file ? */
   if( cur_file->fname[0] == 0 ) return -1;

   cur_file->sector_no = 0;
   cur_file->cur_cluster = cur_file->first_cluster;
   return 0;
}

static int fsdos_close_file(iob)
ioblock* iob;
{
   struct filestatus* cur_file;
   
   if (iob == NULL || iob->context == NULL)
   {
       errno = EBADF;
       return -1;
   }
   cur_file = (struct filestatus*) iob->context;
   free(cur_file);
   iob->context = NULL;

   rawio_reset_disk();
   return 0;
}

long
fsdos_file_length(iob)
ioblock* iob;
{
   struct filestatus* cur_file;
   
   if (iob == NULL || iob->context == NULL)
   {
       errno = EBADF;
       return -1;
   }
   cur_file = (struct filestatus*) iob->context;

   /* Is there an opened file ? */
   if( cur_file->fname[0] == 0 ) return -1;
   
   return cur_file->file_length;
}

static int fsdos_read_block(iob, buffer, block)
ioblock* iob;
char * buffer;
long block;     /* ignored for now */
{
   int s;
   char * ptr = buffer;
   struct filestatus* cur_file;
   long amount_left_in_file;
   long amount_to_read;

#ifdef DEBUG
   fprintf(stderr, "rb: iob = %x, buf = %x, block = %ld\n",
           iob, buffer, block);
#endif
   if (iob == NULL || iob->context == NULL)
   {
#ifdef DEBUG
       fprintf(stderr, "rb: no context\n");
#endif
       errno = EBADF;
       return -1;
   }
   cur_file = (struct filestatus*) iob->context;

   /* Is there an opened file ? */
   if( cur_file->fname[0] == 0 )
   {
#ifdef  DEBUG
      fprintf(stderr, "fsdos_read_block: File is not currently open!\n");
#endif
      errno = EBADF;
      return -1;
   }

   amount_left_in_file = cur_file->file_length - block * 1024;
   
   if (amount_left_in_file < 0)
   {
#ifdef  DEBUG
      fprintf(stderr, "EOF\n");
#endif
      return 0;
   }

   /* Are we before the EOF ? NB: FAT12 ONLY! */
   if( cur_file->cur_cluster >= 0xFF0 || cur_file->cur_cluster < 2 )
   {
#ifdef  DEBUG
      fprintf(stderr, "Hit end of file; cluster 0x%03x\n",
                       cur_file->cur_cluster);
#endif
      return 0;
   }

   for(s=0; s<2; s++)
   {
      unsigned int sectno;

      if (amount_left_in_file <= 0)
      {
#ifdef  DEBUG
         fprintf(stderr, "r2: Hit end of file\n");
#endif
         break;
      }

      sectno = dos_clust0
             + cur_file->cur_cluster * dos_spc
	     + cur_file->sector_no % dos_spc;

      if (rawio_read_sector(sectno, ptr) <= 0)
      {
#ifdef  DEBUG
         fprintf(stderr, "r2: rawio failed\n");
#endif
         return -1;
      }

      cur_file->sector_no++;
      if( cur_file->sector_no % dos_spc == 0 )
      {
         int odd = (cur_file->cur_cluster&1);
	 unsigned int val, val2;

	 val = cur_file->cur_cluster + (cur_file->cur_cluster>>1);
         if (rawio_read_sector(dos_fatpos+(val/512), sect) <= 0) return -1;
	 if( val%512 == 511 )
	 {
	    val2 = sect[511] & 0xFF;
            if (rawio_read_sector(dos_fatpos+(val/512)+1, sect) <= 0) return -1;
	    val2 |= (sect[0]<<8);
	 }
	 else
	    val2 = get_uint(sect, (val%512));

	 if( odd ) val2>>=4;

	 val2 &= 0xFFF;

         cur_file->cur_cluster = val2;
      }

      amount_to_read = amount_left_in_file;
      if (amount_to_read > 512) amount_to_read = 512;
      ptr += amount_to_read;
      amount_left_in_file -= amount_to_read;
   }

   return ptr - buffer;
}

static int read_bootblock()
{
   int rv, media_byte = 0;

#ifdef DEBUG
   fprintf(stderr, "fs_dos:read_bootblock:\n");
#endif
   if (rawio_read_sector(1, sect) <= 0) return -1;
   media_byte = *(unsigned char*)sect;

   /* Valid media byte ? */
   if( (media_byte & 0xF0) != 0xF0 ) return -1;
   if (rawio_read_sector(0, sect) <= 0) return -1;

   if( DOS_MEDIA(sect) != media_byte ) return -1;
   if( DOS_SPT(sect) > 63 ) return -1;
   if( DOS_SECT(sect) != 512 ) return -1;

   if( last_serial != DOS4_SERIAL(sect) ) fsdos_close_file();
   last_serial = DOS4_SERIAL(sect);

   /* Collect important data */
   dir_sect = DOS_RESV(sect) + DOS_NFAT(sect)*DOS_FATLEN(sect);
   dir_nentry = DOS_NROOT(sect);

   dos_fatpos = DOS_RESV(sect);
   dos_spc = DOS_CLUST(sect);
   if( dos_spc < 1 ) dos_spc = 1;
   dos_clust0 = dir_sect + (dir_nentry+15)/16 - 2*dos_spc;

   if( rawio_disk_cyls == 0 )
   {
      rawio_disk_spt   = DOS_SPT(sect);
      rawio_disk_heads = DOS_HEADS(sect);
   }

#ifdef DEBUG
   fprintf(stderr, "read_bootblock: heads(%d), spt(%d), dir_sect(%d)\n",
           rawio_disk_heads,
           rawio_disk_spt,
           dir_sect);
#endif
   return 0;
}
