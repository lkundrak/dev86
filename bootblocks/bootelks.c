
#include <dos.h>
#include "minix.h"

extern union REGS __argr;
int x86_test = 0;
unsigned int boot_mem_top = 0;

void mem_check();

int  boot_drive = 0;
long boot_offset = 0;
int  boot_spt = 0;

char dirname[16]   = "boot";
char imagename[16] = "linux";

char lbuf[80];

int serr();
int (*read_sector)() = serr;

struct super_block b_super;
d_inode            b_inode[INODES_PER_BLOCK];
zone_nr            b_zone[NR_INDIRECTS];
/* dir_struct         directory[]; */

main()
{
   writes("Booting Elks....");

   mem_check();

   writes("\nSystem has ");
   writes(itoa(boot_mem_top/64));
   writes("k of low memory\n");

   boot_drive   = __argr.h.dl;
   boot_offset  = __argr.x.cx;
   boot_offset += ((long)__argr.h.dh << 16);
   boot_spt     = __argr.x.si;

   relocator(-1);

   writes("Relocated code to ");
   writes(itoa(__get_cs()/64));
   writes("k.\n");

   writes("Booting from ");
   if( boot_drive & 0x80 ) writes("hard");
   else                    writes("floppy");
   writes(" drive ");
   writes(itoa(boot_drive&0x7F));
   if( boot_offset )
   {
      writes(" offset ");
      writes(ltoa(boot_offset));
      writes(" sectors");
   }
   writes("\n");

   if( boot_drive & 0x80 ) init_hd(boot_drive);
   else                    init_fd(boot_drive);

   if( load_dir(0) >= 0 && load_dir(dirname) >= 0 && load_file(imagename) >= 0 )
      run_elks();
   else if( load_dir(0) >= 0 && load_file(imagename) >= 0 )
      run_elks();
   else
   {
      writes("Cannot find kernel image file\n");
      read(0, lbuf, 1); exit(1);
   }

   read(0, lbuf, sizeof(lbuf));
}

init_fd(drive) { }
init_hd(drive) { }
load_dir(name) { return -1; }
load_file(name) { return -1; }
run_elks() { }

/****************************************************************************/

writes(str)
char * str;
{
   write(1, str, strlen(str));
}

void mem_check()
{
#asm
  int	0x12		! Amount of boot memory
  mov	cl,#6
  sal	ax,cl		! In segments
  mov	[_boot_mem_top],ax
#endasm
}

serr()
{
   writes("Cannot read sector, drive not initilised\n");
   return -1;
}

/****************************************************************************/

load_blocks()
{
   if( b_super.s_magic != SUPER_MAGIC ) return -1;

try_again:;
#ifdef zone_shift
   if( zone_shift != b_super.s_log_zone_size) return -1;
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

   if(!inode)
   {
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
