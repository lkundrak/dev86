
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include "sysboot.v"
#include "msdos.v"
#include "skip.v"
#include "tarboot.v"

char buffer[1024];

#define FS_NONE	0
#define FS_ADOS	1
#define FS_DOS	2
#define FS_TAR	3
#define FS_STAT	4

struct bblist {
   char * name;
   char * data;
   int	fstype;
   char * desc;
} bblocks[] = {
   { "tar",   tarboot_data,  FS_TAR,	"Bootable GNU tar volume lable"      },
   { "dosfs", msdos_data,    FS_ADOS,	"Boots file BOOTFILE.SYS from dosfs" },
   { "bare",  sysboot_data,  FS_DOS,	"Bare bootblock, lockup if booted"   },
   { "skip",  skip_data,     FS_DOS,	"Bypasses floppy boot with message"  },
   { "stat",  0,	     FS_STAT,	"Display dosfs superblock"	     },
   { "copy",  0,	     FS_STAT,	"Copy boot block to makeboot.sav"    },
   { "Zap",   0,	     FS_NONE,	"Clear boot block to NULs"	     },
   0
};

char * progname = "";

int disktype = 0;
FILE * diskfd;

int disk_sect = 63;	/* These are initilised to the maximums */
int disk_head = 256;	/* Set to the correct values when an MSDOS disk is */
int disk_trck = 256;	/* successfully identified */

main(argc, argv)
int argc;
char ** argv;
{
   FILE * fd;
   struct bblist *ptr;
   int i;

   progname = argv[0];

   if( argc != 3 ) Usage();

   if( (i=strlen(argv[1])) < 2 ) Usage();
   for(ptr = bblocks; ptr->name; ptr++)
      if( strncmp(argv[1], ptr->name, i) == 0 ) break;
   if( ptr->name == 0 ) Usage();

   open_disk(argv[2]);
   if( read_sector(0, buffer) != 0 )
      exit(1);
   read_sector(1, buffer+512);

   switch(ptr->fstype)
   {
   case FS_NONE:	/* override */
   	break;
   case FS_ADOS:
      check_simpledos();
      break;
   case FS_DOS:
   case FS_STAT:
      check_msdos();
      break;
   case FS_TAR:
      check_tar();
      break;

   default:
      fprintf(stderr, "Program error, unknown filesystem requirement\n");
      exit(2);
   }

   switch(ptr->fstype)
   {
   case FS_STAT:
      print_super(buffer);
      if( strcmp(ptr->name, "copy") == 0 )
         save_super(buffer);
      close_disk();
      exit(0);
   case FS_ADOS:
   case FS_DOS:
      for(i=0; i<sysboot_dosfs_stat; i++)
         buffer[i] = ptr->data[i];
      for(i=sysboot_codestart; i<512; i++)
         buffer[i] = ptr->data[i];
      break;

   case FS_TAR:
      copy_tarblock();
      break;

   case FS_NONE:
      if( ptr->data )
	 memcpy(buffer, ptr->data, 512);
      else
	 memset(buffer, '\0', 512);
      break;
   }

   write_sector(0, buffer);
   close_disk();
   exit(0);
}

Usage()
{
   struct bblist *ptr = bblocks;

   if( progname == 0 || *progname == 0 || progname[1] == 0 )
      progname = "makeboot";

#ifdef __MSDOS__
   fprintf(stderr, "Usage: %s bootname a:\n", progname);
#else
   fprintf(stderr, "Usage: %s bootname /dev/fd0\n", progname);
#endif
   fprintf(stderr, "Blocks\n");
   for(;ptr->name; ptr++)
       fprintf(stderr, "\t%s\t%s\n", ptr->name, ptr->desc);
   exit(1);
}

/**************************************************************************/

int
open_disk(diskname)
char * diskname;
{
#ifdef __MSDOS__
   if( strcmp("a:", diskname) == 0 ) { disktype = 1; return 0; }
   if( strcmp("b:", diskname) == 0 ) { disktype = 2; return 0; }
   if( strcmp("A:", diskname) == 0 ) { disktype = 1; return 0; }
   if( strcmp("B:", diskname) == 0 ) { disktype = 2; return 0; }
#endif
   disktype = 0;
   diskfd = fopen(diskname, "r+");
   if( diskfd == 0 )
   {
      fprintf(stderr, "Cannot open %s\n", diskname);
      exit(1);
   }
   return 0;
}

close_disk()
{
   if( diskfd && disktype == 0 ) fclose(diskfd);
   diskfd = 0;
   disktype = 0;
}

int
write_sector(sectno, loadaddr)
int sectno;
char * loadaddr;
{
#ifdef __MSDOS__
   if( disktype == 1 || disktype == 2 )
   {
      int tries, rv;
      int s,h,c;
      s = sectno%disk_sect + 1;
      h = sectno/disk_sect%disk_head;
      c = sectno/disk_sect/disk_head;

      for(tries=0; tries<6; tries++)
         if( (rv = dos_sect_write(disktype-1, c, h, s, loadaddr)) == 0 )
            break;
      if( rv )
      {
         fprintf(stderr, "Error writing sector %d, (%d)\n", sectno, rv/256);
	 return -1;
      }
      return 0;
   }
#endif
   if( disktype )
   {
      fprintf(stderr, "Cannot write sector %d\n", sectno);
      return -1;
   }
   fseek(diskfd, (long)sectno*512, 0);
   if( fwrite(loadaddr, 512, 1, diskfd) != 1 )
   {
      fprintf(stderr, "Cannot write sector %d\n", sectno);
      return -1;
   }
   return 0;
}

int
read_sector(sectno, loadaddr)
int sectno;
char * loadaddr;
{
   int cc;
#ifdef __MSDOS__
   if( disktype == 1 || disktype == 2 )
   {
      int tries, rv;
      int s,h,c;
      s = sectno%disk_sect + 1;
      h = sectno/disk_sect%disk_head;
      c = sectno/disk_sect/disk_head;

      for(tries=0; tries<6; tries++)
         if( (rv = dos_sect_read(disktype-1, c, h, s, loadaddr)) == 0 )
            break;
      if( rv )
      {
         fprintf(stderr, "Error reading sector %d, (%d)\n", sectno, rv/256);
         memset(loadaddr, '\0', 512);
	 return -1;
      }
      return 0;
   }
#endif
   if( disktype )
   {
      fprintf(stderr, "Cannot read sector %d\n", sectno);
      return -1;
   }
   fseek(diskfd, (long)sectno*512, 0);
   if( (cc=fread(loadaddr, 1, 512, diskfd)) != 512 )
   {
      fprintf(stderr, "Cannot read sector %d, clearing\n", sectno);
      if(cc<0) cc=0;
      memset(loadaddr+cc, '\0', 512-cc);
   }
   return 0;
}
/**************************************************************************/

#ifdef __MSDOS__
dos_sect_read(drv, track, head, sector, loadaddr)
{
#asm
  push	bp
  mov	bp,sp

  push	ds
  pop	es

  mov	dh,[bp+2+_dos_sect_read.head]
  mov	dl,[bp+2+_dos_sect_read.drv]
  mov	cl,[bp+2+_dos_sect_read.sector]
  mov	ch,[bp+2+_dos_sect_read.track]

  mov	bx,[bp+2+_dos_sect_read.loadaddr]

  mov	ax,#$0201
  int	$13
  jc	read_err
  mov	ax,#0
read_err:

  pop	bp
#endasm
}
#endif

#ifdef __MSDOS__
dos_sect_write(drv, track, head, sector, loadaddr)
{
#asm
  push	bp
  mov	bp,sp

  push	ds
  pop	es

  mov	dh,[bp+2+_dos_sect_write.head]
  mov	dl,[bp+2+_dos_sect_write.drv]
  mov	cl,[bp+2+_dos_sect_write.sector]
  mov	ch,[bp+2+_dos_sect_write.track]

  mov	bx,[bp+2+_dos_sect_write.loadaddr]

  mov	ax,#$0301
  int	$13
  jc	write_err
  mov	ax,#0
write_err:

  pop	bp
#endasm
}
#endif

/**************************************************************************/

struct tar_head {
   char name[100];
   char mode[8];
   char uid[8];
   char gid[8];
   char size[12];
   char mtime[12];
   char chksum[8];
   char linkflag;
   char linkname[100];
   char magic[8];
   char uname[32];
   char gname[32];
   char devmajor[8];
   char devminor[8];
   char padding[167];
} ;

#define buff_tar	(*(struct tar_head*) buffer)
#define boot_tar	(*(struct tar_head*) tarboot_data)

unsigned int oct(s)
char *s;
{
   unsigned int val = 0;
   int i;
   for(i=0; i<8; i++) if( s[i] >= '0' && s[i] <= '7' )
      val = (val<<3) + s[i] - '0';
   return val;
}

check_tar()
{
   char vbuf[100], *p;
   unsigned int csum = 0;
   long osum = -1;

   for(p=buffer; p<buffer+512; p++)
      if( *p ) goto not_zapped;
   /* Block zapped, ok */
   return 0;
not_zapped:

   osum = oct(buff_tar.chksum);
   memset(buff_tar.chksum, ' ', sizeof(buff_tar.chksum));

   for(p=buffer; p<buffer+512; p++)
      csum += (*p & 0xFF);

   if( csum != osum )
   {
      fprintf(stderr, "TAR file checksum failed, this isn't a tar file.\n");
      exit(9);
   }
   if( buff_tar.linkflag != 'V' )
   {
      fprintf(stderr, "Tar file doesn't start with a volume label\n");
      exit(8);
   }

   strcpy(vbuf, boot_tar.name); strcat(vbuf, " Volume 1");
   if( strcmp(boot_tar.name, buff_tar.name) != 0 
    && strcmp(vbuf, buff_tar.name) != 0 )
   {
      fprintf(stderr, "WARNING: Volume is labeled as '%s' not '%s'\n",
                       buff_tar.name, boot_tar.name);
   }
   return 0;
}

copy_tarblock()
{
   char lbuf[20];
   char * p;
   unsigned int csum = 0;
   int i;

   struct tar_head temp;

   temp = boot_tar;

   /* Copy preserved fields
    */
   if( buff_tar.name[0] )
   {
      memcpy(temp.mtime, buff_tar.mtime, sizeof(temp.mtime));

      memset(temp.name, 0x90, 16);
      for(i=0; buff_tar.name[i] && buff_tar.name[i] != ' ' && i<14; i++)
      {
         int ch = buff_tar.name[i];
         if( islower(ch) ) ch = toupper(ch);
         if( strchr("/?@ABCDEFGHIJKLMNO", ch) == 0 )
            ch = '?';
         temp.name[i] = ch;
      }
      temp.name[i++] = 0;
      temp.name[i]   = 0xC0;
   }
   else
      sprintf(temp.mtime, "%11lo", time((void*)0));

   buff_tar = temp;

   /* Re-calculate the checksum */
   memset(buff_tar.chksum, ' ', sizeof(buff_tar.chksum));

   for(p=buffer; p<buffer+512; p++)
      csum += (*p & 0xFF);

   sprintf(buff_tar.chksum, "%7o", csum);

   printf("Boot block installed");
   if( ((struct tar_head*)buffer)[1].name[0] )
      printf(" to boot file '%s'\n",
         ((struct tar_head*)buffer)[1].name);
   else
      printf(", use 'tar -r' to add executable\n");
}

/**************************************************************************/

#define DOS_SYSID	0
#define DOS_SECT	1
#define DOS_CLUST	2
#define DOS_RESV	3
#define DOS_NFAT	4
#define DOS_NROOT	5
#define DOS_MAXSECT	6
#define DOS_MEDIA	7
#define DOS_FATLEN	8
#define DOS_SPT		9
#define DOS_HEADS	10
#define DOS_HIDDEN	11
#define DOS4_MAXSECT	12
#define DOS4_PHY_DRIVE	13
#define DOS4_SERIAL	14
#define DOS4_LABEL	15
#define DOS4_FATTYPE	16

struct bootfields {
   int offset;
   int length;
   int value;
}
   dosflds[] =
{
   { 0x03, 8, 0},
   { 0x0B, 2, 0},
   { 0x0D, 1, 0},
   { 0x0E, 2, 0},
   { 0x10, 1, 0},
   { 0x11, 2, 0},
   { 0x13, 2, 0},
   { 0x15, 1, 0},
   { 0x16, 2, 0},
   { 0x18, 2, 0},
   { 0x1A, 2, 0},
   { 0x1C, 4, 0},
   { 0x20, 4, 0},
   { 0x24, 1, 0},
   { 0x27, 4, 0},
   { 0x2B, 11, 0},
   { 0x36, 8, 0},
   { -1,0,0}
};

print_super(bootsect)
char * bootsect;
{
static char * fieldnames[] = {
   "System ID",
   "Sector size",
   "Cluster size",
   "Reserved sectors",
   "FAT count",
   "Root dir entries",
   "Sector count (=0 if large FS)",
   "Media code",
   "FAT length",
   "Sect/Track",
   "Heads",
   "Hidden sectors (Partition offset)",
   "Large FS sector count",
   "Phys drive",
   "Serial number",
   "Disk Label (DOS 4+)",
   "FAT type",
   0
};
   int i;

   for(i=0; dosflds[i].offset >= 0; i++)
   {
      printf("%-35s", fieldnames[i]);
      if( dosflds[i].length <= 4 )
      {
         long v = 0; int j;
	 for(j=dosflds[i].length-1; j>=0; j--)
	 {
	    v = v*256 + (0xFF&( bootsect[dosflds[i].offset+j] ));
	 }
	 printf("%ld\n", v);
      }
      else
      {
         int ch, j;
	 for(j=0; j<dosflds[i].length; j++)
	 {
	    ch = bootsect[dosflds[i].offset+j];
	    if( ch <= ' ' || ch > '~' ) putchar('.');
	    else                        putchar(ch);
	 }
	 putchar('\n');
      }
   }
}

decode_super(bootsect)
char * bootsect;
{
   int i;

   for(i=0; dosflds[i].offset >= 0; i++)
   {
      if( dosflds[i].length <= 4 )
      {
         long v = 0; int j;
	 for(j=dosflds[i].length-1; j>=0; j--)
	 {
	    v = v*256 + (0xFF&( bootsect[dosflds[i].offset+j] ));
	 }
	 dosflds[i].value = v;
      }
      else
	 dosflds[i].value = 0;
   }
}

save_super(bootsect)
char * bootsect;
{
   FILE * fd;
   fd = fopen("makeboot.sav", "wb");
   fwrite(bootsect, 1024, 1, fd);
   fclose(fd);
}

/**************************************************************************/

check_msdos()
{
   decode_super(buffer);
   if( dosflds[DOS_CLUST].value == 0 )	/* MSDOS v1.0 */
      dosflds[DOS_CLUST].value = 1;

   if( dosflds[DOS_MEDIA].value < 0xF0 )
      fprintf(stderr, "Dos media descriptor is invalid\n");
   else if( dosflds[DOS_MEDIA].value != (0xFF&buffer[512])
         && dosflds[DOS_RESV].value == 1 )
      fprintf(stderr, "Dos media descriptor check failed\n");
   else
   {
      disk_sect = dosflds[DOS_SPT].value;
      disk_head = dosflds[DOS_HEADS].value;
      disk_trck = dosflds[DOS_MAXSECT].value/disk_head/disk_sect;
      return;
   }
   exit(2);
}

check_simpledos()
{
   int numclust;
   char * err = 0;
   check_msdos();

   /* Work out how many real clusters there are */
   numclust = ( dosflds[DOS_MAXSECT].value
                - dosflds[DOS_RESV].value
                - dosflds[DOS_NFAT].value * dosflds[DOS_FATLEN].value
                - ((dosflds[DOS_NROOT].value+15)/16)
              ) / dosflds[DOS_MAXSECT].value + 2;

   if( dosflds[DOS_NFAT].value > 2 )
      err = "Too many fat copies on disk";
   else if( dosflds[DOS_HIDDEN].value != 0 )
      err = "Dubious MSDOS floppy, it's got hidden sectors.";
   else if( dosflds[DOS_NROOT].value < 15 )
      err = "Root directory has unreasonable size.";
   else if( dosflds[DOS_SECT].value != 512 )
      err = "Drive sector size isn't 512 bytes sorry no-go.";
   else if( dosflds[DOS_HEADS].value != 2 )
      err = "Drive doesn't have two heads, this is required.";
   else if( numclust > 0xFF0 )
      err = "Filesystem has a 16 bit fat, only 12bits allowed.";
   else if( dosflds[DOS_RESV].value + dosflds[DOS_FATLEN].value > 
            dosflds[DOS_SPT].value )
      err = "The bootblock needs all of fat1 on the first track.";
   else
      return;

   fprintf(stderr, "ERROR: %s\n\n", err);
   print_super(buffer);
   exit(2);
}

/**************************************************************************/
