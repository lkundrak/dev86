
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>

#define MAXNO  32767
#define MAXLEN 127
#define XXQSCAN	/* Speed up scanning at the cost of not being optimal */

unsigned char  *fptr;
unsigned short *vptr;
FILE * fd;

#define ITBSIZE  4096
#define itbfunc() (ptr[mainscan]^(ptr[mainscan+1]<<4)^(ptr[mainscan+2]<<2))
/*
#define ITBSIZE  4001
#define itbfunc() ((ptr[mainscan]+ptr[mainscan+1]*79+ptr[mainscan+2]*307)%4001)
*/
int * itb;

int        size;
long        cnt=0;
#ifdef __GNUC__
int        maxno= 16384;
#else 
int        maxno= 8000;
#endif 

long icount = 0;
long ocount = 0;

unsigned char key;

int fl;

main(argc, argv)
int argc;
char ** argv;
{
   if( argc < 2 ) { fprintf(stderr, "Usage; ... \n"); exit(1); }

   if( argc == 3 )
   {
      maxno = atoi(argv[2]);
      if( maxno < 256  ) maxno = 256;
      if( maxno > MAXNO) maxno = MAXNO;
   }

   if( strcmp(argv[1], "-") == 0 )
      fd = stdin;
   else
      fd = fopen(argv[1], "r" );
   if( fd == 0 ) { perror("Open failed\n"); exit(1); }

   fptr = (unsigned char * ) malloc((unsigned)maxno*2);
   itb  = (int * ) malloc(ITBSIZE*sizeof(int));
   if( itb )
      vptr = (unsigned short * ) malloc((unsigned)maxno * sizeof(short)*2 );
   else
      vptr = 0;

   if( fptr == 0 )
   {
      perror("Cannot allocate RAM");
      exit(1);
   }
   if( vptr == 0 && itb ) free(itb);

   fl = 0;
   {
      if( (size = fread(fptr, 1, (int)maxno, fd )) < 0 ) { fprintf(stderr, "\nRead failed\n"); exit(1); }

      if( size )
      {
	 icount += size;
         if( fl == 0 )
         {
            key = scan_min();
            putchar(key); ocount++;
            fl = 1;
         }
         else
            fprintf(stderr, "\rBlock %d \r", fl++ );
         if( vptr) compress();
	 else      slo_compress();
      }
   }

   fprintf(stderr, "\n");
   exit(0);
}

scan_min()
{
   long count[256];
   long i;
   int j, n;

   for( j=0; j<256; j++ ) count[j] = 0;

   for( i=0; i<size; i++) count[ fptr[i] & 0xFF ]++;

   for( i= (((unsigned long) -1) >> 1), j=0; j<256; j++ )
      if( count[j] < i )
      {
         i = count[j] ;
         n = j;
      }

   fprintf(stderr, "Most unused in 0x%lx to 0x%lx is 0x%02x at %ld\n", cnt, cnt+size, n, i );
   cnt+= size;

   return n;
}

compress()
{
   register long mainscan;
   register long secondscan;
   register unsigned char * ptr = (unsigned char * ) fptr;
   register int len;
   register int matchlen;
   long notepos;
   long emark;
#ifdef QSCAN
   int count;
#endif

   for( mainscan=0; mainscan <ITBSIZE; itb[mainscan++] = -1 );

   mainscan=0;
   emark = size - 130 ;
loopback:

   for( ; mainscan < emark; )
   {
      matchlen = 3;
      notepos  = -1;
#ifdef QSCAN
      count = 0;
#endif
      for( secondscan=itb[itbfunc()];
           secondscan >= 0 && mainscan - secondscan < maxno;
           secondscan -= vptr[secondscan] )
      {
#ifdef DEBUG
if( vptr[secondscan] == 0 )
{
   fprintf(stderr, "\nOh !!!!! mainsc %ld, sec-scan %ld\n", mainscan, secondscan);
   vptr[secondscan] = secondscan+1;
}
#endif

         for( len = 0; len < MAXLEN ; len++ )
            if( mainscan+len >= size || ptr[mainscan+len] != ptr[secondscan+len] ) break;
         if( len > matchlen && (len != 4 || mainscan - secondscan < 256 ) )
         {
            notepos = secondscan;
            matchlen = len;
            if( len == MAXLEN ) break;
         }
#ifdef QSCAN
         if( matchlen > 20 && len > 3 && ++count > 5 )
	    break;
#endif
      }

      if( notepos == -1 )
      {
         if( ptr[mainscan] == key )
         {
	    ocount+=2;
            putchar(key);
            putchar(0);
         }
         else
	 {
	    ocount++;
            putchar(ptr[mainscan]);
	 }
         matchlen = 1;
      }
      else
      {
         long x = mainscan - notepos;
	 ocount+=3;
         putchar(key);
         if( x > 255 ) putchar(matchlen | 0x80);
         else putchar(matchlen);
         putchar((int)x);
         if( x > 255 ) { putchar((int)x>>8); ocount++; }
      }

      while( matchlen-- ) 
      {
         len = itbfunc();
         vptr[mainscan] = mainscan - itb[len];
#if 1
	 if( vptr[mainscan] == 0 )
	 {
	    fprintf(stderr, "\nHumm.. ms=%ld, hash=%d, itb[hash]=%ld\n", mainscan, len, itb[len]);
	    vptr[mainscan] = mainscan+1;
	 }
#endif
         itb[len] = mainscan;
         mainscan++;
      }
   }

   fprintf(stderr, "\rBlock %d  ..In:%ld Out:%ld  \r", fl-1, icount, ocount );

   if( emark < size-4 )
   {
      int cnt;
      long l ;
      if(mainscan > maxno )
      {
	 for(cnt=0; cnt<ITBSIZE; cnt++)
	 {
	    if( itb[cnt] < maxno) itb[cnt] = -1;
	    else                  itb[cnt] -= maxno;
	 }
	 for(l=0; l<maxno; l++)
	 {
	    ptr[l] = ptr[l+maxno];
	    vptr[l] = vptr[l+maxno];
	 }
	 mainscan -= maxno;
	 size -= maxno;
      }
      if( size <= maxno )
      {
	 if(( cnt = fread(ptr+size, 1, (int)maxno, fd)) < 0 )
            { fprintf(stderr, "\nRead failed\n"); exit(1); }
	 size += cnt;
	 icount += cnt;
         fprintf(stderr, "\rBlock %d \r", fl++ );
      }
      emark = size - 130;
      if( mainscan >= emark )
	 emark = size -4;

      goto loopback;
   }

   for( ; mainscan < size; )
   {
      if( ptr[mainscan] == key )
      {
	 ocount+=2;
         putchar(key);
         putchar(0);
      }
      else
      {
	 ocount++;
         putchar(fptr[mainscan]);
      }
      mainscan++;
   }
   fprintf(stderr, "\rBlock %d  ..In:%ld Out:%ld   \r", fl-1, icount, ocount );
   /* end */
}

slo_compress()
{
   register long mainscan;
   register long secondscan;
   register unsigned char * ptr = (unsigned char * ) fptr;
   register int len;
   register int matchlen;
   long notepos;
   long emark;
#ifdef QSCAN
   int count;
#endif

   mainscan=0;
   emark = size - 130 ;
loopback:

   for( ; mainscan < emark; )
   {
      matchlen = 3;
      notepos  = -1;
#ifdef QSCAN
      count = 0;
#endif
      for( secondscan=mainscan-1;
           secondscan >= 0 && mainscan - secondscan < maxno;
           secondscan-- )
      {
         for( len = 0; len < MAXLEN ; len++ )
            if( mainscan+len >= size || ptr[mainscan+len] != ptr[secondscan+len] ) break;
         if( len > matchlen && (len != 4 || mainscan - secondscan < 256 ) )
         {
            notepos = secondscan;
            matchlen = len;
            if( len == MAXLEN ) break;
         }
#ifdef QSCAN
         if( matchlen > 20 && len > 3 && ++count > 5 )
	    break;
#endif
      }

      if( notepos == -1 )
      {
         if( ptr[mainscan] == key )
         {
	    ocount+=2;
            putchar(key);
            putchar(0);
         }
         else
	 {
	    ocount++;
            putchar(ptr[mainscan]);
	 }
         matchlen = 1;
      }
      else
      {
         long x = mainscan - notepos;
	 ocount+=3;
         putchar(key);
         if( x > 255 ) putchar(matchlen | 0x80);
         else putchar(matchlen);
         putchar((int)x);
         if( x > 255 ) { putchar((int)x>>8); ocount++; }
      }

      mainscan += matchlen;
   }

   fprintf(stderr, "\rBlock %d  ..In:%ld Out:%ld  \r", fl-1, icount, ocount );

   if( emark < size-4 )
   {
      int cnt;
      long l ;
      if(mainscan > maxno )
      {
	 for(l=0; l<maxno; l++)
	 {
	    ptr[l] = ptr[l+maxno];
	 }
	 mainscan -= maxno;
	 size -= maxno;
      }
      if( size <= maxno )
      {
	 if(( cnt = fread(ptr+size, 1, (int)maxno, fd)) < 0 )
            { fprintf(stderr, "\nRead failed\n"); exit(1); }
	 size += cnt;
	 icount += cnt;
         fprintf(stderr, "\rBlock %d \r", fl++ );
      }
      emark = size - 130;
      if( mainscan >= emark )
	 emark = size -4;

      goto loopback;
   }

   for( ; mainscan < size; )
   {
      if( ptr[mainscan] == key )
      {
	 ocount+=2;
         putchar(key);
         putchar(0);
      }
      else
      {
	 ocount++;
         putchar(fptr[mainscan]);
      }
      mainscan++;
   }
   fprintf(stderr, "\rBlock %d  ..In:%ld Out:%ld   \r", fl-1, icount, ocount );
   /* end */
}

