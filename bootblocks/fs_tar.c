
#ifdef __ELKS__
#include <stdio.h>
#endif

#include <dos.h>
#include "readfs.h"

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
  } member;
} HEADER;

#ifdef __STANDALONE__
extern union REGS __argr;
#endif

tar_open_file(fname)
char * fname;
{
   HEADER * sptr;

#ifdef __STANDALONE__
   if( disk_drive != __argr.h.dl ) return -1;	/* Only the one booted off */
   if( __argr.x.si < 9 || __argr.x.si > 63 ) return -1;	/* SPT good */
#endif

   sptr = read_sector(0);

   /* Boot sector a volume label ? */
   if( sptr->member.m_linked != 'V' ) return -1;
   if( !valid_tar_checksum(sptr) ) return -1;

#ifdef __STANDALONE__
   disk_spt = __argr.x.si;
#else
   disk_spt = 18;	/* Testing only */
#endif

#ifdef __ELKS__
   fprintf(stderr, "Got vaild tar header\n");
#endif

   return -1;
}

tar_rewind_file()
{
   return -1;
}

tar_close_file()
{
   return -1;
}

long
tar_file_length()
{
   return -1;
}

tar_read_block(buffer)
char * buffer;
{
   return -1;
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
HEADER * sptr;
{
   register char *ptr;
   register int ac = 0;

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
