/*
 * This is a Xenix style hex dump command.
 *
 * The 'reverse hex dump' option is an addition that allows a simple 
 * method of editing binary files.
 *
 * The overkill Linux 'hexdump' command can be configured to generate
 * the same format as this command by this shell macro:
 *
 * hd() { hexdump -e '"%06.6_ax:" 8/1 " %02x" " " 8/1 " %02x" "  " ' \
 *                -e '16/1 "%_p" "\n"' \
 *                "$@"
 *      }
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#ifndef MSDOS
#ifndef __BCC__
#include <locale.h>
#endif
#endif

int   lastnum[16] = {-1};
long  lastaddr = -1;
long  offset = 0;

FILE *fd;

FILE * ofd;
char * outfile = 0;
int reverse = 0;

main(argc, argv)
int   argc;
char **argv;
{
   int   done = 0;
   int   ar;
   int   aflag = 1;

#ifndef MSDOS
#ifndef __BCC__
   setlocale(LC_CTYPE, "");
#endif
#endif

   ofd = stdout;

   for (ar = 1; ar < argc; ar++)
      if (aflag && argv[ar][0] == '-')
	 switch (argv[ar][1])
	 {
	 case 'r': /* Reverse */
	    reverse = 1;
	    break;
	 case 's': /* Skip */
	    offset = strtol(argv[ar] + 2, (void *) 0, 0);
	    break;
	 case '-':
	    aflag = 0;
	    break;
	 case 'o': /* Output */
	    if( argv[ar][2] ) outfile = argv[ar]+2;
	    else
	    {
	       if( ++ar >= argc ) Usage();
	       outfile = argv[ar];
	    }
	    break;
	 default:
	    Usage();
	 }
      else
      {
         if( outfile )
	 {
	    if( ofd != stdout ) fclose(ofd);
#ifdef MSDOS
	    if( reverse )
	       ofd = fopen(outfile, "wb");
	    else
#endif
	       ofd = fopen(outfile, "w");
	    if( ofd ==  0 )
	    {
	       fprintf(stderr, "Cannot open file '%s'\n", outfile);
	       exit(9);
	    }
	 }
	 fd = fopen(argv[ar], "rb");
	 if (fd == 0)
	    fprintf(stderr, "Cannot open file '%s'\n", argv[ar]);
	 else
	 {
	    if( reverse )
	       do_rev_fd();
	    else
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
      if( reverse )
         do_rev_fd();
      else
         do_fd();
   }
#endif

   exit(0);
}

Usage()
{
   fprintf(stderr, "Usage: hd [-o Outfile][-r]|[[-sSkip_bytes] file]\n");
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
	 if (isprint(ch))
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
	    fprintf(ofd, "*\n");
	    fflush(ofd);
	 }
	 return;
      }
   }

   lastaddr = address;
   fprintf(ofd, "%06lx:", address);
   for (j = 0; j < 16; j++)
   {
      if (j == 8)
	 fputc(' ', ofd);
      if (num[j] >= 0)
	 fprintf(ofd, " %02x", num[j]);
      else
	 fprintf(ofd, "   ");
      lastnum[j] = num[j];
      num[j] = -1;
   }

   fprintf(ofd, "  %.16s\n", chr);
}


/*
 * This function takes output from hd and converts it back into a binary
 * file
 */

do_rev_fd()
{
   char  str[160];
   char * ptr;
   int   c[16], i, nxtaddr, addr;
   int   zap_last = 1;

   for (i = 0; i < 16; i++)
      c[i] = 0;
   nxtaddr = 0;

   for (nxtaddr = 0;;)
   {
      if (fgets(str, sizeof(str), fd) == NULL)
	 break;

      str[57] = 0;
      ptr = str;

      if( *ptr == '*' ) zap_last = 0;
      if( *ptr != ':' ) {
	 if( !isxdigit(*ptr) ) continue;
	 addr = strtol(ptr, &ptr, 16);
      }
      else 
	 addr = nxtaddr;
      if( *ptr == ':' ) ptr++;

      if (nxtaddr == 0)
	 nxtaddr = addr;
      if( zap_last ) memset(c, 0, sizeof(c));
      else zap_last = 1;
      while (nxtaddr < addr)
      {
	 for (i = 0; nxtaddr < addr && i < 16; i++, nxtaddr++)
	    fputc(c[i], ofd);
      }
      for (i = 0; i < 16 && *ptr; i++)
      {
	 char * ptr2;
	 c[i] = strtol(ptr, &ptr2, 16);
	 if( ptr == ptr2 ) break;
	 fputc(c[i], ofd);
	 ptr = ptr2;
      }
      nxtaddr += 16;
   }
   return 0;
}
