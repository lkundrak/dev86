
#include <stdio.h>
#if __STDC__
#include <stdlib.h>
#endif
#include <ctype.h>
#include <string.h>
#include <malloc.h>

#include "cc.h"

#define MAXINCPATH	5

void print_toks_cpp _P((void));
void print_toks_raw _P((void));
void define_macro _P((char *));
void undefine_macro _P((char *));

char * include_paths[MAXINCPATH];

char last_name[512] = "";
int last_line = -1;
int debug_mode = 0;

char * outfile = 0;
FILE * ofd = 0;

int
main(argc, argv)
int argc;
char ** argv;
{
   int ar, i;
   char * p;
static char Usage[] = "Usage: cpp -E -0 -Dxxx -Uxxx -Ixxx infile -o outfile";

   in_asm = 2;	/* Always in assembler mode */

   for(ar=1; ar<argc; ar++) if( argv[ar][0] == '-') switch(argv[ar][1])
   {
   case 'd': debug_mode = 1; break;
   case 'T': in_asm = 0; break;
   case 'E': /* in_asm = 0; */ break;
   case 'A': ansi_c = 1; break;

	     /* Some options for the code generator. */
   case '0': define_macro("__BCC__");
	     define_macro("__AS386_16__");
	     define_macro("__8086__");
	     break;
   case '3': define_macro("__BCC__");
	     define_macro("__AS386_32__");
	     define_macro("__i386__");
	     break;
   case 'c': define_macro("__CALLER_SAVES__");
	     break;
   case 'f': define_macro("__FIRST_ARG_IN_AX__");
	     break;
   case 'O': define_macro("__OPTIMISED__");
	     break;

   case 'C': /* Keep comments. */
	     cfatal("-C not implemented");
	     break;
   case 'P': /* Supress #line lines. */
	     cfatal("-P not implemented");
	     break;

   case 'I':
	     if (argv[ar][2]) p=argv[ar]+2;
	     else {
		ar++;
		if (ar>=argc) cfatal(Usage);
		p = argv[ar];
	     }
	     for(i=0; i<MAXINCPATH; i++)
		if (!include_paths[i]) {
		   include_paths[i] = p;
		   break;
		}
	     if (i>=MAXINCPATH)
		cfatal("Too many items in include path for CPP");
	     break;
   case 'D':
	     if (argv[ar][2]) p=argv[ar]+2;
	     else {
		ar++;
		if (ar>=argc) cfatal(Usage);
		p = argv[ar];
	     }
	     define_macro(p);
	     break;
   case 'U':
	     if (argv[ar][2]) p=argv[ar]+2;
	     else {
		ar++;
		if (ar>=argc) cfatal(Usage);
		p = argv[ar];
	     }
	     undefine_macro(p);
	     break;
   case 'o':
	     if (argv[ar][2]) p=argv[ar]+2;
	     else {
		ar++;
		if (ar>=argc) cfatal(Usage);
		p = argv[ar];
	     }
	     if (outfile) cfatal(Usage);
	     outfile = p;
	     break;
   default:
	     fprintf(stderr, "CPP Unknown option %s\n", argv[ar]);
	     cfatal(Usage);
   } else if (!curfile) {
      /* Input file */
      curfile = fopen(argv[ar], "r");
      c_fname = argv[ar]; c_lineno = 1;
   } else
      cfatal(Usage);

   if (outfile) ofd = fopen(outfile, "w");
   else         ofd = stdout;
   if (!ofd)
      cfatal("Cannot open output file");

   if (debug_mode)
      print_toks_raw();
   else
      print_toks_cpp();

   if (outfile) fclose(ofd);
   exit(0);
}

void
undefine_macro(name)
char * name;
{
   struct define_item * ptr;

   ptr = read_entry(0, name);
   if (ptr) {
      set_entry(0, name, (void*)0);
      if (!ptr->in_use) free(ptr);
   }
}

void
define_macro(name)
char * name;
{
   char * p;
   char * value;
   struct define_item * ptr;

   if ((p=strchr(name, '=')) != 0) {
      *p = 0;
      value = p+1;
   } else
      value = "1";

   undefine_macro(name);

   ptr = malloc(sizeof(struct define_item) + strlen(value));
   ptr->name = set_entry(0, name, ptr);
   strcpy(ptr->value, value);
   ptr->arg_count = -1;
   ptr->in_use = 0;
   ptr->next = 0;
}

FILE *
open_include(fname, mode, checkrel)
char * fname;
char * mode;
int checkrel;
{
   FILE * fd;
   int i;
   char buf[256];

   if( checkrel )
   {
      fd=fopen(fname,  mode);
      if( fd ) return fd;
   }
    for(i=0; i<MAXINCPATH; i++)
       if (include_paths[i]) {
	 strcpy(buf, include_paths[i]);
	 if (buf[strlen(buf)-1] != '/') strcat(buf, "/");
	 strcat(buf, fname);
	 fd=fopen(buf,  mode);
	 if( fd ) return fd;
       }
   return 0;
}

/*----------------------------------------------------------------------*/

static int outpos = 0;

void
cmsg(mtype, str)
char * mtype;
char * str;
{
   if (mtype) {
      if (c_fname && (*c_fname || c_lineno))
	 fprintf(stderr, "%s %s(%d): %s\n", mtype, c_fname, c_lineno, str);
      else
	 fprintf(stderr, "%s %s\n", mtype, str);
   } else
      fprintf(stderr, "%s\n", str);
}

void
cfatal(str)
char * str;
{
   cmsg("CPP-FATAL error", str);
   exit(255);
}

void
cerror(str)
char * str;
{
   cmsg("CPP error", str);
}

void
cwarn(str)
char * str;
{
   cmsg("CPP warning", str);
}

void
pr_indent(count)
int count;
{
   if(count>10) count=10;
   while(count>0) {fprintf(ofd, "\t"); count--; }
}

void
hash_line()
{
   if( strcmp(last_name, c_fname) != 0 ) last_line = -1;
   if( c_lineno != last_line || last_line <= 0 )
   {
      if( outpos != 0 ) {
	 fputc('\n', ofd); outpos=0; 
	 if (last_line > 0) last_line++;
      }
      while( c_lineno > last_line && 
             c_lineno < last_line+2 &&  /* XXX: Change to 10 */
	     last_line > 0 && 
	    !debug_mode )
      {
	 fputc('\n', ofd); last_line++;
      }

      if( c_lineno != last_line || last_line <= 0 )
      {
	 fprintf(ofd, "# %d", c_lineno);
	 if( last_line <= 0 ) fprintf(ofd, " \"%s\"", c_fname);
	 /* if( last_line > 0 ) fprintf(ofd, " // From line %d", last_line); */
	 fprintf(ofd, "\n");
	 strcpy(last_name, c_fname);
	 last_line = c_lineno;
      }
   }
}

void
print_toks_cpp()
{
   int i;
   int indent=0;
   int paren=0;

   hash_line();
   while( (i=gettok()) != EOF )
   {
      hash_line();
      switch(i)
      {
      case '\n': 
	 cwarn("Newline!?");
	 break;

      case TK_STR:
	 outpos += strlen(curword);
	 fprintf(ofd, "%s", curword);
	 break;

      case TK_COPY:
	 if( outpos ) { fputc('\n', ofd); last_line++; }
	 outpos = 0; last_line++;
	 fprintf(ofd, "#%s\n", curword);
	 break;

      case TK_FILE: sprintf(curword, "\"%s\"", c_fname); if(0) {
      case TK_LINE: sprintf(curword, "%d", c_lineno);
      default: ; }

	 if (!in_asm) {
	    if(i == '}' || i == TK_CASE || i == TK_DEFAULT ) indent--;
	    if(i ==')') paren--;

	    if(outpos) { fputc(' ', ofd); outpos++; }
	    else pr_indent(indent+(paren!=0));

	    if(i == '{' || i == TK_CASE || i == TK_DEFAULT ) indent++;
	    if(i ==';') paren=0;
	    if(i =='(') paren++;
	 }

	 fprintf(ofd, "%s", curword);
	 outpos += strlen(curword);

	 if ( i == '"' )
	 {
	    while((i=gettok()) == TK_STR) {
	       outpos += strlen(curword);
	       fprintf(ofd, "%s", curword);
	    }
	    if (i != '\n') {
	       outpos += strlen(curword);
	       fprintf(ofd, "%s", curword);
	    } else {
	       outpos++;
	       fputc('"', ofd);
	       cerror("Unterminated string");
	    }
	 }
	 break;
      }
   }
   if( outpos ) fputc('\n', ofd);
   outpos = 0;
}

void
print_toks_raw()
{
   int i;
   long val;

   hash_line();
   while( (i=gettok()) != EOF )
   {
      hash_line();
      switch(i)
      {
      default:       fprintf(ofd, "%04x: '", i);
		     {
			char *p;
			for(p=curword; *p; p++)
			   if(isprint(*p) && *p != '\'')
			      fputc(*p, ofd);
			   else
			      fprintf(ofd, "\\x%02x", (unsigned char)*p);
		     }
		     fprintf(ofd, "'\n");
		     break;
      case '"':
		     fprintf(ofd, "QSTR: \"");
		     while((i=gettok()) == TK_STR) {
			if (i == '\n')
			   fprintf(ofd, "\\N");
			else
			   fputc(*curword, ofd), outpos++;
		     }
		     if ( i == '\n' ) fprintf(ofd, "\" --> No terminator\n");
		     else             fprintf(ofd, "\"\n");
		     break;
      case TK_NUM:
		     val = strtoul(curword, (void*)0, 0);
		     fprintf(ofd, "NUMB: %s => %ld\n", curword, val);
		     break;
      case TK_COPY: 
		     fprintf(ofd, "AMSG: #%s\n", curword);
		     break;
      }
   }
}

