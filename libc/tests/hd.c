
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

int   lastnum[16] = {-1};
long  lastaddr = -1;
long  offset = 0;

FILE *fd;

main(argc, argv)
int   argc;
char **argv;
{
   int   done = 0;
   int   ar;
   int   aflag = 1;

   for (ar = 1; ar < argc; ar++)
      if (aflag && argv[ar][0] == '-')
	 switch (argv[ar][1])
	 {
	 case 'r':
	    return reverse_hd(argc, argv);
	 case 'o':
	    offset = strtol(argv[ar] + 2, (void *) 0, 0);
	    break;
	 case '-':
	    aflag = 0;
	    break;
	 default:
	    Usage();
	 }
      else
      {
	 fd = fopen(argv[ar], "rb");
	 if (fd == 0)
	    fprintf(stderr, "Cannot open file '%s'\n", argv[ar]);
	 else
	 {
	    do_fd();
	    fclose(fd);
	 }
	 done = 1;
      }

   if (!done)
#ifdef MSDOS
      Usage();
#else
   {
      fd = stdin;
      do_fd();
   }
#endif
}

Usage()
{
   fprintf(stderr, "Usage: hd [-r]|[[-oOffset] file]\n");
   exit(1);
}

do_fd()
{
   int   j, ch;
   char  buf[20];
   int   num[16];

   if (offset)
      fseek(fd, offset, 0);

   for (ch = 0; ch != EOF; offset += 16)
   {
      memset(buf, '\0', 16);
      for (j = 0; j < 16; j++)
	 num[j] = -1;
      for (j = 0; j < 16; j++)
      {
	 ch = fgetc(fd);
	 if (ch == EOF)
	    break;

	 num[j] = ch;
	 if (isascii(ch) && isprint(ch))
	    buf[j] = ch;
	 else
	    buf[j] = '.';
      }
      printline(offset, num, buf, ch == EOF);
   }
}

printline(address, num, chr, eofflag)
long  address;
int  *num;
char *chr;
int   eofflag;
{
   int   j;

   if (lastaddr >= 0)
   {
      for (j = 0; j < 16; j++)
	 if (num[j] != lastnum[j])
	    break;
      if (j == 16 && !eofflag)
      {
	 if (lastaddr + 16 == address)
	 {
	    printf("*\n");
	    fflush(stdout);
	 }
	 return;
      }
   }

   lastaddr = address;
   printf("%06lx:", address);
   for (j = 0; j < 16; j++)
   {
      if (j == 8)
	 putchar(' ');
      if (num[j] >= 0)
	 printf(" %02x", num[j]);
      else
	 printf("   ");
      lastnum[j] = num[j];
      num[j] = -1;
   }

   printf("  %.16s\n", chr);
}


/*
 * This function takes output from hd and converts it back into a binary
 * file
 */

/* --   0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f */
static char *datafmt = "%x: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n";
reverse_hd()
{
   char  str[160];
   char * ptr;
   int   c[16], d[16], x, i, nxtaddr, addr;

   for (i = 0; i < 16; i++)
      c[i] = 0;
   nxtaddr = 0;

   for (nxtaddr = 0;;)
   {
      if (gets(str) == NULL)
	 break;

      str[57] = 0;
      ptr = str;

      if( !isxdigit(*ptr) ) continue;
      addr = strtol(ptr, &ptr, 16);
      if( *ptr == ':' ) ptr++;

      if (nxtaddr == 0)
	 nxtaddr = addr;
      while (nxtaddr < addr)
      {
	 nxtaddr += 16;
	 for (i = 0; i < 16; i++)
	    putchar(c[i]);
      }
      for (i = 0; i < 16 && *ptr; i++)
      {
	 char * ptr2;
	 c[i] = strtol(ptr, &ptr2, 16);
	 if( ptr == ptr2 ) break;
	 putchar(c[i]);
	 ptr = ptr2;
      }
      nxtaddr += 16;
   }
   return 0;
}
