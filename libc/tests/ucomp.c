
/*
 *  Uncompress program this is very very fast 
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>

#define MAXLEN 255

#define     maxno  61000U
#define     USE_BSS

#ifdef USE_BSS
unsigned char  fptr[maxno];
#else
unsigned char *fptr;
#endif
FILE *      fd;
int         key;

main(argc, argv)
int argc;
char ** argv;
{
#ifndef USE_BSS
   fptr = (unsigned char * ) malloc(maxno);

   if( fptr == 0 )
   {
      perror("Cannot allocate memory");
      exit(1);
   }
#endif

   if( argc < 2 )
   {
      fd = stdin;
      key = getc(fd);
      uncompress();
   }
   else
   {
      fd = fopen(argv[1], "r" );
      if( fd == 0 ) { perror("Open failed"); exit(1); }

      key = getc(fd);
      uncompress();
   }
}

/* 

   Uncompression routine -- v.v.fast
*/

uncompress()
{
   register unsigned char * mainscan;
   register unsigned char * secondscan;
   register unsigned char * ptr = (unsigned char * ) fptr;
   register unsigned char * eptr = ptr+maxno;
   register unsigned int len;
   register int ch;

   mainscan = ptr;

   for(;;)
   {
      ch = getc(fd);
      if(ch == EOF) break;
      ch &= 0xFF;
      if(ch == key)
      {
         ch = getc(fd);
         if( ch == 0 ) 
            *mainscan++ = key;
         else
         {
            len = (unsigned char) getc(fd);
            if( ch & 0x80 )
               len += ((unsigned char) getc(fd)) << 8;
            secondscan = mainscan - len;
            if(len > mainscan - ptr) secondscan += maxno;
            len = (unsigned char) ch & 0x7F;
            for( ; len>0; len-- ) 
            {
               *mainscan++ = *secondscan++;
               if( secondscan == eptr ) secondscan = ptr;
               if( mainscan == eptr )
                  { write(1, ptr, (int)(mainscan-ptr)); mainscan = ptr; }
            }
         }
      }
      else
         *mainscan++ = ch;

      if( mainscan == eptr )
         { write(1, ptr, (int)(mainscan-ptr)); mainscan = ptr; }
   }
   if( mainscan != ptr )
      { write(1, ptr, (int)(mainscan-ptr)); mainscan = ptr; }
}

