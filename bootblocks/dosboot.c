
#include <stdio.h>

#include "sysboot.v"
#include "msdos.v"
#include "skip.v"

char buffer[512];

struct bblist {
   char * name;
   char * data;
   char * desc;
} bblocks[] = {
   { "bare",  sysboot_data,	"Bare bootblock, lockup if booted"   },
   { "dosfs", msdos_data,	"Boots file BOOTFILE.SYS from dosfs" },
   { "skip",  skip_data,	"Bypasses floppy boot with message"  },
   0
};

char * progname = "";

main(argc, argv)
int argc;
char ** argv;
{
   FILE * fd;
   struct bblist *ptr = bblocks;
   int i;

   progname = argv[0];

   if( argc != 3 ) Usage();

   for(;ptr->name; ptr++) if( strcmp(argv[1], ptr->name) == 0 ) break;
   if( ptr->name == 0 ) Usage();

   fd = fopen(argv[2], "r+");
   if( fd == 0 )
   {
      fprintf(stderr, "Cannot open %s\n", argv[2]);
      exit(1);
   }
   if( fread(buffer, 512, 1, fd) != 1 )
   {
      fprintf(stderr, "Cannot read boot block\n");
      exit(1);
   }
   for(i=0; i<sysboot_dosfs_stat; i++)
      buffer[i] = ptr->data[i];
   for(i=sysboot_codestart; i<512; i++)
      buffer[i] = ptr->data[i];

   rewind(fd);
   if( fwrite(buffer, 512, 1, fd) != 1 )
   {
      fprintf(stderr, "Cannot write boot block\n");
      exit(1);
   }
   fclose(fd);
}

Usage()
{
   struct bblist *ptr = bblocks;

   fprintf(stderr, "Usage: %s bootname /dev/fd0\n", progname);
   fprintf(stderr, "Blocks\n");
   for(;ptr->name; ptr++)
       fprintf(stderr, "\t%s\t%s\n", ptr->name, ptr->desc);
   exit(1);
}
