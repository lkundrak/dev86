
#include "msdos.v"
#include "msdos16.v"

#define DOS_SYSID	0x03
#define DOS_SECT	0x0B
#define DOS_CLUST	0x0D
#define DOS_RESV	0x0E
#define DOS_NFAT	0x10
#define DOS_NROOT	0x11
#define DOS_MAXSECT	0x13
#define DOS_MEDIA	0x15
#define DOS_FATLEN	0x16
#define DOS_SPT		0x18
#define DOS_HEADS	0x1A
#define DOS_HIDDEN	0x1C
#define DOS4_MAXSECT	0x20
#define DOS4_PHY_DRIVE	0x24
#define DOS4_SERIAL	0x27
#define DOS4_LABEL	0x2B
#define DOS4_FATTYPE	0x36

unsigned char buffer[1024];

int drive = 0;

main(argc, argv)
int argc;
char ** argv;
{
   int tries, rv, i;

   if( argc > 1 )
   {
      static char * s = "Usage: lsys [a:]\n";
      if( argc == 2 && argv[1][1] == ':' && argv[1][2] <= ' ' )
      {
         if( argv[1][0] == 'a' || argv[1][0] == 'A' )
	    drive = 0;
         else if( argv[1][0] == 'b' || argv[1][0] == 'B' )
	    drive = 1;
	 else
            fatal(s);
      }
      else
         fatal(s);
   }

   for(tries=0; tries<6; tries++)
      if( (rv = dos_sect_read(drive, 0, 0, 1, buffer)) == 0 )
         break;
   if( rv == 0 )
      for(tries=0; tries<6; tries++)
         if( (rv = dos_sect_read(drive, 0, 0, 2, buffer+512)) == 0 )
            break;
   if( rv ) fatal("Cannot read bootsector");

   if( buffer[DOS_MEDIA] != buffer[512] ||
       buffer[DOS_MEDIA] < 0xF0 ||
       buffer[DOS_NFAT] > 2 ||
       buffer[DOS_SECT+1] != 2 )
      fatal("Floppy has invalid format");

   if( memcmp(buffer+DOS4_FATTYPE, "FAT16", 5) )
   {
      for(i=0; i<msdos_dosfs_stat - msdos_start; i++)
	 buffer[i] = msdos_data[i];
      for(i=msdos_codestart - msdos_start; i<512; i++)
	 buffer[i] = msdos_data[i];
   }
   else
   {
      for(i=0; i<msdos16_dosfs_stat - msdos_start; i++)
	 buffer[i] = msdos16_data[i];
      for(i=msdos_codestart - msdos_start; i<512; i++)
	 buffer[i] = msdos16_data[i];
   }

   for(tries=0; tries<6; tries++)
      if( (rv = dos_sect_write(drive, 0, 0, 1, buffer)) == 0 )
         break;
   if( rv ) fatal("Cannot write bootsector");

   write(0, "Wrote bootsector\r\n", 18);
   return 0;
}

fatal(str)
{
   write(0, str, strlen(str));
   write(0, "\r\n", 2);
   exit(1);
}

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

