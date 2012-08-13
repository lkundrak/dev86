
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#ifndef __MSDOS__
#include <stdlib.h>
#endif

#include "sysboot.v"
#include "sysboot16.v"
#include "noboot.v"
#include "msdos.v"
#include "msdos16.v"
#include "skip.v"
#include "killhd.v"
#include "tarboot.v"
#include "minix.v"
#include "minixhd.v"
#include "mbr.v"
#include "mbr_dm.v"
#include "mbr_chs.v"
#include "mbr_lin.v"
#include "mbr_lba.v"
#include "pbr.v"
#include "nombr.v"

/* Binary bootblocks */
#include "freedos.h"
#ifdef USE_WIN95BB
#include "win95.h"
#endif

unsigned char buffer[1024];

#define FS_NONE	0	/* Bootsector is complete */
#define FS_DOS	1	/* Bootsector needs 'normal' DOS FS */
#define FS_ADOS	2	/* Bootsector likes any DOS FS */
#define FS_TAR	3	/* Bootsector needs GNU-tar volume label */
#define FS_STAT	4	/* Any bootsector is checked */
#define FS_ZERO 5	/* Boot sector must be already Zapped */
#define FS_MBR  6	/* Boot sector is an MBR */

#ifndef __MSDOS__
#define X_HAS_2M20
#endif

struct bblist {
   char * name;
   char * desc;
   char * data;
   int	  size;
   int  name_type;
   int	boot_name;
   int	fstype;
   int  fsmod;
} bblocks[] = {
/* Default */
{ "stat", "Display dosfs superblock",                          
           0, 0, 0, 0, 					FS_STAT},
{ "copy", "Copy boot block to makeboot.sav or named file",                   
           0, 0, 0, 0, 					FS_STAT},
{ "Zap",  "Clear boot block to NULs",                       
           0, 1024, 0, 0, 				FS_NONE},

{ "none", "No OS bootblock, just display message",  
           noboot_data, noboot_size, 
	   2, noboot_boot_message-noboot_start, 	FS_ADOS},
{ "skip", "Bypasses floppy boot with message",  
           skip_data, skip_size,  
	   2, skip_mesg-skip_start,			FS_ADOS},
#if __STDC__
{ "mbr",  "Master boot record for HD"
#if defined(mbr_Banner) || mbr_diskman || mbr_linear || mbr_mbrkey || mbr_preboot || mbr_direct || mbr_pbr
   ", Options:"
#ifdef mbr_Banner
   " Banner"
#endif
#if mbr_diskman
   " DiskMan"
#endif
#if mbr_linear
   " LBA"
#if !mbr_useCHS
   "-Only"
#endif
#endif
#if mbr_useCHS && mbr_linCHS
   " linCHS"
#endif
#if mbr_mbrkey
   " BootKeys"
#endif
#if mbr_preboot
   " PreBoot"
#endif
#if mbr_direct
   " Direct"
#endif
#if mbr_pbr
   " PBR"
#endif
#endif
	   ,             
           mbr_data,mbr_size, 
#ifdef mbr_Banner
	   2, mbr_Banner-mbr_start, 			FS_MBR},
#else
           0, 0, 					FS_MBR},
#endif

#else
{ "mbr",  "Master boot record for HD",
	   mbr_data,mbr_size, 0, 0,                     FS_MBR},
#endif

{ "pbr",   "LBA-Only Partition boot record",
	   pbr_data,pbr_size, 0, 0,                     FS_ADOS},
{ "mbrchs","MBR using CHS addressing and BIOS",
	   mbr_chs_data,mbr_chs_size, 0, 0,             FS_MBR},
{ "mbrlin","MBR using Linear addressing and CHS BIOS",
	   mbr_lin_data,mbr_lin_size, 0, 0,             FS_MBR},
{ "mbrlba","MBR using Linear addressing and LBA BIOS",
	   mbr_lba_data,mbr_lba_size, 0, 0,             FS_MBR},
{ "nombr", "Boot failure message for MBR",
	   nombr_data,nombr_size,
	   2, nombr_message-nombr_start,                FS_MBR},

#ifdef mbr_dm_size
{ "mbrdm", "DM Master boot record for HD",
	   mbr_dm_data,mbr_dm_size, 0, 0,               FS_MBR},
#endif

{ "minix","Minix floppy FS booter",           
           minix_data, minix_size, 
	   2, minix_bootfile-minix_start,		FS_ZERO},
{ "hdmin","Minix Hard disk FS booter",    
           minixhd_data, minixhd_size, 
	   2, minixhd_bootfile-minixhd_start,		FS_ZERO},
{ "tar",  "Bootable GNU tar volume label",
   	   tarboot_data, tarboot_size, 0, 0, 		FS_TAR},
{ "dos12","Boot file BOOTFILE.SYS from dos floppy",
           msdos_data, msdos_size,
	   1, msdos_boot_name-msdos_start,		FS_DOS, 12},
{ "dos16","Boot file BOOTFILE.SYS from FAT16 hard disk or floppy",
           msdos16_data, msdos16_size,
	   1, msdos16_boot_name-msdos16_start,		FS_DOS, 16},
{ "fd32l","Freedos FAT32 LBA Bootblock",
           freedos_fat32lba, 512,
	   0, 0,					FS_DOS, 32},
{ "fd32c","Freedos FAT32 CHS Bootblock",
           freedos_fat32chs, 512,
	   0, 0,					FS_DOS, 32},
#ifdef USE_WIN95BB
{ "win95","The Windows 95 bootblock, (C) Microsoft.",
           win95_bootblock, 512,
	   0, 0,					FS_DOS, 12},
#endif
{ "killhd", "Deletes MBR from hard disk when booted",  
           killhd_data, killhd_size,  
	   2, killhd_boot_message-killhd_start, 	FS_ADOS},
   0
};

char * progname = "";

int disktype = 0;
FILE * diskfd;

int disk_sect = 63;	/* These are initialised to the maximums */
int disk_head = 256;	/* Set to the correct values when an MSDOS disk is */
int disk_trck = 256;	/* successfully identified */

int force = 0;
int write_zero = 1;	/* Write sector 0 */
int write_one = 0;	/* Write sector 1 */
int copy_zero = 0;	/* Copy of sector zero */

char * boot_id = 0;

/* Overrides for bpb in dos bootblocks. */
unsigned char bpb_buffer[100];
unsigned char bpb_flags[100];
int has_bpb_overrides = 0;

main(argc, argv)
int argc;
char ** argv;
{
   FILE * fd;
   struct bblist *ptr = bblocks;
   int i;
   int ar, opton=1;
   char *p;
   char * bbname = 0;
   char * devname = 0;

   progname = argv[0];

   /* Traditional option processing. */
   for(ar=1; ar<argc; ar++) 
      if(opton && argv[ar][0] == '-' && argv[ar][1] != 0) 
	 for(p=argv[ar]+1;*p;p++)
	    switch(*p) {
	       char ch, * ap;
	       case '-': opton = 0; break;
	       case 'f': force++; break;
	       case 'H': set_superfield("drive=0x80");
	       case 'g': set_superfield("spt=63");
                         set_superfield("heads=255");
			 break;
	       default: 
		  ch = *p;
	          if (p[1]) { ap = p+1; p=" "; }
		  else {
		     if (ar+1>=argc) Usage();
		     ap = argv[++ar];
		  }
		  switch(ch) {
		     case 'S': set_superfield(ap); break;
		     default:  Usage();
		  }
		  break;
	    }
      else {
         if (bbname) Usage();
	 bbname = devname;
	 devname = argv[ar];
      }

   if( devname == 0 ) Usage();
   /* First bootblock is default */
   if( bbname == 0 ) bbname = bblocks->name;

   boot_id = strchr(bbname, '=');
   if( boot_id )
      *boot_id++ = '\0';

   if( (i=strlen(bbname)) < 2 ) Usage();
   for(ptr = bblocks; ptr->name; ptr++)
      if( strncmp(bbname, ptr->name, i) == 0 ) break;
   if( ptr->name == 0 ) Usage();

   if (has_bpb_overrides && ptr != bblocks &&
       ptr->fstype != FS_DOS && ptr->fstype != FS_ADOS)
      Usage();

   open_disk(devname);
   if( read_sector(0, buffer) != 0 )
      exit(1);
   read_sector(1, buffer+512);

   write_zero = (ptr->size >= 512);
   write_one =  (ptr->size >= 1024);

   switch(ptr->fstype)
   {
   case FS_NONE:	/* override */
   case FS_STAT:
   case FS_ADOS:
      if ( has_bpb_overrides )
         check_msdos();
      break;
   case FS_DOS:
      check_msdos();
      if(ptr->fsmod) check_simpledos(ptr->fsmod);
      break;
   case FS_TAR:
      check_tar();
      break;
   case FS_ZERO:
      check_zapped();
      break;
   case FS_MBR:
      check_mbr();
      break;

   default:
      fprintf(stderr, "Program error, unknown filesystem requirement\n");
      exit(2);
   }

   switch(ptr->fstype)
   {
   case FS_DOS:
   case FS_ADOS:
      if (ptr->fsmod == 12 || ptr->fsmod == 16)
      {
	 for(i=0; i<sysboot16_dosfs_stat; i++)
	    buffer[i] = ptr->data[i];
	 for(i=sysboot16_codestart; i<512; i++)
	    buffer[i] = ptr->data[i];
      }
      else
      {
	 for(i=0; i<sysboot_dosfs_stat; i++)
	    buffer[i] = ptr->data[i];
	 for(i=sysboot_codestart; i<512; i++)
	    buffer[i] = ptr->data[i];
      }
      break;

   case FS_TAR:
      copy_tarblock();
      break;

   case FS_MBR:
      copy_mbr(ptr->data, ptr->size);
      break;

   case FS_ZERO:
   case FS_NONE:
      if( ptr->data )
	 memcpy(buffer, ptr->data, ptr->size);
      else
      {
	 memset(buffer, '\0', 1024);
	 write_one = 1;
      }
      break;
   }

   if ( has_bpb_overrides )
   {
      copy_superfields(buffer);
      write_zero = 1;
   }

   switch(ptr->fstype)
   {
   case FS_STAT:
      if( strcmp(ptr->name, "copy") == 0 )
         save_super(buffer);
      else
         print_super(buffer);

      if ( !has_bpb_overrides )
      {
         close_disk();
         exit(0);
      }
      break;
   }

   if( boot_id ) switch(ptr->name_type)
   {
   case 1:
      set_dosname(ptr->boot_name);
      break;
   case 2:
      set_asciz(ptr->boot_name);
      break;
   default:
      fprintf(stderr, "Cannot specify boot name for this block\n");
      exit(1);
   }

   if( write_zero ) {
      write_sector(0, buffer);
      if (copy_zero)
	 write_sector(copy_zero, buffer); /* FAT32 backup */
   }
   if( write_one )  write_sector(1, buffer+512);

   close_disk();
   exit(0);
}

Usage()
{
   struct bblist *ptr = bblocks;

   if( progname == 0 || *progname == 0 || progname[1] == 0 )
      progname = "makeboot";

#ifdef __MSDOS__
   fprintf(stderr, "Usage: %s [-f] [-S var=val] bootblock[=bootname] a:\n\n", progname);
   fprintf(stderr, "The 'a:' can be any drive or file or @: for the MBR.\n");
#else
   fprintf(stderr, "Usage: %s [-f] [-S var=val] bootblock[=bootname] /dev/fd0\n\n", progname);
#endif
   fprintf(stderr, "-S sets BPB fields in the msdos filesystem superblock.\n");
   fprintf(stderr, "The bootname is a filename or message to use with the block,\n");
   fprintf(stderr, "the blocks are:\n");
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
   /* Freedos fix */
   if( diskname[2] == '\r' ) diskname[2] = 0;

   if( diskname[2] == 0 && diskname[1] == ':' )
   {
      if (isalpha(diskname[0])) {
	 disktype = toupper(diskname[0])-'A'+1;
	 return 0;
      }
      if (diskname[0] =='@') {
	 disktype = 129;
	 return 0;
      }
   }
#endif
   disktype = 0;
   diskfd = fopen(diskname, "r+b");
   if( diskfd == 0 ) diskfd = fopen(diskname, "rb");
   if( diskfd == 0 && force ) diskfd = fopen(diskname, "w+b");
   if( diskfd == 0 )
   {
      fprintf(stderr, "Cannot open '%s'\n", diskname);
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
   if( disktype == 1 || disktype == 2 || disktype == 129 )
   {
      int tries, rv;
      int s,h,c;
      s = sectno%disk_sect + 1;
      h = sectno/disk_sect%disk_head;
      c = sectno/disk_sect/disk_head;

      for(tries=0; tries<6; tries++)
         if( (rv = bios_sect_write(disktype-1, c, h, s, loadaddr)) == 0 )
            break;
      if( rv )
      {
	 if (rv/256 == 3)
	    fprintf(stderr, "Write protect error writing sector %d\n", sectno);
	 else
	    fprintf(stderr, "Error writing sector %d, (%d)\n", sectno, rv/256);
	 return -1;
      }
      return 0;
   }
   if( disktype )
   {
      int tries, rv;

      for(tries=0; tries<6; tries++)
         if( (rv = dos_sect_write(disktype-1, sectno, loadaddr)) == 0 )
            break;
      if( rv )
      {
         fprintf(stderr, "Error writing sector %d, (0x%04d)\n", sectno, rv);
         memset(loadaddr, '\0', 512);
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
   printf("Wrote sector %d\n", sectno);
   return 0;
}

int
read_sector(sectno, loadaddr)
int sectno;
char * loadaddr;
{
   int cc;
#ifdef __MSDOS__
   if( disktype == 1 || disktype == 2 || disktype == 129 )
   {
      int tries, rv;
      int s,h,c;
      s = sectno%disk_sect + 1;
      h = sectno/disk_sect%disk_head;
      c = sectno/disk_sect/disk_head;

      for(tries=0; tries<6; tries++)
         if( (rv = bios_sect_read(disktype-1, c, h, s, loadaddr)) == 0 )
            break;
      if( rv )
      {
         fprintf(stderr, "Error reading sector %d, (%d)\n", sectno, rv/256);
         memset(loadaddr, '\0', 512);
	 return -1;
      }
      return 0;
   }
   if( disktype )
   {
      int tries, rv;

      for(tries=0; tries<6; tries++)
         if( (rv = dos_sect_read(disktype-1, sectno, loadaddr)) == 0 )
            break;
      if( rv )
      {
         fprintf(stderr, "Error reading sector %d, (0x%04d)\n", sectno, rv);
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
bios_sect_read(drv, track, head, sector, loadaddr)
{
#asm
  push	bp
  mov	bp,sp

  push	ds
  pop	es

  mov	dh,[bp+2+_bios_sect_read.head]
  mov	dl,[bp+2+_bios_sect_read.drv]
  mov	cl,[bp+2+_bios_sect_read.sector]
  mov	ch,[bp+2+_bios_sect_read.track]

  mov	bx,[bp+2+_bios_sect_read.loadaddr]

  mov	ax,#$0201
  int	$13
  jc	bios_read_err
  mov	ax,#0
bios_read_err:

  pop	bp
#endasm
}

bios_sect_write(drv, track, head, sector, loadaddr)
{
#asm
  push	bp
  mov	bp,sp

  push	ds
  pop	es

  mov	dh,[bp+2+_bios_sect_write.head]
  mov	dl,[bp+2+_bios_sect_write.drv]
  mov	cl,[bp+2+_bios_sect_write.sector]
  mov	ch,[bp+2+_bios_sect_write.track]

  mov	bx,[bp+2+_bios_sect_write.loadaddr]

  mov	ax,#$0301
  int	$13
  jc	bios_write_err
  mov	ax,#0
bios_write_err:

  pop	bp
#endasm
}
#endif

/**************************************************************************/

#ifdef __MSDOS__

/* All this mess just to read one sector!! */

struct disk_packet {
   long	sector;
   int	count;
   long	addr;
} disk_packet;

dos_sect_read(drv, sector, loadaddr)
{
#asm
  push	bp
  mov	bp,sp

  mov	al,[bp+2+_dos_sect_read.drv]
  mov	cx,#1
  mov	dx,[bp+2+_dos_sect_read.sector]
  mov	bx,[bp+2+_dos_sect_read.loadaddr]

  int	$25
  pop	bx
  jnc	dos_read_ok

  mov	bp,sp

  ! Fill the disk packet
  mov	ax,[bp+2+_dos_sect_read.sector]
  mov	[_disk_packet],ax
  xor	ax,ax
  mov	[_disk_packet+2],ax
  inc	ax
  mov	[_disk_packet+4],ax
  mov	ax,[bp+2+_dos_sect_read.loadaddr]
  mov	[_disk_packet+6],ax
  mov	ax,ds
  mov	[_disk_packet+8],ax

  mov	dl,[bp+2+_dos_sect_read.drv]
  inc	dl
  mov	bx,#_disk_packet
  mov	cx,#0xFFFF
  mov	si,#0
  mov	ax,#0x7305

  int	$21

  jc	dos_read_err
dos_read_ok:
  mov	ax,#0
dos_read_err:

  pop	bp
#endasm
}

dos_sect_write(drv, sector, loadaddr)
{
#asm
  push	bp
  mov	bp,sp

  mov	al,[bp+2+_dos_sect_write.drv]
  mov	cx,#1
  mov	dx,[bp+2+_dos_sect_write.sector]
  mov	bx,[bp+2+_dos_sect_write.loadaddr]

  int	$26
  pop	bx
  jnc	dos_write_ok

  mov	bp,sp

  ! Fill the disk packet
  mov	ax,[bp+2+_dos_sect_write.sector]
  mov	[_disk_packet],ax
  xor	ax,ax
  mov	[_disk_packet+2],ax
  inc	ax
  mov	[_disk_packet+4],ax
  mov	ax,[bp+2+_dos_sect_write.loadaddr]
  mov	[_disk_packet+6],ax
  mov	ax,ds
  mov	[_disk_packet+8],ax

  mov	dl,[bp+2+_dos_sect_write.drv]
  inc	dl
  mov	bx,#_disk_packet
  mov	cx,#0xFFFF
  mov	si,#1
  mov	ax,#0x7305

  int	$21

  jc	dos_write_err
dos_write_ok:
  mov	ax,#0
dos_write_err:

  pop	bp
#endasm
}
#endif
/**************************************************************************/

check_zapped()
{
   int i;
   for(i=0; i<512; i++)
      if( buffer[i] )
         break;

   if( i != 512 )
   {
      fprintf(stderr, "Boot block isn't empty, zap it first\n");
      if(!force) exit(1);
   }
}

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
   char vbuf[100];
   unsigned char *p;
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
      if(!force) exit(9);

      write_one = 1;
      memset(buffer, '\0', 1024);
   }
   if( buff_tar.linkflag != 'V' )
   {
      fprintf(stderr, "Tar file doesn't start with a volume label\n");
      if(!force) exit(8);
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
   unsigned char * p;
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

#define DOS7_MAXSECT	17
#define DOS7_FAT32LEN	18
#define DOS7_FLAGS	19
#define DOS7_VERSION	20
#define DOS7_ROOT_CLUST	21
#define DOS7_INFO_SECT	22
#define DOS7_BOOT2	23

#define DOS7_PHY_DRIVE	24
#define DOS7_SERIAL	25
#define DOS7_LABEL	26
#define DOS7_FATTYPE	27

struct bootfields {
   char * label;
   int offset;
   int length;
   unsigned value;
   long lvalue;
   char * new_value;
}
   dosflds[] =
{
   { "sysid",  0x03, 8, 0},
   { "ssize",  0x0B, 2, 0},
   { "csize",  0x0D, 1, 0},
   { "resvs",  0x0E, 2, 0},
   { "nofats", 0x10, 1, 0},
   { "noroot", 0x11, 2, 0},
   { "sects",  0x13, 2, 0},
   { "media",  0x15, 1, 0},
   { "fatlen", 0x16, 2, 0},
   { "spt",    0x18, 2, 0},
   { "heads",  0x1A, 2, 0},
   { "hidden", 0x1C, 4, 0},

   { "lsects", 0x20, 4, 0},	/* DOS4+ */
   { "drive",  0x24, 1, 0},
   { "serial", 0x27, 4, 0},
   { "label",  0x2B, 11, 0},
   { "fatid",  0x36, 8, 0},

   { "lsects", 0x20, 4, 0},	/* DOS7 FAT32 */
   { "fatlen", 0x24, 4, 0},
   { "flags",  0x28, 2, 0},
   { "version",0x2A, 2, 0},
   { "root",   0x2C, 4, 0},
   { "info",   0x30, 2, 0},
   { "boot",   0x32, 2, 0},

   { "drive",  0x40, 1, 0},
   { "serial", 0x43, 4, 0},
   { "label",  0x47, 11, 0},
   { "fatid",  0x52, 8, 0},

   { 0,        0x3e8, 4, 0},
   { 0,        0x3ec, 4, 0},

   { 0, -1,0,0}
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
   "Sector count",
   "Media code",
   "FAT length",
   "Sect/Track",
   "Heads",
   "Hidden sectors (Partition offset)",

   "DOS4 Large Filesystem sector count",
   "DOS4 Phys drive",
   "DOS4 Serial number",
   "DOS4 Disk Label (DOS 4+)",
   "DOS4 FAT type",

   "FAT32 Filesystem sector count",
   "FAT32 FAT length",
   "FAT32 Flags",
   "FAT32 version",
   "FAT32 Root Cluster",
   "FAT32 Info Sector",
   "FAT32 Backup Boot",

   "FAT32 Phys Drive",
   "FAT32 Serial number",
   "FAT32 Disk Label",
   "FAT32 FAT Type",

   "FAT32 Free clusters",
   "FAT32 Next free cluster",

   0
};
   int i;
   long numclust = 0xFFFF;
   int fatbits = 0;
   int fat_len = -1;

   for(i=0; dosflds[i].offset >= 0; i++)
   {
      if( i>= DOS4_MAXSECT && (fat_len==0) != (i>=DOS7_MAXSECT) )
	 continue;

      if( dosflds[i].length <= 4 )
      {
         long v = 0; int j;
	 for(j=dosflds[i].length-1; j>=0; j--)
	 {
	    v = v*256 + (0xFF&( bootsect[dosflds[i].offset+j] ));
	 }

	 if( i==DOS_FATLEN )
	    fat_len = v;

	 if (v==0 && 
	  (i==DOS_FATLEN || i==DOS_MAXSECT || i==DOS4_MAXSECT || i==DOS_NROOT))
	    continue;

	 if ( i==DOS7_SERIAL )
            printf("%-35s%08x\n", fieldnames[i], v);
	 else
            printf("%-35s%ld\n", fieldnames[i], v);

	 if (i==DOS_SECT && v!=512 && v!=1024 && v!=2048)
	    break;
      }
      else
      {
         int ch, j;
         printf("%-35s", fieldnames[i]);
	 for(j=0; j<dosflds[i].length; j++)
	 {
	    ch = bootsect[dosflds[i].offset+j];
	    if( ch < ' ' || ch > '~' ) putchar('.');
	    else                       putchar(ch);
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
      if( i>= DOS4_MAXSECT && 
	    (dosflds[DOS_FATLEN].value==0) != (i>=DOS7_MAXSECT) )
      {
	 dosflds[i].lvalue = dosflds[i].value = 0;
	 continue;
      }

      if( dosflds[i].length <= 4 )
      {
         long v = 0; int j;
	 for(j=dosflds[i].length-1; j>=0; j--)
	 {
	    v = v*256 + (0xFF&( bootsect[dosflds[i].offset+j] ));
	 }
	 dosflds[i].value = v;
	 dosflds[i].lvalue = v;
      }
      else
	 dosflds[i].lvalue = dosflds[i].value = 0;
   }

   if (dosflds[DOS_FATLEN].value == 0)
     copy_zero = dosflds[DOS7_BOOT2].value;
   else
     copy_zero = 0;
}

save_super(bootsect)
char * bootsect;
{
   FILE * fd;
   char * fname = "makeboot.sav";
   if( boot_id ) fname = boot_id;

   printf("Copying boot block to '%s'\n", fname);
   fd = fopen(fname, "wb");
   fwrite(bootsect, 1024, 1, fd);
   fclose(fd);
}

set_superfield(setstr)
char * setstr;
{
   int i, l;
   char * av;
   has_bpb_overrides = 1;

   av = strchr(setstr, '=');
   if (av == 0) Usage();
   l = av++ - setstr;

   for(i=0; dosflds[i].offset >= 0; i++)
   {
      if ( dosflds[i].label && 
           l == strlen(dosflds[i].label) && 
           strncmp(dosflds[i].label, setstr, l) == 0)
         dosflds[i].new_value = av;
   }
}

copy_superfields(bootsect)
char * bootsect;
{
   int i, j;

   for(i=0; dosflds[i].offset >= 0; i++)
   {
      if (dosflds[i].new_value == 0) continue;
      if( i>= DOS4_MAXSECT && 
	    (dosflds[DOS_FATLEN].value==0) != (i>=DOS7_MAXSECT) )
      {
	 continue;
      }

      if( dosflds[i].length <= 4 )
      {
         long v = 0;
	 v = strtol(dosflds[i].new_value, 0, 0);
	 for(j=0; j<dosflds[i].length; j++)
	 {
	    bootsect[dosflds[i].offset+j] = (v & 0xFF);
	    v >>= 8;
	 }
      }
      else
      {
	 char * p = dosflds[i].new_value;
	 for(j=0; j<dosflds[i].length && *p; j++)
	 {
	    bootsect[dosflds[i].offset+j] = *p++;
	 }
      }
   }
}

/**************************************************************************/

check_msdos()
{
   decode_super(buffer);
   if( dosflds[DOS_CLUST].value == 0 )	/* MSDOS v1.0 */
      dosflds[DOS_CLUST].value = 1;

   if( dosflds[DOS_MEDIA].value < 0xF0 )
   {
      if (!force)
         fprintf(stderr, "Dos media descriptor is invalid\n");
   }
   else if( dosflds[DOS_MEDIA].value != (0xFF&buffer[512])
         && dosflds[DOS_RESV].value == 1 )
      fprintf(stderr, "Dos media descriptor check failed\n");
   else
   {
      disk_sect = dosflds[DOS_SPT].value;
      disk_head = dosflds[DOS_HEADS].value;
      if( disk_sect > 0 && disk_head > 0 )
         disk_trck = dosflds[DOS_MAXSECT].value/disk_head/disk_sect;

      return;
   }
   if(!force) exit(2);
}

check_simpledos(bb_fatbits)
int bb_fatbits;
{
   long numclust = 0xFFFF;
   char * err = 0;
   int fatbits = 0;

   /* Work out how many real clusters there are */
   if( dosflds[DOS_MAXSECT].value + 2 > 2 )
      numclust = ( dosflds[DOS_MAXSECT].value
                   - dosflds[DOS_RESV].value
                   - dosflds[DOS_NFAT].value * dosflds[DOS_FATLEN].value
                   - ((dosflds[DOS_NROOT].value+15)/16)
                 ) / dosflds[DOS_CLUST].value + 2;
   else if( dosflds[DOS4_MAXSECT].value > 0 )
      numclust = ( dosflds[DOS4_MAXSECT].lvalue
                   - dosflds[DOS_RESV].value
                   - dosflds[DOS_NFAT].value * dosflds[DOS_FATLEN].value
                   - ((dosflds[DOS_NROOT].value+15)/16)
                 ) / dosflds[DOS_CLUST].value + 2;
   else
      numclust = ( dosflds[DOS7_MAXSECT].lvalue
                   - dosflds[DOS_RESV].value
                   - dosflds[DOS_NFAT].value * dosflds[DOS7_FAT32LEN].value
                 ) / dosflds[DOS_CLUST].value;

   if( memcmp(buffer+dosflds[DOS4_FATTYPE].offset, "FAT12", 5) == 0 )
      fatbits=12;
   else if( memcmp(buffer+dosflds[DOS4_FATTYPE].offset, "FAT16", 5) == 0 )
      fatbits=16;
   else if(dosflds[DOS_FATLEN].value == 0)
      fatbits=32;
   else
      fatbits=12+4*(numclust > 0xFF0) + 16*(numclust > 0xFFF0L);

   if( dosflds[DOS_NFAT].value > 2 )
      err = "Too many fat copies on disk";
   else if( dosflds[DOS_SECT].value != 512 )
      err = "Drive sector size isn't 512 bytes sorry no-go.";
   else if( fatbits == 16 && numclust < 0xFF0 )
      err = "Weirdness, FAT16 but less than $FF0 clusters";
   else if( fatbits != bb_fatbits )
      err = "Filesystem has the wrong fat type for this bootblock.";

   if( !err && (bb_fatbits == 12 || bb_fatbits == 16))
   {
      if( dosflds[DOS_NROOT].value < 15 )
         err = "Root directory has unreasonable size.";
      else if( numclust * dosflds[DOS_CLUST].lvalue / 
	    dosflds[DOS_SPT].value > 65535 )
         err = "Boot sector untested with more than 65535 tracks";
   }

   if( !err && bb_fatbits == 12 )
   {
      if( dosflds[DOS4_PHY_DRIVE].value != 0 )
         err = "This boot sector is only for floppies";
      else if( (0x7C00-msdos_start-512)/512 < dosflds[DOS_FATLEN].value )
         err = "The FAT is too large to load in the available space.";
      else if( dosflds[DOS_RESV].value + dosflds[DOS_FATLEN].value > 
               dosflds[DOS_SPT].value )
         err = "The bootblock needs all of fat1 on the first track.";
      else if( msdos_heads == 2 && dosflds[DOS_HEADS].value != 2 )
         err = "Drive doesn't have two heads, this is required.";
      else if( dosflds[DOS_HIDDEN].lvalue != 0 )
         err = "MSDOS floppies shouldn't have hidden sectors.";
   }
   
   if( err )
   {
      fprintf(stderr, "ERROR: %s\n\n", err);
      print_super(buffer);
      if(!force) exit(2);
   }
}

set_dosname(boot_name)
int boot_name;
{
   char dos_name[20];
   int i,j;

   strcpy(dos_name, "           ");

   for(i=0; boot_id[i] && boot_id[i] != '.' && i<16; i++)
      dos_name[i] = toupper(boot_id[i]);

   if( boot_id[i] == '.' )
   {
      for(j=8,i++; boot_id[i] && boot_id[i] != '.' && j<16; i++,j++)
         dos_name[j] = toupper(boot_id[i]);
   }

   printf("Bootfile set to '%11.11s'\n", dos_name);

   memcpy(buffer+boot_name, dos_name, 11);
}

set_asciz(boot_name)
int boot_name;
{
   int i, j;

   for(i=boot_name; buffer[i]; i++) ;
   for(           ; !buffer[i]; i++) ;
   i = i - boot_name -1;

   if( strlen(boot_id) > i )
   {
      fprintf(stderr, "Name '%s' is too long for bootblock\n", boot_name);
      exit(1);
   }
   else
   {
      for(i=0,j=boot_name; boot_id[i]; i++)
      {
	 if( boot_id[i] == '\\' && boot_id[i+1] )
	 {
	    i++;
	    switch(boot_id[i])
	    {
	       case 'n': buffer[j++] = '\n'; break;
	       case 'r': buffer[j++] = '\r'; break;
	       case 'b': buffer[j++] = '\b'; break;
	       case 't': buffer[j++] = '\t'; break;
	       case 'a': buffer[j++] = '\007'; break;
	       case 'e': buffer[j++] = '\033'; break;
	       default:  buffer[j++] = boot_id[i]; break;
	    }
	 }
#ifdef __MSDOS__
	 else if(boot_id[i] == '_')   buffer[j++] = ' ';
	 else if(boot_id[i] == '^') { buffer[j++] = '\r'; buffer[j++] = '\n'; }
#endif
	 else buffer[j++] = boot_id[i];
      }
      buffer[j] = 0;
   }
}

check_mbr()
{
   int i = 0;

   if( buffer[510] == 0x55 && buffer[511] == 0xAA )
      i = 512;

   for(; i<512; i++)
      if( buffer[i] )
         break;

   /* Check for Disk Manager partition tables */
   if( buffer[252] == 0x55 && buffer[253] == 0xAA )
   {
      if( (unsigned char)mbr_data[252] != 0x55 || 
	  (unsigned char)mbr_data[253] != 0xAA )
	 i = 252;
   }

   if( i != 512 )
   {
      if(force)
         fprintf(stderr, "That doesn't look like a compatible MBR but ...\n");
      else
      {
         fprintf(stderr, "That doesn't look like a compatible MBR\n");
         exit(1);
      }
   }
}

copy_mbr(boot_data, boot_size)
char * boot_data;
int boot_size;
{
   int boot_to_copy = 446;

   if (boot_size < boot_to_copy) 
     boot_to_copy = boot_size;

   if ( boot_to_copy > 254 && 
        buffer[252] == 0xAA && buffer[253] == 0x55 && 
        (unsigned char)boot_data[252] == 0xAA && boot_data[253] == 0x55 )
      boot_to_copy = 254;

   memcpy(buffer, boot_data, boot_to_copy);
   buffer[510] = 0x55;
   buffer[511] = 0xAA;
   write_zero = 1;
}
