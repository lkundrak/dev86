#include <stdio.h>
#include <ctype.h>

int  lastnum[16] = { -1 };
long lastaddr = -1;

main(argc, argv)
int argc;
char ** argv;
{
   FILE * fd;
   int j, ch;
   char buf[20];
   int  num[16];
   long offset = 0;

#ifndef MSDOS
   if( argc == 1 )
   {
      fd = stdin;
   }
   else
#endif
   {
      if( argc == 3 ) offset = strtol(argv[2], (char*)0, 16);
      else if( argc != 2 )
      {
         fprintf(stderr, "Usage: hd file [hexoffset]\n");
         exit(1);
      }
      fd = fopen(argv[1], "rb");
      if( fd == 0 )
      {
         fprintf(stderr, "Cannot open file '%s'\n", argv[1]);
         exit(1);
      }
   }

   /* if( offset ) fseek(fd, offset, 0); */

   for(ch=0; ch!=EOF; offset+=16)
   {
      memset(buf, '\0', 16);
      for(j=0; j<16; j++) num[j] = -1;
      for(j=0; j<16; j++)
      {
         ch = fgetc(fd);
         if( ch == EOF ) break;

         num[j] = ch;
         if( isascii(ch) && isprint(ch) ) buf[j] = ch;
         else buf[j] = '.';
      }
      printline(offset, num, buf, ch==EOF);
   }
   fclose(fd);
}

printline(address, num, chr, eofflag)
long address;
int * num;
char * chr;
int eofflag;
{
   int j;

   if( lastaddr >= 0 )
   {
      for(j=0; j<16; j++)
         if( num[j] != lastnum[j] )
            break;
      if( j == 16 && !eofflag ) return;
      if( lastaddr+16 != address )
         printf("*\n");
   }

   lastaddr = address;
   printf("%06lx:", address);
   for(j=0; j<16; j++)
   {
      if( num[j] >= 0 )
         printf(" %02x", num[j]);
      else
         printf("   ");
      lastnum[j] = num[j];
      num[j] = -1;
   }

   printf("  %.16s\n", chr);
}
