
#include "monitor.h"
#include "minix.h"

/* #define DEBUG 1 /**/

#define SECTOR_SIZE	512

block_nr inode_tbl = 0; 	/* super.s_imap_blocks+super.s_zmap_blocks+2 */
static int fs_dir32 = 0;	/* Using 30 char filenames ? */

struct file_pos {
   inode_nr	inode;
   zone_nr	seek_addr;
   file_pos	i_size;              	/* current file size in bytes */ 
   mask_bits    i_mode;			/* File type */
   zone_nr	i_zone[NR_ZONE_NUMS]; 	/* block numbers from inode */
} current_file = { 0,0 };

static int first_time = 1;
static inode_nr current_dir = ROOT_INODE;

static block_nr fs_ind   = 0;
static block_nr fs_block = 0;
static char     fs_buf[BLOCK_SIZE];	/* For storing inodes and ind sectors */

static min_inode_open();

#define b_super(x) ((struct super_block *)(x))
#define b_dir(x)   ((dir_struct*)(x))

min_open_file(fname)
char * fname;
{
   char * sptr;

   if( disk_heads == 0 ) return -1;

   if( (sptr = read_sector(SUPER_BLOCK*BLOCK_SIZE/SECTOR_SIZE)) == 0 )
      return -1;

   /* Check super */
   if(b_super(sptr)->s_magic == SUPER_MAGIC )
      fs_dir32 = 0;
   else if(b_super(sptr)->s_magic == SUPER_MAGIC2 )
      fs_dir32 = 16;
   else
      return -1;

   inode_tbl = b_super(sptr)->s_imap_blocks+b_super(sptr)->s_zmap_blocks+2;

   /* Check for boot directory */
   current_dir = ROOT_INODE;
   if( min_name_open("boot") >= 0 && current_file.i_mode == I_DIRECTORY )
      current_dir = current_file.inode;
#ifdef DEBUG
   fprintf(stderr, "Opening minix file <%d>/%s[1,%d]\n",
	           current_dir,fname,fs_dir32+14);
#endif
   return min_name_open(fname);
}

static
min_name_open(fname)
char * fname;
{
   int  do_dir = 0;
   char dirbuf[1024];
   long len;
   int  v; 
   inode_nr file_inode = 0;

   if( strcmp(fname, ".") == 0 ) do_dir = 1;

   /* open current_dir inode */
   if( min_inode_open(current_dir) < 0 ) return -1;

   /* scan for file */
   for(len = min_file_length(); len>0; len-=1024)
   {
      if( min_read_block(dirbuf) < 0 ) break;

      for(v=0; v<1024; v+= 16+fs_dir32)
      {
	 if( b_dir(dirbuf+v)->d_inum == 0 )
	    continue;

#ifndef NOCOMMAND
	 if( do_dir )
	 {
	    if( b_dir(dirbuf+v)->d_name[0] == '.' )
	       continue;

	    if( fs_dir32 ) printf("%.30s\n", b_dir(dirbuf+v)->d_name);
	    else           printf("%.14s\n", b_dir(dirbuf+v)->d_name);
	 }
	 else
#endif
	 if( strncmp(b_dir(dirbuf+v)->d_name, fname,14+fs_dir32) ==0)
	 {
            file_inode = b_dir(dirbuf+v)->d_inum;
	    break;
	 }
      }
   }

   /* open */
   if( file_inode )
   {
      if( min_inode_open(file_inode) >= 0 ) return 0;
   }

   return -1;
}

static 
min_inode_open(inode)
int inode;
{
   int inode_sector = inode_tbl*BLOCK_SIZE/SECTOR_SIZE;
   d_inode * sptr = 0;
   int i;

   min_close_file();

   inode--;
   inode_sector += inode/(SECTOR_SIZE/sizeof(d_inode));
   if( (sptr = read_sector(inode_sector)) == 0 ) return -1;

   sptr += inode%(SECTOR_SIZE/sizeof(d_inode));

   current_file.i_mode = (sptr->i_mode & I_TYPE);

   if( current_file.i_mode != I_REGULAR &&
       current_file.i_mode != I_DIRECTORY )
      return -1;

   current_file.i_size = sptr->i_size;
   for(i=0; i<NR_ZONE_NUMS; i++)
      current_file.i_zone[i] = sptr->i_zone[i];

   current_file.inode = inode+1;

   return 0;
}

min_rewind_file()
{
   current_file.seek_addr = 0;
   return 0;
}

min_close_file()
{
   if( current_file.inode == 0 ) return -1;
   current_file.inode = 0;
   current_file.seek_addr = 0;
   fs_ind = 0;
   return 0;
}

long
min_file_length()
{
   if( current_file.inode == 0 ) return -1;
   return current_file.i_size;
}

min_read_block(buffer)
char * buffer;
{
   block_nr block_no;

   if( current_file.seek_addr < NR_DZONE_NUM )
      block_no = current_file.i_zone[current_file.seek_addr];
   else if( current_file.seek_addr < NR_DZONE_NUM + NR_INDIRECTS )
   {
      if( fs_block != current_file.i_zone[NR_DZONE_NUM] )
      {
         block_no = current_file.i_zone[NR_DZONE_NUM];
	 read_fs_block(block_no, fs_buf);
         fs_block = current_file.i_zone[NR_DZONE_NUM];
	 fs_ind = 0;
      }

      block_no = ((zone_nr*)fs_buf)[current_file.seek_addr-NR_DZONE_NUM];
   }
   else
   {
      int ind_block;

      ind_block = current_file.seek_addr - NR_DZONE_NUM - NR_INDIRECTS;

      if( fs_ind != ind_block/NR_INDIRECTS + 1 )
      {
         block_no = current_file.i_zone[NR_DZONE_NUM+1];
	 read_fs_block(block_no, fs_buf);
         fs_block = current_file.i_zone[NR_DZONE_NUM+1];

         block_no = ((zone_nr*)fs_buf)[ind_block/NR_INDIRECTS];

	 read_fs_block(block_no, fs_buf);
         fs_block = current_file.i_zone[NR_DZONE_NUM];
         fs_ind = ind_block/NR_INDIRECTS + 1;
      }

      block_no = ((zone_nr*)fs_buf)[ind_block%NR_INDIRECTS];
   }

   current_file.seek_addr++;

   read_fs_block(block_no, buffer);
   return 0;
}

read_fs_block(block_no, buffer)
block_nr block_no;
char * buffer;
{
   long sector_no;
   char * sptr;

   if( block_no == 0 ) goto zappit;

   sector_no = ((long)block_no) * BLOCK_SIZE/SECTOR_SIZE;

   sptr = read_sector(sector_no);
   if( sptr ) memcpy(buffer, sptr, SECTOR_SIZE);
   else goto zappit;

   sptr = read_sector(sector_no+1);
   if( sptr ) memcpy(buffer+SECTOR_SIZE, sptr, SECTOR_SIZE);
   else goto zappit;

   return;
zappit:;
   memset(buffer, '\0', 1024);
}
