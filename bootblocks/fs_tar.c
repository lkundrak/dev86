
#include "monitor.h"

#define HEADER_SIZE	512
#define NAME_SIZE	100
#define BLOCK_BOUNDARY	 20

typedef union {
  char hdr_block[HEADER_SIZE];
  struct m {
	char m_name[NAME_SIZE];
	char m_mode[8];
	char m_uid[8];
	char m_gid[8];
	char m_size[12];
	char m_time[12];
	char m_checksum[8];
	char m_linked;
	char m_link[NAME_SIZE];
	char m_ustar[8];
	char m_uname[32];
	char m_gname[32];

	char m_major[8];	/* GNU fields */
	char m_minor[8];
	char m_atime[12];
	char m_ctime[12];
	char m_offset[12];
	/* An so forth */
  } member;
} HEADER;

long tar_convert();
int  valid_tar_checksum();
void tar_set_drive();

static int disk_size = 0;

struct tx {
   char name[NAME_SIZE];
   int  first_sectno;
   int  cur_sectno;
   int  sectcount;
   int  diskoffset;
   long file_length;
} tar_status;

tar_open_file(fname)
char * fname;
{
   HEADER * sptr;
#ifndef NOCOMMAND
   int dodir = 0;
#endif
   int sectno;

   sptr = read_sector(0);

   /* Is it a tar disk ? */
   if( !valid_tar_checksum(sptr) ) return -1;

   tar_set_drive();

#ifndef NOCOMMAND
   if( strcmp(fname, ".") == 0 )
      dodir=1;
   else
#endif
   {
      if( tar_status.diskoffset == 0 && strcmp(fname, tar_status.name) == 0 )
         return tar_rewind_file();
   }

   tar_close_file();

   for(sectno=0;;)
   {
      long fl, v;

      if(sectno) sptr = read_sector(sectno);
      if(!sptr || !*sptr->member.m_name) break;

      if( !valid_tar_checksum(sptr) )
      {
         printf("Checksum error on tar header\n");
         return -1;
      }

      fl = tar_convert(sptr->member.m_size, 12);
      v = (fl+511)/512 + 1;

      if( sptr->member.m_linked != 0 && sptr->member.m_linked != '0' )
         ;
#ifndef NOCOMMAND
      else if( dodir )
          printf("%s %d tape blocks\n", sptr->member.m_name, (int)v-1);
#endif
      else if( strcmp(fname, sptr->member.m_name, NAME_SIZE) == 0 )
      {
         strncpy(tar_status.name, sptr->member.m_name, NAME_SIZE);
	 tar_status.first_sectno = sectno+1;
	 tar_status.cur_sectno = sectno+1;
	 tar_status.file_length = fl;
	 tar_status.sectcount = v-1;
	 tar_status.diskoffset = 0;
	 return 0;
      }

      if( v < 1 || (sectno += v) > disk_size ) break;
   }

   return -1;
}

tar_rewind_file()
{
   if( tar_status.name[0] == '\0' || tar_status.diskoffset != 0 )
   {
      tar_close_file();
      return -1;
   }

   tar_status.cur_sectno = tar_status.first_sectno;
   return 0;
}

tar_close_file()
{
   tar_status.name[0] = 0;
   tar_status.first_sectno = -1;
   tar_status.cur_sectno = -1;
   tar_status.file_length = -1;
   tar_status.diskoffset = -1;

   return 0;
}

long
tar_file_length()
{
   if( tar_status.name[0] == '\0' ) return -1;

   return tar_status.file_length;
}

tar_read_block(buffer)
char * buffer;
{
   char * ptr;
   HEADER * sptr;
   int i;
   if( tar_status.name[0] == '\0' ) return -1;

   for(i=0; i<2; i++)
   {
      if( tar_status.cur_sectno - tar_status.first_sectno >= tar_status.sectcount )
      {
         memset(buffer, '\0', 512);
      }
      else
      {
	 if( tar_status.cur_sectno >= tar_status.diskoffset+disk_size )
	 {
	    int k;
	    tar_status.diskoffset += disk_size-2;

	    for(;;)
	    {
	       printf("Please insert next disk and press return:");
	       fflush(stdout);
	       while( (k=(getch() & 0x7F)) != '\r' && k != '\n')
	          if( k == 27 || k == 3 )
		  {
		     printf("... Aborting\n");
		     return -1;
	          }
	       printf("\n");

	       sptr = read_sector(0);
	       if( !valid_tar_checksum(sptr) )
	       {
	          printf("Checksum failed reading volume label\n");
		  continue;
	       }
	       tar_set_drive();
	       sptr = read_sector(1);
	       if( !valid_tar_checksum(sptr)
	         || sptr->member.m_linked != 'M'
		 || 512*(long)(tar_status.cur_sectno-tar_status.first_sectno)
		    != tar_convert(sptr->member.m_offset, 12)
	         )
	       {
	          printf("Wrong disk inserted, ");
		  continue;
	       }
	       break;
	    }
            ptr = read_sector(tar_status.cur_sectno-tar_status.diskoffset);
	 }
	 else
            ptr = read_sector(tar_status.cur_sectno-tar_status.diskoffset);
         if( ptr == 0 ) return -1;
   
         memcpy(buffer, ptr, 512);
      }
      buffer+=512;
      tar_status.cur_sectno++;
   }

   return 0;
}

long 
tar_convert(str, type)
char  str[];
int   type;
{
   register long ac = 0L;
   register int i;

   for (i = 0; i < type; i++)
   {
      if (str[i] >= '0' && str[i] <= '7')
      {
	 ac <<= 3;
	 ac += (long) (str[i] - '0');
      }
   }
   return ac;
}

valid_tar_checksum(sptr)
register HEADER * sptr;
{
   register char *ptr;
   int ac = 0;

   ptr = sptr->hdr_block;
   while (ptr < sptr->hdr_block+sizeof(sptr->hdr_block))
      if( ptr < sptr->member.m_checksum ||
          ptr >= sptr->member.m_checksum+sizeof(sptr->member.m_checksum))
	 ac += (*ptr++ & 0xFF);
      else
	 ptr++, (ac += ' ');

   ac -= tar_convert(sptr->member.m_checksum, sizeof(sptr->member.m_checksum));
   return ac == 0;
}

void
tar_set_drive()
{
#ifdef __STANDALONE__
   disk_spt = __argr.x.si;

   /* Choose some formats, note Boot block only sees a few SPT.
    *      9x40=360k, 15x80=1200k, 18x80=1440k, 21x82=1722k, 36x80=2880k
    */
   if( disk_spt <= 9 )			 disk_cyls = 40;
   if( disk_spt == 21 || disk_spt > 36 ) disk_cyls = 82;
   else if( disk_spt == 32 )             disk_cyls = 1024;
   else					 disk_cyls = 80;
#else
   disk_spt = 18;	/* Testing only */
   disk_cyls= 80;
#endif
   disk_heads=2;

   disk_size = disk_spt*disk_cyls*disk_heads;
}

#if 0

#asm
!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
!
! These are the number of sectors per track that will be scanned for.
! For 3.5 inch floppies 36 is 2.88 Mb, 18 is 1.44Mb, 21 is 1.68Mb on
! a 1.44Mb floppy drive. 15 and 9 are for 5.25 inch floppies.

disksizes: .byte 36,21,18,15,9

! It seems that there is no BIOS call to get the number of sectors.  Guess
! 36 sectors if sector 36 can be read, 18 sectors if sector 18 can be read,
! 15 if sector 15 can be read.  Otherwise guess 9.

export _probe_sectors
_probe_sectors:
	mov	si,#disksizes		! table of sizes to try

probe_loop:
	lodsb
	cbw				! extend to word
	mov	_disk_spt, ax
	cmp	al,#9
	je	got_sectors		! if all else fails, try 9
	xchg	ax, cx			! cx = track and sector
	xor	dx, dx			! drive 0, head 0
	mov	bx,#probe_buf 		! address after setup (es = cs)
	mov	ax,#0x0201		! service 2, 1 sector
	int	0x13
	jc	probe_loop		! try next value
got_sectors:

	ret
#endasm

#endif
