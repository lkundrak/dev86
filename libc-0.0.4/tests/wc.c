
#include <stdio.h>
#include <ctype.h>

int   lflag;			/* Want count lines */
int   wflag;			/* Want count words */
int   cflag;			/* Want count characters */

long  lcount;			/* File count of lines */
long  wcount;			/* File count of words */
long  ccount;			/* File count of characters */

long  ltotal;			/* Total count of lines */
long  wtotal;			/* Total count of words */
long  ctotal;			/* Total count of characters */

int
main(argc, argv)
int   argc;
char **argv;
{
   char *p;
   int   ar;

   if (argc > 1 && argv[1][0] == '-')
   {
      for (p = argv[1] + 1; *p; p++)
      {
	 switch (*p)
	 {
	 case 'l':
	    lflag++;
	    break;
	 case 'w':
	    wflag++;
	    break;
	 case 'c':
	    cflag++;
	    break;
	 default:
	    Usage();
	 }
      }
      argc--;
      argv++;
   }

   /* If no flags are set, treat as wc -lwc. */
   if (!lflag && !wflag && !cflag)
      lflag = wflag = cflag = 1;

   /* No filename, use stdin */
   if (argc == 1)
   {
      count(stdin, "");
      exit(0);
   }

   /* There is an explicit list of files.  Loop on files. */
   for (ar = 1; ar < argc; ar++)
   {
      FILE *f;

      if ((f = fopen(argv[ar], "r")) == NULL)
	 fprintf(stderr, "wc: cannot open %s\n", argv[ar]);
      else
      {
	 count(f, argv[ar]);
	 fclose(f);
      }
   }

   if (argc > 2)
   {
      if (lflag)
	 printf("%7ld ", ltotal);
      if (wflag)
	 printf("%7ld ", wtotal);
      if (cflag)
	 printf("%7ld ", ctotal);
      printf("total\n");
   }
   exit(0);
}

count(f, fname)
FILE *f;
char *fname;
{
   register int c;
   register int inword = 0;

   lcount = 0;
   wcount = 0;
   ccount = 0;

   while ((c = getc(f)) != EOF)
   {
      ccount++;

      if (isspace(c))
      {
	 if (inword)
	    wcount++;
	 inword = 0;
      }
      else
	 inword = 1;

      if (c == '\n' || c == '\f')
	 lcount++;
   }

   ltotal += lcount;
   wtotal += wcount;
   ctotal += ccount;

   if (lflag)
      printf("%7ld ", lcount);
   if (wflag)
      printf("%7ld ", wcount);
   if (cflag)
      printf("%7ld ", ccount);
   if (fname && *fname)
      printf("%s", fname);
   printf("\n");
}

Usage()
{
   fprintf(stderr, "Usage: wc [-lwc] [name ...]\n");
   exit(1);
}
