
#include <stdio.h>

#include "tarboot.v"
#include "sysboot.v"

char tarblock[512];

char *bootblock = tarboot_data;

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

#define head     (*(struct tar_head*) tarblock)
#define boothead (*(struct tar_head*) bootblock)

int force = 0;
int relocatable = 0;
int tableload = 0;
long loadaddress = 0x8000;
long execaddress = 0x8000;

main(argc, argv)
int argc;
char ** argv;
{
   int ar;
   int done=0;

   if( sizeof(head) != sizeof(tarblock) )
      { fprintf(stderr, "Program structure error\n"); exit(1); }

   for(ar=1; ar<argc; ar++) if(argv[ar][0]=='-') switch(argv[ar][1])
   {
   case 'f': force++; break;
   case 'r': relocatable++; break;
   case 't': tableload++; break;
   case 'l': sscanf(argv[ar]+2, "%li", &loadaddress); break;
   case 'x': sscanf(argv[ar]+2, "%li", &execaddress); break;

   case '?': Usage(1); break;
   default:  Usage(0); break;
   }
   else
   {
      mktarboot(argv[ar]);
      done++;
   }
}

Usage(flg)
int flg;
{
   fprintf(stderr, "Usage:  makeboot [-f] device\n");
   exit(9);
}

mktarboot(fname)
char * fname;
{
   FILE * fd;

   fd = fopen(fname, "r+");
   if( !fd ) { perror(fname); exit(1); }

   if( fread(tarblock, sizeof(tarblock), 1, fd) != 1 )
   { fprintf(stderr, "Cannot read boot block\n"); exit(1); }

   check_tar(1);
   mangle_tarvol();

   rewind(fd);
   if( fwrite(tarblock, sizeof(tarblock), 1, fd) != 1 )
   { fprintf(stderr, "Cannot write boot block\n"); exit(1); }

   rewind(fd);
   if( fread(tarblock, sizeof(tarblock), 1, fd) != 1 )
   { fprintf(stderr, "Cannot re-read boot block\n"); exit(1); }

   if( fread(tarblock, sizeof(tarblock), 1, fd) == 1 && head.linkflag == '0' )
      printf("Boot block installed to boot file %s\n", head.name);

   fclose(fd);
   exit(0);
}

unsigned int oct(s)
char *s;
{
   unsigned int val = 0;
   int i;
   for(i=0; i<8; i++) if( s[i] >= '0' && s[i] <= '7' )
      val = (val<<3) + s[i] - '0';
   return val;
}

check_tar(fatal)
int fatal;
{
   char vbuf[100], *p;
   unsigned int csum = 0;
   long osum = -1;

   osum = oct(head.chksum);
   memset(head.chksum, ' ', sizeof(head.chksum));

   for(p=tarblock; p<tarblock+sizeof(tarblock); p++)
      csum += (*p & 0xFF);

   if( csum != osum )
   {
      fprintf(stderr, "TAR file checksum failed, this isn't a tar file.\n");
      if(fatal) exit(9);
      return -1;
   }
   if( head.linkflag != 'V' )
   {
      fprintf(stderr, "Tar file doesn't start with a volume label\n");
      if(fatal) exit(8);
      return -1;
   }

   strcpy(vbuf, boothead.name); strcat(vbuf, " Volume 1");
   if( !force && strcmp(boothead.name, head.name) != 0 
    && strcmp(vbuf, head.name) != 0 )
   {
      fprintf(stderr, "Volume is labeled as '%s' not '%s'\n",
                       head.name, boothead.name);
      fprintf(stderr, "Use -f flag to force write\n");
      if(fatal) exit(1);
      return -1;
   }
   return 0;
}

mangle_tarvol()
{
   char lbuf[20];
   char * p;
   unsigned int csum = 0;
   int i;

   struct tar_head temp;

   temp = boothead;

   /* Copy preserved fields
    */
   memcpy(temp.mtime, head.mtime, sizeof(temp.mtime));

   memset(temp.name, 0x90, 16);
   for(i=0; head.name[i] && head.name[i] != ' ' && i<14; i++)
   {
      int ch = head.name[i];
      if( islower(ch) ) ch = toupper(ch);
      if( strchr("/?@ABCDEFGHIJKLMNO", ch) == 0 )
         ch = '?';
      temp.name[i] = ch;
   }
   temp.name[i++] = 0;
   temp.name[i]   = 0xC0;

   head = temp;
   /* Calculate the checksum */
   memset(head.chksum, ' ', sizeof(head.chksum));

   for(p=tarblock; p<tarblock+sizeof(tarblock); p++)
      csum += (*p & 0xFF);

   sprintf(head.chksum, "%7o", csum);
}

