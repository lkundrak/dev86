/*
 * bcc.c Version 2001.1
 *       Complete rewrite because the old one was just too confusing!
 *
 *       There are no significant compile time options (MC6809 and CCC
 *       just change defaults) but you should set LOCALPREFIX.
 *
 *       Personality flags are:
 *
 *	-Mn	Normal ELKS
 *	-Md	MSDOS
 *	-Ms	PC Standalone.
 *	-Ml	i386 Linux
 *	-M8	CvW's c386
 *	-M9	MC6809 with bcc
 */
#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#ifndef MSDOS
#include <unistd.h>
#endif
#endif
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef MSDOS
#include <sys/wait.h>
#include <signal.h>
#endif

#ifdef MSDOS
#define LOCALPREFIX     /linux86
#define EXESUF		".exe"
#define R_OK	4		/* Test for read permission.  */
#define W_OK	2		/* Test for write permission.  */
#define X_OK	1		/* Test for execute permission.	 */
#define F_OK	0		/* Test for existence.	*/
#define L_TREE	1		/* Use different tree style */
#define DEFARCH 0		/* Default to 8086 code */
#else
#define EXESUF
#endif

#define AS	"as" EXESUF
#define LD	"ld" EXESUF
#define CPP	"cpp" EXESUF
#define CC1	"cc1" EXESUF
#define OPT	"opt" EXESUF

#define CC1C386 "c386" EXESUF

#define AS09	"as09" EXESUF
#define LD09	"ld09" EXESUF

#define CPPBCC	"bcc-cc1" EXESUF
#define CC1BCC	"bcc-cc1" EXESUF
#define AS86	"as86" EXESUF
#define LD86	"ld86" EXESUF

#define GCC	"gcc"
#define UNPROTO "unproto" EXESUF
#define OPTIM	"copt" EXESUF

#if __STDC__ == 1
#define P(x)	x
#define HASHIT(x) #x
#define QUOT(x) HASHIT(x)
#else
#define P(x)	()
/* Well you find something that works! */
#define QUOT(x) "x"
#endif

struct command {
   char * cmd;
   char * fullpath;
   int  numargs;
   int  maxargs;
   char ** arglist;
} command = { 0,0,0,0,0 };

struct file_list {
   struct file_list * next;
   char * file;
   char * oldfile;
   char * name;
   int    filetype;	/* Char, notional extention of file. */
} * files;

struct opt_list {
   struct opt_list * next;
   char * opt;
   int    opttype;	/* Where the option should go */
} * options;

int opt_v, opt_V, opt_e, opt_x, opt_I, opt_L, opt_W, opt_i,
    opt_O, opt_M;

#ifdef DEFARCH
int opt_arch = (DEFARCH != 0);
#else
int opt_arch = sizeof (char *) >= 4;
#endif

int do_preproc = 1;	/* c -> i */
int do_unproto = 0;	/* i -> i */
int do_compile = 1;	/* i -> s */
int do_optim   = 0;	/* s -> s */
int do_as      = 1;	/* s -> o */
int do_link    = 1;	/* o -> done */
char * executable_name = 0;

int file_count = 0;
int dyn_count = 0;
int error_count = 0;
char * progname = "C";
#ifdef MSDOS
char * tmpdir = "";
#else
char * tmpdir = "/tmp/";
#endif

int main P((int argc, char **argv));
void getargs P((int argc, char **argv));
void add_prefix P((char * path));
void run_aspreproc P((struct file_list * file));
void run_preproc P((struct file_list * file));
void run_unproto P((struct file_list * file));
void run_compile P((struct file_list * file));
void run_optim P((struct file_list * file));
void run_as P((struct file_list * file));
void run_link P((void));
void command_reset P((void));
void command_opt P((char * option));
void command_arch P((void));
void command_opts P((int opykey));
void newfilename P((struct file_list * file, int last_stage, int new_extn, int use_o));
void run_unlink P((void));
void append_file P((char * filename, int ftype));
void append_option P((char * option, int otype));
char * expand_tilde P((char * str));
void * xalloc P((int size));
void Usage P((void));
void fatal P((char * why));
char * copystr P((char * str));
char * catstr P((char * str, char * str2));
#ifdef L_TREE
void reset_localprefix P((void));
#endif
void run_command P((struct file_list * file));

#ifndef LOCALPREFIX
#define LOCALPREFIX     	/usr
#endif
char * localprefix = QUOT(LOCALPREFIX);
#ifndef L_TREE
char * default_include = "-I~/include";
char * default_libdir0 = "-L~/lib/bcc/i86/";
char * default_libdir3 = "-L~/lib/bcc/i386/";
char * optim_rules     = "-d~/lib/bcc/i86";
#else
char * default_include = "-I~/include";
char * default_libdir0 = "-L~/lib/";
char * default_libdir3 = "-L~/lib/i386/";
char * optim_rules     = "-d~/lib";
#endif

char devnull[] = "/dev/null";
char * exec_prefixs[] = {

   /* Place fillers for dynamic fill */
   devnull, devnull, devnull, devnull, devnull,

   "~/lib/bcc/",
#ifdef BINDIR
   QUOT(BINDIR) "/",
#endif
   "~/lib/",
   "~/bin/",
   "/usr/bin/",
   0
};

char * libc = "-lc";

int
main(argc, argv)
int argc;
char ** argv;
{
   struct file_list * next_file;

   progname = argv[0];
#ifdef L_TREE
   reset_localprefix();
#endif
   getargs(argc, argv);

   default_include = expand_tilde(default_include);
   default_libdir0 = expand_tilde(default_libdir0);
   default_libdir3 = expand_tilde(default_libdir3);
   optim_rules     = expand_tilde(optim_rules);

   if (opt_v>1) { command.cmd = ""; command_reset(); }
   
   for(next_file = files; next_file && !error_count; next_file = next_file->next) 
   {
      if (next_file->filetype == 'o') continue;

      if (opt_V)
         fprintf(stderr, "%s:\n", next_file->file);

      /* Assembler that's not to be optimised. */
      if (do_preproc && next_file->filetype == 'S') run_aspreproc(next_file);
      if (do_as      && next_file->filetype == 's') run_as(next_file);

      /* C source */
      if (do_preproc && next_file->filetype == 'c') run_preproc(next_file);
      if (do_unproto && next_file->filetype == 'i') run_unproto(next_file);
      if (do_compile && next_file->filetype == 'i') run_compile(next_file);
      if (do_optim   && next_file->filetype == 's') run_optim(next_file);
      if (do_as      && next_file->filetype == 's') run_as(next_file);

      if (next_file->filetype == '~') error_count++;
   }

   if (do_link && !error_count)
      run_link();

   run_unlink();
   exit(error_count>0);
}

char *
copystr(str)
char * str;
{
   return strcpy(xalloc(strlen(str)+1), str);
}

char *
catstr(str, str2)
char * str, * str2;
{
   return strcat(strcpy(xalloc(strlen(str)+strlen(str2)+1), str), str2);
}

void
run_aspreproc(file)
struct file_list * file;
{
   if (opt_arch<5) command.cmd = CPPBCC;
   else            command.cmd = CPP;
   command_reset();
   newfilename(file, !do_as, 's', (opt_arch<5));
   if (opt_arch<5)
      command_opt("-E");
   command_opts('p');
   command_opt("-D__ASSEMBLER__");
#if 0
   if (!opt_I)
      command_opt(default_include);
#endif
   command_arch();
   run_command(file);
}

void
run_preproc(file)
struct file_list * file;
{
   int last_stage = 0;;

   if (opt_arch<5) command.cmd = CPPBCC;
   else            command.cmd = CPP;
   command_reset();

   if (!opt_e && !do_optim && !do_as )        last_stage =1;
   if (opt_e && !do_unproto && !do_compile )  last_stage =1;

   newfilename(file, last_stage, (opt_e?'i':'s'), (opt_arch<5));

   if (opt_e && opt_arch<5) command_opt("-E");

   command_opts('p');
   if (!opt_e)
   {
      command_opts('c');
      if (opt_arch<5 && !do_as)
	 command_opt("-t");
   }

   if (!opt_I)
      command_opt(default_include);

   command_arch();

   run_command(file);
}

void
run_unproto(file)
struct file_list * file;
{
   command.cmd = UNPROTO;
   command_reset();
   newfilename(file, !do_compile, 'i', 0);
   command_opts('u');

   run_command(file);
}

void
run_compile(file)
struct file_list * file;
{
   if (opt_arch == 3)   command.cmd = CC1C386;
   else if (opt_arch<5) command.cmd = CC1BCC;
   else                 command.cmd = CC1;
   command_reset();
   newfilename(file, !(do_optim || do_as), 's', (opt_arch != 3 && opt_arch<5));

   command_opts('c');

   command_arch();

   run_command(file);
}

void
run_optim(file)
struct file_list * file;
{
   char buf[32];
   if (opt_arch<5) command.cmd = OPTIM;
   else            command.cmd = OPT;
   command_reset();
   newfilename(file, !do_as, 's', 1);
   command_opt("-c!");
   if (opt_O)
   {
      sprintf(buf, "-huse16 %c86", opt_O);
      command_opt(buf);
   }
   command_opt(optim_rules);

   command_opt("rules.start");
   command_opts('o');
   if (opt_O)
   {
      sprintf(buf, "rules.%c86", opt_O);
      command_opt(buf);
   }
   command_opt("rules.86");
   command_opt("rules.end");

   run_command(file);
}

void
run_as(file)
struct file_list * file;
{
   char * buf;
   switch(opt_arch)
   {
   case 0: case 1: case 2:
            command.cmd = AS86; break;
   case 4:  command.cmd = AS09; break;
   default: command.cmd = AS; break;
   }
   command_reset();
   newfilename(file, (!do_link && opt_arch!=2), 'o', 1);
   if (opt_arch==3)
      command_opt("-j");
   if (opt_arch<5)
      command_opt("-u");
   command_opts('a');
   if (opt_W)
      command_opt("-w-");
   else
      command_opt("-w");
   command_arch();
   command_opt("-n");
   buf = catstr(file->name, ".s");
   command_opt(buf);
   free(buf);

   run_command(file);

   if (opt_arch == 2)
   {
      command.cmd = LD86;
      command_reset();
      newfilename(file, !do_link, 'o', 1);
      command_opt("-r");
      run_command(file);
   }
}

void
run_link()
{
   struct file_list * next_file;

   switch(opt_arch)
   {
   case 0: case 1:
            command.cmd = LD86; break;
   case 2:  command.cmd = GCC; break;
   case 4:  command.cmd = LD09; break;
   default: command.cmd = LD; break;
   }
   command_reset();
   if (executable_name == 0) executable_name = "a.out";

   command_opt("-o");
   command_opt(executable_name);

   command_opts('l');
   if (opt_arch != 2) 
   {
      if (opt_arch == 0 && !opt_i)
	 command_opt("-i");

      if (!opt_L)
      {
	 if (opt_arch==1) command_opt(default_libdir3);
	 else             command_opt(default_libdir0);
      }
      command_arch();

      if (!opt_x)
	 command_opt("-C0");
   }

   for(next_file = files; next_file; next_file = next_file->next) 
      command_opt(next_file->file);

   if (opt_arch != 2) 
      command_opt(libc);
   run_command(0);
}

void
command_reset()
{
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif
   char buf[MAXPATHLEN];
   char ** prefix;

   if (command.arglist) 
   {
      int i;
      for(i=0; i<command.maxargs; i++)
         if(command.arglist[i])
	    free(command.arglist[i]);
      free(command.arglist);
   }
   command.arglist = 0;
   command.numargs = 1;
   command.maxargs = 20;

   command.arglist = xalloc(command.maxargs*sizeof(char**));
   command.arglist[0] = copystr(command.cmd);

   if (command.fullpath) free(command.fullpath);
   command.fullpath = 0;

   /* Search for the exe, nb as this will probably be called from 'make'
    * there's not much point saving this.
    */
   for(prefix=exec_prefixs; *prefix; prefix++) 
   {
      char * p;
      if (*prefix == devnull) continue;

      p = strchr(*prefix, '~');
      if (!p) strcpy(buf, *prefix);
      else 
      {
         memcpy(buf, *prefix, p-*prefix);
	 buf[p-*prefix] = 0;

	 strcat(buf, localprefix);
	 strcat(buf, p+1);
      }
      strcat(buf, command.cmd);

      if (!*command.cmd)
	 fprintf(stderr, "PATH+=%s\n", buf);
      else if (access(buf, X_OK) == 0)
      {
         command.fullpath = copystr(buf);
	 break;
      }
   }
   if (!command.fullpath)
      command.fullpath = copystr(command.cmd);
}

void
command_opt(option)
char * option;
{
   if (command.maxargs <= command.numargs+1) {
      char ** newbuf = xalloc(command.maxargs*2*sizeof(char**));
      memcpy(newbuf, command.arglist, command.maxargs*sizeof(char**));
      command.maxargs *= 2;
      free(command.arglist);
      command.arglist = newbuf;
   }

   command.arglist[command.numargs++] = copystr(option);
}

void
command_arch()
{
   if (opt_arch==0) command_opt("-0");
   if (opt_arch==1) command_opt("-3");
   if (opt_arch==2) command_opt("-3");
}

void
command_opts(optkey)
int optkey;
{
   struct opt_list * ol;
   for(ol=options; ol; ol=ol->next)
      if (ol->opttype == optkey)
	 command_opt(ol->opt);
}

void newfilename(file, last_stage, new_extn, use_o)
struct file_list * file;
int last_stage;
int new_extn;
int use_o;
{
   file->filetype = new_extn;
   if (file->oldfile) free(file->oldfile);
   file->oldfile = file->file;
   file->file = 0;

   if (last_stage) {
      if (executable_name)
         file->file = copystr(executable_name);
      else
      {
         char buf[4];
	 buf[0] = '.';
	 buf[1] = file->filetype;
	 buf[2] = 0;
	 file->file = catstr(file->name, buf);
      }
   }
   else
   {
      char buf[16];
#ifdef MSDOS
      sprintf(buf, "$$%05d$", dyn_count++);
#else
      sprintf(buf, "$$%04d%05d", dyn_count++, getpid());
#endif
      file->file = catstr(tmpdir, buf);
   }

   command_opt(file->oldfile);
   /* *.i files go to the stdout */
   if (last_stage && file->filetype == 'i') return;
   if (use_o) command_opt("-o");
   command_opt(file->file);
}

void
run_unlink()
{
   int i;
   for(i=0; i<dyn_count; i++)
   {
      char buf[16];
      char * p;
#ifdef MSDOS
      sprintf(buf, "$$%05d$", i);
#else
      sprintf(buf, "$$%04d%05d", i, getpid());
#endif
      p = catstr(tmpdir, buf);
      if (opt_v>1)
	 fprintf(stderr, "rm %s\n", p);
      if (opt_v>2)
	 continue;
      if (unlink(p) < 0)
      {
	 if (error_count==0 || opt_v>1)
	    fprintf(stderr, "Error unlinking %s\n", p);
	 error_count++;
      }
      free(p);
   }
}

void
getargs(argc, argv)
int argc;
char ** argv;
{
   int ar;
   char * pflag = 0;
   int control_count = 0;
   int exe_count = 0;

   for(ar=1; ar<argc; ) if (argv[ar][0] != '-')
   {
      append_file(argv[ar++], 0);
      file_count++;
   }
   else
   {
      int opt;
      int used_arg = 1, inc_ar=0;
      char * opt_arg;

      if (argv[ar][2]) opt_arg = argv[ar]+2;
      else
      {
         inc_ar++;
         if (argv[ar+1]) opt_arg = argv[ar+1];
	 else
	 {
	    inc_ar++;
	    opt_arg = "ERROR";
	 }
      }
      /* Special case -? is different from -?abcdef */
      if(!pflag && argv[ar][2] == 0) switch(argv[ar][1])
      {
      case 'a': case 'L': case 'M': case 'O': case 'P': case 'Q': 
	 pflag = argv[ar]+1;
	 used_arg = 0;
	 break;
      }
      /* Options that need an argument */
      if(!pflag) switch(argv[ar][1])
      {
      case 'a':
	 if(strcmp(argv[ar], "-ansi") == 0) {
	    do_unproto = 1;
	    opt_e = 1;
	    /* NOTE I'm setting this to zero, this isn't a _real_ STDC */
	    append_option("-D__STDC__=0", 'p');
	 }
	 else 
	    Usage();
	 break;

      case 't':
         append_option("-t", 'a');
	 /*FALLTHROUGH*/
      case 'A':
         append_option(opt_arg, 'a');
	 break;
      case 'C':
         append_option(opt_arg, 'c');
	 break;
      case 'P':
	 append_option(opt_arg, 'p');
	 break;
      case 'X':
         append_option(opt_arg, 'l');
	 break;

      case 'L':
         append_option(argv[ar], 'l');
	 break;

      case 'Q':
         append_option(argv[ar], 'c');
	 break;

      case 'O':
         do_optim=1;
	 if (!opt_arg[1] && ( opt_arg[0] >= '1' && opt_arg[0] <= '3' ))
	    opt_O = opt_arg[0];
	 else
	 {
	    char * p = xalloc(strlen(opt_arg)+8);
	    strcpy(p, "rules.");
	    strcat(p, opt_arg);
	    append_option(p, 'o');
	    free(p);
	 }
	 break;

      case 'o':
	 exe_count++;
	 executable_name = opt_arg;
	 break;

      case 'B':
	 add_prefix(opt_arg);
         break;

      case 'I':
      case 'D':
      case 'U':
         append_option(argv[ar], 'p');
         break;

      case 'T':
	 tmpdir = catstr(opt_arg, "/");
         break;

      case 'M':
	 if (opt_arg[1]) Usage();
	 opt_M    = *opt_arg;
	 break;

      default:
	 pflag = argv[ar]+1;
	 used_arg = 0;
	 break;
      }
      /* Singleton flags */
      if(pflag) switch(opt = *pflag++)
      {
      case 'P':
         append_option("-P", 'p');
	 /*FALLTHROUGH*/
      case 'E':
	 control_count++;
         do_compile = do_link = do_as = 0;
	 opt_e = 1;
	 break;
      case 'S':
	 control_count++;
         do_as = do_link = 0;
	 break;
      case 'c':
	 control_count++;
         do_link = 0;
	 break;
      case 'O':
         do_optim=1;
	 break;

      case 'G': opt_M = 'G'; break;

      case 'v': opt_v++; break;
      case 'V': opt_V++; break;
      case 'e': opt_e++; break;
      case 'x': opt_x++; break;
      case 'I': opt_I++; break;
      case 'L': opt_L++; break;
      case 'i': opt_i++; break;

      case 'W': opt_W++; break;

      case '0': opt_arch=0; opt_M='x'; break;
      case '3': opt_arch=1; opt_M='x'; break;
         
      case 'w': /*IGNORED*/ break;
      case 'g': /*IGNORED*/ break;
      case 'f': /*IGNORED*/ break;
      case 'p': /*IGNORED*/ break;

      default: 
	if (pflag == argv[ar]+2) {
	   /* Special; unknown options saved as flags for the linker */
	   append_file(argv[ar], 'o');
	   pflag = 0;
	}
	else
	   Usage();
      }
      if (!pflag || !*pflag)  { ar++; pflag = 0; } 
      if (used_arg && inc_ar) ar++;
      if (used_arg && inc_ar==2)
         fatal("Last option requires an argument");
   }

   if (control_count>1)
      fatal("only one option from -E -P -S -c allowed");
   if (exe_count>1)
      fatal("only one -o option allowed");

   if (file_count==0) Usage();

   if (exe_count && file_count != 1 && !do_link)
      fatal("only one input file for each non-linked output");

   add_prefix(getenv("BCC_EXEC_PREFIX"));

#ifdef MC6809
   if (opt_M==0) opt_M = '9';
#endif
#ifdef CCC
   if (opt_M==0) opt_M = '8';
#endif
#ifdef MSDOS
   if (opt_M==0) opt_M = 'd';
#endif
   if (opt_M==0) opt_M = (opt_arch==1 ?'l':'n');
   switch(opt_M)
   {
   case 'n': 
      append_option("-D__ELKS__", 'p');
      append_option("-D__unix__", 'p');
      libc="-lc";
      break;
   case 'f': 
      append_option("-D__ELKS__", 'p');
      append_option("-D__unix__", 'p');
      append_option("-c", 'p');
      append_option("-f", 'p');
      libc="-lc_f";
      break;
   case 'c': 
      append_option("-D__ELKS__", 'p');
      append_option("-D__unix__", 'p');
      append_option("-c", 'p');
      libc="-lc";
      break;
   case 's': 
      append_option("-D__STANDALONE__", 'p');
      libc="-lc_s";
      break;
   case 'd': 
      append_option("-D__MSDOS__", 'p');
      libc="-ldos";
      append_option("-d", 'l');
      append_option("-T100", 'l');
      break;
   case 'l': 
      opt_arch=1;
      append_option("-D__linux__", 'p');
      append_option("-D__unix__", 'p');
      libc="-lc";
      append_option("-N", 'l');
      break;
   case 'G':
      opt_arch = 2;
      break;
   case '8':
      opt_arch = 3;
      opt_e = 1;
      break;
   case '9':
      opt_arch = 4;
      default_libdir0 = "-L~/lib/bcc/m09/";
      optim_rules     = "-d~/lib/bcc/m09";
      add_prefix("~/lib/bcc/m09/");
      break;
   case '0':
      opt_arch = 5;
      opt_e = 1;
      opt_I = 1;
      opt_L = 1;
      opt_x = 1;
      append_option("/lib/crt0.o", 'l');
      break;
   }

   if (do_optim)
   {
      if (opt_e) 
	 append_option("-O", 'c');
      append_option("-O", 'p');
      append_option("-O", 'a');
   }
}

void
add_prefix(path)
char * path;
{
   char ** p; 
   if (!path || !*path) return;

   for(p=exec_prefixs; *p; p++) {
      if( *p == devnull )
      {
	 *p = path;
	 break;
      }
   }
   if (!*p) fatal("Too many -B options");
}

void append_file (filename, ftype)
char * filename;
int ftype;
{
   struct file_list * newfile = xalloc(sizeof(struct file_list));
   char * s;
   char * name;

   newfile->file = copystr(filename);
   name = copystr(filename);

   s = strrchr(name, '.');

   if (ftype) 
   {
      newfile->name = copystr(name);
      newfile->filetype = ftype;
   }
   else if (s && s == name + strlen(name) - 2) {
      newfile->filetype = s[1];
      *s = 0;
      newfile->name = copystr(name);
   }
   else
      newfile->name = copystr(name);
   free(name);

   if (newfile->filetype == 0)   newfile->filetype = 'o'; /* Objects */

   if (files==0)
      files = newfile;
   else
   {
      struct file_list * fptr;
      for(fptr=files; fptr->next; fptr=fptr->next);
      fptr->next = newfile;
   }
}

void 
append_option (option, otype)
char * option;
int otype;
{
   struct opt_list * newopt = xalloc(sizeof(struct opt_list));

   newopt->opt = copystr(option);
   newopt->opttype = otype;

   if (options==0)
      options = newopt;
   else
   {
      struct opt_list * optr;
      for(optr=options; optr->next; optr=optr->next);
      optr->next = newopt;
   }
}

char * expand_tilde(str)
char * str;
{
   char * newstr;
   char * ptr = strchr(str, '~');
   if( ptr == 0 ) return copystr(str);

   newstr = xalloc(strlen(str)+strlen(localprefix));
   if( ptr!=str ) memcpy(newstr, str, ptr-str);
   strcpy(newstr+(ptr-str), localprefix);
   strcat(newstr, ptr+1);
   return newstr;
}

void *
xalloc (size)
int size;
{
   void * p = malloc(size);
   if (!p) fatal("Out of memory");
   memset(p, '\0', size);
   return p;
}

void Usage()
{
   fatal("Usage: bcc [-ansi] [-options] [-o output] file [files]");
}

void fatal(str)
char * str;
{
   fprintf(stderr, "%s: Fatal error: %s.\n", progname, str);
   exit(1);
}

#ifdef L_TREE
#ifdef MSDOS
void reset_localprefix()
{
   char *ptr, *temp;

   temp = copystr(progname);
   if( (ptr = strrchr(temp, '\\')) != 0
         && temp<ptr-4 && strncmp(ptr-4, "\\BIN", 4) == 0 )
   {
      ptr[-4] = 0;
      localprefix = temp;
   }
   else
      free(temp);
}
#else

void reset_localprefix()
{
   char *ptr, *temp;

   if( *progname == '/' )
      temp = copystr(progname);
   else
   {
      char * s, * d;
      ptr = getenv("PATH");
      if( ptr==0 || *ptr == 0 ) return;
      ptr = copystr(ptr);
      temp = copystr("");

      for(d=s=ptr; d && *s; s=d)
      {
#ifdef MAXPATHLEN
         char buf[MAXPATHLEN];
#else
         char buf[1024];
#endif

	 free(temp);
         d=strchr(s, ':');
	 if( d ) *d='\0';
	 temp = xalloc(strlen(progname)+strlen(s)+2);
	 strcpy(temp, s);
	 strcat(temp, "/");
	 strcat(temp, progname);
         if( realpath(temp, buf) != 0 )
	 {
	    free(temp);
	    temp = copystr(buf);
         }
	 if( access(temp, X_OK) == 0 ) break;
	 d++;
      }
      if( s == 0 )
      {
         free(temp);
	 temp = copystr(progname);
      }
      free(ptr);
   }

   if( (ptr = strrchr(temp, '/')) != 0
         && temp<ptr-4 && strncmp(ptr-4, "/bin", 4) == 0 )
   {
      ptr[-4] = 0;
      localprefix = temp;
   }
   else
      free(temp);
}
#endif
#endif


void
run_command(file)
struct file_list * file;
{
#ifdef __BCC__
static char ** minienviron[] = {
   "PATH=/bin:/usr/bin",
   "SHELL=/bin/sh",
   0
};
#endif
   int i, status;
#ifndef MSDOS
   void *oqsig, *oisig, *otsig, *ocsig;
#endif


   if (opt_v)
   {
      fprintf(stderr, "%s", command.fullpath);
      for(i=1; command.arglist[i]; i++)
	 fprintf(stderr, " %s", command.arglist[i]);
      fprintf(stderr, "\n");
      if (opt_v>2) return;
   }

#ifdef MSDOS
   status = spawnv(0, command.fullpath, command.arglist);
   if (status<0)
   {
      fprintf(stderr, "Unable to execute %s\n", command.fullpath);
   }
#else
   oqsig = signal(SIGQUIT, SIG_IGN);
   oisig = signal(SIGINT,  SIG_IGN);
   otsig = signal(SIGTERM,  SIG_IGN);
   ocsig = signal(SIGCHLD, SIG_DFL);

   switch(fork())
   {
   case -1:
      fatal("Forking failure");
   case 0:
      (void) signal(SIGQUIT, SIG_DFL);
      (void) signal(SIGINT,  SIG_DFL);
      (void) signal(SIGTERM, SIG_DFL);
      (void) signal(SIGCHLD, SIG_DFL);

#ifdef __BCC__
      execve(command.fullpath, command.arglist, minienviron);
#else
      execv(command.fullpath, command.arglist);
#endif
      fprintf(stderr, "Unable to execute %s.\n", command.fullpath);
      exit(1);
   default:
      wait(&status);
      if (status&0xFF)
      {
	 fprintf(stderr, "%s: killed by signal %d\n", 
	                 command.fullpath, (status&0xFF));
      }
   }

   (void) signal(SIGQUIT, oqsig);
   (void) signal(SIGINT,  oisig);
   (void) signal(SIGTERM, otsig);
   (void) signal(SIGCHLD, ocsig);
#endif
   if (status && file)
      file->filetype = '~';
}

