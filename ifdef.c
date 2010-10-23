/* (C) 1992,1996,1998-9 R de Bath */

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifdef __STDC__		/* == Not braindead compiler (hopefully!) */
#include <stdlib.h>

#define P(x) x
#else
#define P(x) ()
#define void char
#endif

/*
 * Linked list of variables, the only source for these is the command line
 * and the (optional) manifest constants.
 */
struct varrec
{
   struct varrec * next;
   int state;
   char name[1];
}
   *varlist = 0;

int cuttype = 0;	/* 0 = use blank lines */
			/* 1 = use #lines */
			/* 2 = use '# 'oldtext */
			/* 3 = delete unused lines */
int striphash = 0;
int stripcomment = 0;

char linebuf[2048];

char * filename;
int  lineno;
int  prline;
FILE * fd;

int iflevel = 0;
int keeptext = 1;
int unknown_stat = 0;

char * commentstr = "#";

char names[16][32];
char state[16];

int  main P((int argc, char ** argv));
void Usage P((char * prog));
void save_name P((char * varname, int state));
void do_file P((char * fname));
void set_line P((int lineno));
int do_hashcom P((void));
int do_ifdef P((char * p, int which));
void check_name P((char * nm));
void fatal P((char * msg));
void manifest_constant P((void));

int
main(argc, argv)
int argc;
char ** argv;
{
   int ar;
   char * ptr;

   if( argc <= 1 ) Usage(argv[0]);

   for(ar=1; ar<argc; ar++) if( argv[ar][0] == '-' )
   {
      if( argv[ar][1] == 'D' || argv[ar][1] == 'U' )
      {
	 if( argv[ar][2] )
	    save_name(argv[ar]+2, argv[ar][1]);
         else
            unknown_stat = argv[ar][1];
      }
      else if( argv[ar][1] == 'C' )
      {
	 cuttype = 2;
         if( argv[ar][2] ) commentstr = argv[ar]+2;
      }
      else for(ptr=argv[ar]+1; *ptr; ptr++) switch(*ptr)
      {
      case 'b': cuttype = 0; break;
      case 'l': cuttype = 1; break;
      case 'C':
      case 'c': cuttype = 2; break;
      case 'r': cuttype = 3; break;
      case 'h': striphash=1; break;

      case 'M': manifest_constant(); break;
      case 'D': unknown_stat = 'D'; break;
      case 'U': unknown_stat = 'U'; break;

      default: Usage(argv[0]);
      }
   }
   else
   {
      do_file(argv[ar]);
   }
   exit(0);
}

void Usage(prog)
char * prog;
{
   fprintf(stderr, "Usage: %s [-DFLAG] [-UFLAG] [-blcrhMDU] [-C##] files\n",
		   prog);
   fprintf(stderr, "\t-DFLAG\tDefine flag.\n");
   fprintf(stderr, "\t-UFLAG\tUndefine flag.\n");
   fprintf(stderr, "\t-C##\tComment out liines with '##'\n");
   fprintf(stderr, "\t-b\tRemove contents of lines leaving empty lines\n");
   fprintf(stderr, "\t-l\tUse #line lines.\n");
   fprintf(stderr, "\t-c\tComment out with '# '\n");
   fprintf(stderr, "\t-r\tRemove undefined lines completely.\n");
   fprintf(stderr, "\t-h\tRemove any line beginning with a '#'\n");
   fprintf(stderr, "\t-M\tInclude manifest constants.\n");
   fprintf(stderr, "\t-D\tAssume any unknown names are defined.\n");
   fprintf(stderr, "\t-U\tAssume any unknown names are undefined.\n");
   exit(1);
}

void
save_name(varname, state)
char * varname;
int state;
{
   struct varrec * curr;
   struct varrec * prev = 0;

   for(curr=varlist;
       curr && strcmp(curr->name, varname) != 0;
       prev=curr, curr=curr->next)
      ;

   if( curr == 0 )
   {
      curr = (struct varrec*) malloc(sizeof(struct varrec)+strlen(varname));
      if( curr == 0 )
	 fatal("Out of memory error");
      if( prev ) prev->next = curr;
      else       varlist = curr;
      curr->next = 0;
      strcpy(curr->name, varname);
   }
   curr->state = state;
}

void
do_file(fname)
char * fname;
{
   filename = fname;
   prline = lineno = 0;
   fd = fopen(fname, "r");
   if( fd == 0 )
      fatal("Cannot open file");

   if( cuttype == 1 ) 
      printf("#line 1 \"%s\"\n", filename);

   /* NB: limited line length */
   while( fgets(linebuf, sizeof(linebuf), fd) != 0 )
   {
      int f = 1;
      char * p = linebuf;
      lineno++;
      while(isspace(*p)) p++;
      if( *p == '#' )
	 f = do_hashcom();
      if(f)
      {
         if( keeptext )
         {
	    if( cuttype == 1 ) set_line(lineno);
	    printf("%s", linebuf);
         }
         else
	 {
	    if( cuttype == 0 ) printf("\n");
	    if( cuttype == 2 ) printf("%s %s", commentstr, linebuf);
         }
      }
   }
   fclose(fd);
   if( iflevel != 0 ) fatal("Not enough endif's");
}

void
set_line(lineno)
int lineno;
{
   if( prline+2 == lineno )
      printf("\n");
   else if( prline+1 != lineno )
      printf("#line %d \"%s\"\n", lineno, filename);
   prline = lineno;
}

int
do_hashcom()
{
   char * p = linebuf;
   int flg = -1;

   while(isspace(*p)) p++;
   if( *p == '#' ) p++;
   while(isspace(*p)) p++;

   if(strncmp(p, "ifdef", 5) == 0)
   {
      do_ifdef(p+5, 'D');
   }
   else
   if(strncmp(p, "ifndef", 6) == 0)
   {
      do_ifdef(p+6, 'U');
   }
   else
   if(strncmp(p, "if", 2) == 0 && isspace(p[2]) )
   {
      state[iflevel++] = keeptext;
      keeptext |= 2;
   }
   else
   if(strncmp(p, "else", 4) == 0)
   {
      if( iflevel == 0 ) fatal("#else at top level");
      if( iflevel && state[iflevel-1]) keeptext ^= 1;
   }
   else
   if(strncmp(p, "endif", 5) == 0)
   {
      flg = (keeptext&2);
      if( iflevel == 0 ) fatal("Too many endif's");
      iflevel--;
      keeptext = state[iflevel];
   }
   else if( !striphash ) return 1;

   if( flg < 0 ) flg = (keeptext&2);

   if( flg ) return 1;

   if( cuttype == 0 ) printf("\n");
   if( cuttype == 2 ) printf("%s %s", commentstr, linebuf);
   return 0;
}

int
do_ifdef(p, which)
char * p;
int which;
{
   char * nm;
   char * s;
   struct varrec * curr;

   while( *p == ' ' || *p == '\t') p++;
   nm = p;

   for(curr=varlist; curr ; curr = curr->next)
   {
      s = curr->name;
      p = nm;
      while( *p == *s ) { p++; s++; }
      if( *s == '\0' && *p <= ' ' )		/* FIXME alphanum */
	 break;
   }
   state[iflevel] = keeptext;
   iflevel++;

   p=nm;
   s=names[iflevel];
   while(*p > ' ') *s++ = *p++;
   *s = '\0';

   if( curr == 0 )
   {
      if( unknown_stat == 0 )
      {
         if( keeptext ) keeptext |= 2;
         return 0;
      }
      if( keeptext )
         keeptext = (unknown_stat == which);
      return 1;
   }
   if( keeptext )
      keeptext = (curr->state == which);
   return 1;
}

#if 0

do_if(p)
char * p;
{
   /*
    * Look for:
    *                 defined Label
    * one sort of     ||   &&
    *                 0
    *                 1
    *
    * Skip characters: WS, (, )
    *
    * Give up on unknown characters and labels without a defined or assumed
    * state. BUT note that if there is another label that forces the state
    * of the result irrespective of an unknown label the state should be set.
    *
    */

static char wbuf[256];

   int combiner   = 0;
   int true_defs  = 0;
   int false_defs = 0;
   int unkn_defs  = 0;

   int state = 0;

   char * d;

   for(;;)
   {
      d=wbuf;
      do
      {
         while( isspace(*p) || (d==wbuf && (*p == '(' || *p == ')' ))) p++;

         if( d!=wbuf && (*p == '(' || *p == ')' )) break;

         while( !(isspace(*p) || *p == '(' || *p == ')' ))
	    if( d<wbuf+sizeof(wbuf)-2) *d++ = *p++;
	    else                       p++;

         *d= 0;
      }
      while( strcmp(wbuf, "!") == 0 );

      if( state == 0 )
      {
	 if( strcmp(wbuf, "defined" ) == 0 )
	 {
	    state = 1;
	    continue;
	 }
	 if( strcmp(wbuf, "!defined" ) == 0 )
	 {
	    state = 2;
	    continue;
	 }
	 if( strcmp(wbuf, "||" ) == 0 || strcmp(wbuf, "&&") == 0 )
	 {
	    if( combiner == 0 ) combiner = wbuf[0];
	    if( combiner != wbuf[0] ) combiner = '@';
	    continue;
	 }
	 /* Something we're not ready for ... */
	 goto Dont_know;
      }
   }
}
#endif

void
check_name(nm)
char * nm;
{
   char * p;
   char * s;
   while(*nm == ' ' || *nm == '\t' ) nm++;
   s = nm;
   if( *s == '\n' || *s == '\r' || *s == '\0' ) return;
   p = names[iflevel];
   while(*p)
   {
      if( *p++ != *s++ )
	 goto x_error;
   }
   if( *s <= ' ' ) return;
x_error:
   fprintf(stderr, "Ifdef mis-nesting Line %d Expect '%s' Got '",
			lineno,
			names[iflevel]);
   while(*nm > ' ' )
      fputc(*nm++, stderr);
   fputc('\'', stderr);
   fputc('\n', stderr);
}

void
fatal(msg)
char * msg;
{
   fprintf(stderr, "Fatal error:%s\n", msg);
   exit(1);
}

/* This places manifest constants defined by the C compiler into ifdef
 *
 * Unfortunatly I can find no way of discovering the variables automatically
 */
void
manifest_constant()
{
/* General */
#ifdef __STDC__
   save_name("__STDC__", 'D');
#endif
#ifdef __GNUC__
   save_name("__GNUC__", 'D');
#endif
#ifdef GNUMAKE
   save_name("GNUMAKE", 'D');
#endif
   if( sizeof(int) < 4 )
      save_name("__SMALL_INTS__", 'D');
   if( sizeof(char *) <= 2 )
      save_name("__SMALL_MEMORY__", 'D');
   if( sizeof(long) == 4 )
      save_name("__LONG_32_BIT__", 'D');
   if( sizeof(long) == 8 )
      save_name("__LONG_64_BIT__", 'D');

/* MSDOS */
#ifdef MSDOS
   save_name("MSDOS", 'D');
#endif
#ifdef __MSDOS__
   save_name("__MSDOS__", 'D');
#endif

/* Linux/unix */
#ifdef __linux__
   save_name("__linux__", 'D');
#ifdef __i386__
   save_name("__elksemu_works__", 'D');
#endif
#endif
#ifdef __unix__
   save_name("__unix__", 'D');
#endif
#ifdef __GNUC__
   save_name("__GNUC__", 'D');
#endif
#ifdef __ELF__
   save_name("__ELF__", 'D');
#endif
#ifdef __i386__
   save_name("__i386__", 'D');
#endif
#ifdef __i486__
   save_name("__i486__", 'D');
#endif
#ifdef i386
   save_name("i386", 'D');
#endif
#ifdef linux
   save_name("linux", 'D');
#endif
#ifdef unix
   save_name("unix", 'D');
#endif

/* BCC */
#ifdef __BCC__
   save_name("__BCC__", 'D');
#endif
#ifdef __AS386_16__
   save_name("__AS386_16__", 'D');
#endif
#ifdef __AS386_32__
   save_name("__AS386_32__", 'D');
#endif

/* AIX, RS6000 */
#ifdef _IBMR2
   save_name("_IBMR2", 'D');
#endif
#ifdef _AIX
   save_name("_AIX", 'D');
#endif
#ifdef _AIX32
   save_name("_AIX32", 'D');
#endif

/* Ugh! Minix is actually _very_ nasty */
#ifdef __minix
   save_name("__minix", 'D');
#endif
/* This isn't much nicer */
#ifdef __CYGWIN__
   save_name("__CYGWIN__", 'D');
#endif
#ifdef __APPLE__
   save_name("__APPLE__", 'D');
#endif
}
