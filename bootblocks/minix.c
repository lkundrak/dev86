/*
 * This bootblock loads the linux-8086 executable in the file 'linux'
 * from the root directory of a minix filesystem.
 *
 * Copyright (C) 1990-1996 Robert de Bath, distributed under the GPL Version 2
 * Based on minix filesystem definitions.
 */

#include <a.out.h>

#undef DOTS		/* define to have dots printed */
#define zone_shift 0	/* for any < 32M (non-zero not supported yet) */

#define seg_at(k) ((k)*64)
#define seg_of(p) ((unsigned int)p >>4)
#define BOOTSEG	  (0x07c0)
#define LOADSEG   (0x1000)

#asm 
BOOTSEG  = 0x07c0
BOOTADDR = 0x7c00
LOADSEG  = 0x1000	! This must be sector aligned

.text
! Apparently on startup the only things we can assume are that we start at
! `start` (ABS addr $07C00) ...

! So first set CS=DS=ES=SS=0
! The we move this to $0500 and put the stack at the top of the first 64k.
! The directory 'file' is loaded $1500 and scanned there.
! The final executable will be loaded in the 2nd 64k chunk.
!
org $0500	! The lowest available address.
start:
  include sysboot.s
  org codestart

  xor	ax,ax
  mov	ds,ax
  mov	es,ax
  mov	ss,ax
  mov	sp,ax

  mov	ax,#$0204		! Read 4 sectors, code + superblock.
  mov	bx,#start		! Where this _should_ be
  mov	cx,#$0001		! From sector 1
  xor	dx,dx			! Of the floppy drive head zero
  int	$13

  jmpi	code,#0

#endasm

/****************************************************************************/
/* Super block table.  The root file system and every mounted file system
 * has an entry here.  The entry holds information about the sizes of the bit
 * maps and inodes.  The s_ninodes field gives the number of inodes available
 * for files and directories, including the root directory.  Inode 0 is 
 * on the disk, but not used.  Thus s_ninodes = 4 means that 5 bits will be
 * used in the bit map, bit 0, which is always 1 and not used, and bits 1-4
 * for files and directories.  The disk layout is:
 *
 *      Item        # blocks
 *    boot block      1
 *    super block     1
 *    inode map     s_imap_blocks
 *    zone map      s_zmap_blocks
 *    inodes        (s_ninodes + 1 + INODES_PER_BLOCK - 1)/INODES_PER_BLOCK
 *    unused        whatever is needed to fill out the current zone
 *    data zones    (s_nzones - s_firstdatazone) << s_log_zone_size
 *
 * A super_block slot is free if s_dev == NO_DEV. 
 */

#define BLOCK_SIZE      1024	/* # bytes in a disk block */

/* Flag bits for i_mode in the inode. */
#define I_TYPE          0170000	/* this field gives inode type */
#define I_REGULAR       0100000	/* regular file, not dir or special */
#define I_BLOCK_SPECIAL 0060000	/* block special file */
#define I_DIRECTORY     0040000	/* file is a directory */
#define I_CHAR_SPECIAL  0020000	/* character special file */
#define I_SET_UID_BIT   0004000	/* set effective uid on exec */
#define I_SET_GID_BIT   0002000	/* set effective gid on exec */
#define ALL_MODES       0006777	/* all bits for user, group and others */
#define RWX_MODES       0000777	/* mode bits for RWX only */
#define R_BIT           0000004	/* Rwx protection bit */
#define W_BIT           0000002	/* rWx protection bit */
#define X_BIT           0000001	/* rwX protection bit */
#define I_NOT_ALLOC     0000000	/* this inode is free */

/* Type definitions */
typedef unsigned short unshort;	/* must be 16-bit unsigned */
typedef unshort block_nr;	/* block number */
typedef unshort inode_nr;	/* inode number */
typedef unshort zone_nr;	/* zone number */
typedef unshort bit_nr;		/* if inode_nr & zone_nr both unshort,
				   then also unshort, else long */

typedef unshort sect_nr;

typedef long zone_type;		/* zone size */
typedef unshort mask_bits;	/* mode bits */
typedef unshort dev_nr;		/* major | minor device number */
typedef char links;		/* number of links to an inode */
typedef long real_time;		/* real time in seconds since Jan 1, 1970 */
typedef long file_pos;		/* position in, or length of, a file */
typedef short uid;		/* user id */
typedef char gid;		/* group id */

/* Tables sizes */
#define NR_ZONE_NUMS       9	/* # zone numbers in an inode */
#define NAME_SIZE         14	/* # bytes in a directory component */

/* Miscellaneous constants */
#define SUPER_MAGIC   0x137F	/* magic number contained in super-block */

#define BOOT_BLOCK  (block_nr)0	/* block number of boot block */
#define SUPER_BLOCK (block_nr)1	/* block number of super block */
#define ROOT_INODE  (inode_nr)1	/* inode number for root directory */

/* Derived sizes */
#define NR_DZONE_NUM     (NR_ZONE_NUMS-2)		/* # zones in inode */
#define INODES_PER_BLOCK (BLOCK_SIZE/sizeof(d_inode))	/* # inodes/disk blk */
#define NR_INDIRECTS     (BLOCK_SIZE/sizeof(zone_nr))	/* # zones/indir blk */
#define INTS_PER_BLOCK   (BLOCK_SIZE/sizeof(int))	/* # integers/blk */

struct super_block {
  inode_nr s_ninodes;		/* # usable inodes on the minor device */
  zone_nr s_nzones;		/* total device size, including bit maps etc */
  unshort s_imap_blocks;	/* # of blocks used by inode bit map */
  unshort s_zmap_blocks;	/* # of blocks used by zone bit map */
  zone_nr s_firstdatazone;	/* number of first data zone */
  short s_log_zone_size;	/* log2 of blocks/zone */
  file_pos s_max_size;		/* maximum file size on this device */
  short s_magic;		/* magic number to recognize super-blocks */

} ;

/* Type definitions local to the File System. */
typedef struct {		/* directory entry */
  inode_nr d_inum;		/* inode number */
  char d_name[NAME_SIZE];	/* character string */
} dir_struct;

/* Declaration of the disk inode used in rw_inode(). */
typedef struct {		/* disk inode.  Memory inode is in "inotab.h" */
  mask_bits i_mode;		/* file type, protection, etc. */
  uid i_uid;			/* user id of the file's owner */
  file_pos i_size;		/* current file size in bytes */
  real_time i_modtime;		/* when was file data last changed */
  gid i_gid;			/* group number */
  links i_nlinks;		/* how many links to this file */
  zone_nr i_zone[NR_ZONE_NUMS];	/* block nums for direct, ind, and dbl ind */
} d_inode;

/****************************************************************************/

#ifdef zone_shift
#if zone_shift == 0
#define load_zone load_block
#endif
#else
static short     zone_shift;
#endif

/* The name of the file and inode to start */
extern char bootfile[];
extern inode_nr inode;

/* For multi-sector reads */
extern sect_nr lastsect;
extern sect_nr firstsect;
extern unsigned loadaddr;
extern unsigned loadcount;

/* Keep track of zones to load */
extern zone_nr * next_zone;
extern zone_nr * end_zone;
extern zone_nr   indirect;

/* Where to load zones */
extern unsigned ldaddr;

/* Directory reading */
extern dir_struct * dirptr;
extern unsigned flength;

/* The 'shape' of the floppy - intuit from superblock */
extern unsigned n_sectors;

/*
 * #define b_super	(*(struct super_block *) 1024)
 * #define b_inode	((d_inode *)2048)
 * #define b_zone	((zone_nr *)3072)
 */

extern struct super_block b_super;
extern d_inode            b_inode[INODES_PER_BLOCK];
extern zone_nr            b_zone[NR_INDIRECTS];
extern dir_struct         directory[];

/****************************************************************************/

#asm
! A few variables we need to know the positions of for patching, so export
! them and as86_encaps will make some variables.
.text
export	_inode			! Inode to search
_inode: .word	1		! ROOT_INODE

export	_bootfile		! File to boot, make this whatever you like,
_bootfile:			! 'boot' is good too.
   .ascii	"linux"
   .byte	0,0,0,0,0,0,0,0,0

_loadcount: .word	0
_firstsect: .word	0
_loadaddr:  .word	0
_lastsect:  .word	0

code:
  call	_loadprog
  call	_runprog
  br	_nogood
#endasm

static
load_block(address, blkno)
unsigned address, blkno;
{
   register sect_nr sectno;
   if(blkno == 0) { zero_block(address); return; }
#ifdef DOTS
   bios_putc('.');
#endif

   sectno = (sect_nr)blkno * 2;
   load_sect(address,    sectno);
   load_sect(address+32, sectno+1);
}

static
load_sect(address, sectno)
unsigned address;
sect_nr sectno;
{
   register sect_nr nsect;

   nsect   =  sectno%n_sectors +1;
   sectno /= n_sectors;
   nsect  |= (sectno<<8);

   if( loadcount )
   {
      lastsect++;
      if( ( address & 4095 ) && nsect == lastsect )
      {
         loadcount++;
         return;
      }
      get_now();
   }

   lastsect  = firstsect = nsect;
   loadaddr  = address;
   loadcount = 1;
}

static
get_now()
{
#asm
  xor	dx,dx
  mov	cx,[_firstsect]
  shr	ch,#1
  adc	dh,#0
  mov	es,[_loadaddr]
  xor	bx,bx
  mov	ax,[_loadcount]
  test	ax,ax
  jz	no_load
  mov	ah,#2
  int	$13
  jc	nogood
no_load:
  xor	ax,ax
  mov	[_loadcount],ax
#endasm
}

#asm
nogood:
  j _nogood
#endasm

static
zero_block(address)
{
#asm
#if __FIRST_ARG_IN_AX__
  mov	es,ax
#else
  mov	bx,sp
  mov	es,[bx+2]
#endif
  push	di
  mov	cx,#512
  xor	ax,ax
  mov	di,ax
  rep
   stosw
  pop	di
#endasm
}

#ifndef load_zone
NOT_DEFINED_load_zone(address, zoneno)
{
}
#endif

#ifdef DOTS
static
bios_putc(c)
{
#asm
#if !__FIRST_ARG_IN_AX__
  mov	bx,sp
  mov	ax,[bx+2]
#endif
  mov	ah,#$0E
  mov	bx,#7
  int	$10
#endasm
}
#endif

static
nogood()
{
/* This didn't work, chain the boot sector of the HD */
#asm
  push	cs
  pop	es
hcode:
  mov	ax,#$0201		! Read 1 sector
  mov	bx,#BOOTADDR		! In the boot area
  mov	cx,#$0001		! From sector 1
  mov	dx,#$0080		! Of the hard drive head zero
  int	$13
  jc	hcode			! Keep trying forever!
  jmpi	BOOTADDR,0
#endasm
}

#asm
end_of_part1:
  if *>start+0x200
     fail
  endif
  .blkb	0x200+start-*
#endasm

static
loadprog()
{
#ifdef DOTS
   bios_putc('+');
#endif
   if( b_super.s_magic != SUPER_MAGIC ) nogood();
   n_sectors = b_super.s_nzones / 80;
   if( n_sectors < 5 ) n_sectors = b_super.s_nzones / 40;

try_again:;
#ifdef zone_shift
   if( zone_shift != b_super.s_log_zone_size) nogood();
#else
   zone_shift = b_super.s_log_zone_size;
#endif

   inode--;
   load_block(seg_of(b_inode), inode/INODES_PER_BLOCK
                          + b_super.s_imap_blocks
                          + b_super.s_zmap_blocks
                          + 2);
   get_now();

   ldaddr = LOADSEG;		/* Load at 64k mark */

   {
      register d_inode * i_ptr;
      i_ptr = b_inode + inode%INODES_PER_BLOCK;
      next_zone = i_ptr->i_zone;
      flength   = i_ptr->i_size;
      if( (i_ptr->i_mode & I_TYPE) == I_DIRECTORY )
      {
	 ldaddr = seg_of(directory);
	 inode  = 0;		/* Mark - we've no _file_ inode yet */
      }
   }

   end_zone = next_zone+NR_DZONE_NUM;
   load_zone(seg_of(b_zone), (indirect = next_zone[NR_DZONE_NUM]));
   get_now();

   for(;;)
   {
      if( next_zone >= end_zone )
      {
         if( indirect != 0 )
         {
            next_zone = b_zone;
            end_zone  = next_zone + NR_INDIRECTS;
            indirect  = 0;
            continue;
         }
	 break;
      }
      load_zone(ldaddr, *next_zone);
      next_zone++;
      ldaddr += (seg_at(1) << zone_shift);
   }
   get_now();

#ifdef DOTS
   bios_putc('\r');
   bios_putc('\n');
#endif
   if(!inode)
   {
#ifdef DOTS
      bios_putc('+');
#endif
      dirptr = directory;
      while(flength > 0)
      {
register char * s = bootfile;
register char * p = dirptr->d_name;

	 if( dirptr->d_inum )
	 {
	    for(;;)
	    {
	       if( *s == '\0')
	       {
		  if(*p == '\0')
		  {
		     inode = dirptr->d_inum;
		     goto try_again;
		  }
		  break;
	       }
	       if( *s++ != *p++ ) break;
	    }
	 }
	 flength -= 16;
	 dirptr++;
      }
      nogood();
   }
}

static
runprog()
{
/* This did work, run the loaded executable */
#asm
  xor	dx,dx		! DX=0 => floppy drive
  push	dx		! CX=0 => partition offset = 0
  mov	si,[_n_sectors]	! Save for monitor.out

  mov	bx,#LOADSEG
  mov	ds,bx		! DS = loadaddress
  inc	bx
  inc	bx		! bx = initial CS
  xor	di,di		! Zero
  mov	ax,[di]
  cmp	ax,#0x0301	! Right magic ?
  bne	nogood		! Yuk ...
  mov	ax,[di+2]
  and	ax,#$20		! Is it split I/D ?
  jz	impure		! No ...
  mov	cl,#4
  mov	ax,[di+8]
  shr	ax,cl
impure:
  pop	cx
  add	ax,bx
  mov	ss,ax
  mov	sp,[di+24]	! Chmem value
  mov	ds,ax

  push	bx
  push	di		!  jmpi	0,#LOADSEG+2
  retf
#endasm
}

#asm
! These functions are pulled from the C library.
libstuff:
imodu:
	xor	dx,dx
	div	bx
	mov	ax,dx		! instruction queue full so xchg slower
	ret
idiv_u:
	xor	dx,dx
	div	bx
	ret
libend:

vars:
_n_sectors:	.word 0
_next_zone:	.word 0
_end_zone:	.word 0
_indirect:	.word 0
_ldaddr:	.word 0
_dirptr:	.word 0
_flength:	.word 0
varend:

end_of_prog:
  if *>start+0x400
     fail
  endif

  .blkb	0x3FF+start-*
  .byte 0xFF

_b_super:	.blkb 1024
_b_inode:	.blkb 1024
_b_zone:	.blkb 1024
_directory:	.blkb 32768

#endasm
