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
 *	-M0	A framework for the -B option.
 */
#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#ifndef MSDOS
#include <unistd.h>
#endif
#else
#include <malloc.h>
#endif
#include <string.h>
#include <memory.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef MSDOS
#include <sys/wait.h>
#include <signal.h>
#endif
#include "version.h"

#ifdef MSDOS
#define LOCALPREFIX     /linux86
#define EXESUF		".exe"
#define R_OK	4		/* Test for read permission.  */
#define W_OK	2		/* Test for write permission.  */
#define X_OK	1		/* Test for execute permission.	 */
#define F_OK	0		/* Test for existence.	*/
#define DEFARCH 0		/* Default to 8086 code */
#include "version.h"
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

#define CPPBCC	"bcc-cpp" EXESUF
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
   char * altcmd;
   char * fullpath;
   int  numargs;
   int  maxargs;
   char ** arglist;
} command = { 0,0,0,0,0,0 };

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
    opt_O, opt_M, opt_f;

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
void build_prefix P((char * path1, char * path2, char * path3));
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
void validate_link_opts P((void));
void append_file P((char * filename, int ftype));
void append_option P((char * option, int otype));
void prepend_option P((char * option, int otype));
char * build_libpath P((char * opt, char * str, char * suffix));
void * xalloc P((int size));
void Usage P((void));
void fatal P((char * why));
char * copystr P((char * str));
char * catstr P((char * str, char * str2));
void reset_prefix_path P((void));
void run_command P((struct file_list * file));

char * prefix_path = "";

#ifdef LOCALPREFIX
char * localprefix  = QUOT(LOCALPREFIX);
#else
char * localprefix  = "/";
#endif

/* These paths are NATIVE install paths, change others below */
char * default_include = "/usr/include";
char * optim_rules     = "/lib";
#ifdef LIBDIR
char * default_libdir  = QUOT(LIBDIR);
#else
char * default_libdir  = "/lib";
#endif
char * libdir_suffix   = "";

char devnull[] = "/dev/null";
char * exec_prefixs[16] = {
   0		/* Last chance is contents of $PATH */
};

char * libc = "-lc";

int
main(argc, argv)
int argc;
char ** argv;
{
   struct file_list * next_file;
   char * temp;

   progname = argv[0];
   if ((temp = getenv("BCC_PREFIX")) != 0 )
      localprefix = copystr(temp);

   getargs(argc, argv);
   validate_link_opts();

   reset_prefix_path();

   if (!*localprefix || !localprefix[1]) {

      if (*localprefix == '/') {
	 /* Paths for full NATIVE install "-M/" */
	 build_prefix(default_libdir, libdir_suffix, "");
	 build_prefix(default_libdir, "", "");

	 default_include = build_libpath("-I", "/usr/include", "");
	 default_libdir  = build_libpath("-L", default_libdir, libdir_suffix);
	 optim_rules     = build_libpath("-d", optim_rules, libdir_suffix);
#if 0
      } else if (*localprefix == '+') {
	 /* Paths for a special */
#endif
      } else {
	 /* Relative paths to a build dir "-M-" */
	 build_prefix("/lib", libdir_suffix, "");
	 build_prefix("/lib", "", "");

	 default_include = build_libpath("-I", "/include", "");
	 default_libdir  = build_libpath("-L", "/lib", libdir_suffix);
	 optim_rules     = build_libpath("-d", "/lib", libdir_suffix);
      }

   } else {
      /* Relative paths to normal PREFIX directory */
      default_include = build_libpath("-I", "/lib/bcc/include", "");
      default_libdir  = build_libpath("-L", "/lib/bcc", libdir_suffix);
      optim_rules     = build_libpath("-d", "/lib/bcc", libdir_suffix);

      build_prefix("/lib/bcc", libdir_suffix, "");
      build_prefix("/lib/bcc", "", "");
   }

   build_prefix("/bin", "", "");
#ifdef BINDIR
   add_prefix(QUOT(BINDIR) "/");
#endif

   if (opt_v>1) { command.cmd = ""; command_reset(); }
   
   for(next_file = files; next_file && !error_count; next_file = next_file->next) 
   {
      if (next_file->filetype == 'o') continue;

      if (opt_V)
         fprintf(stderr, "%s:\n", next_file->file);

      /* Assembler that's not to be optimised. */
      if (do_preproc && next_file->filetype == 'x') run_aspreproc(next_file);
      if (do_preproc && next_file->filetype == 'S') run_aspreproc(next_file);
      if (do_as      && next_file->filetype == 's') run_as(next_file);

      /* C source */
      if (do_preproc && next_file->filetype == 'c') run_preproc(next_file);
      if (do_unproto && do_compile && next_file->filetype == 'i')
	                                            run_unproto(next_file);
      if (do_compile && next_file->filetype == 'i') run_compile(next_file);
      if (do_optim   && next_file->filetype == 's') run_optim(next_file);
      if (do_as      && next_file->filetype == 's') run_as(next_file);
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
   static char cc1bcc[] = CC1BCC;

   if (opt_arch<5) {
      if (opt_e)
	 command.cmd = cc1bcc;
      else {
	 command.cmd = CPPBCC;
	 command.altcmd = cc1bcc;
      }
   } else
      command.cmd = CPP;
   command_reset();
   newfilename(file, (!do_as && !do_optim), (do_compile?'s':'i'), (opt_arch<5));
   if (opt_arch<5 && command.cmd == cc1bcc)
      command_opt("-E");
   else if (opt_arch<5 && do_unproto)
      command_opt("-A");
   command_opts('p');
   command_opt("-D__ASSEMBLER__");

   command_arch();
   run_command(file);
}

void
run_preproc(file)
struct file_list * file;
{
   int last_stage = 0;
   int combined_cpp;
   static char cc1bcc[] = CC1BCC;

   if (opt_arch<5) {
      if (opt_e)
	 command.cmd = cc1bcc;
      else {
	 command.cmd = CPPBCC;
	 command.altcmd = cc1bcc;
      }
   } else
      command.cmd = CPP;
   command_reset();

   combined_cpp = (command.cmd == cc1bcc && 
	           opt_arch != 3 &&
		   opt_e < 2 &&
	           !do_unproto && 
		   do_compile);

   if (combined_cpp && !do_optim && !do_as )    last_stage =1;
   if (!combined_cpp && !do_compile ) 		last_stage =1;

   newfilename(file, last_stage, (combined_cpp?'s':'i'), (opt_arch<5));

   if (!combined_cpp && opt_arch<5) {
      if (command.cmd == cc1bcc)
	 command_opt("-E");
      else if (do_unproto)
	 command_opt("-A");
   }

   command_opts('p');
   command_opts('C');
   if (combined_cpp)
   {
      if (opt_arch<5 && !do_as)
	 command_opt("-t");
      command_opts('c');
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

   if (opt_arch<5 && !do_as)
      command_opt("-t");

   command_opts('c');
   command_opts('C');

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
   if (opt_O && opt_arch == 0)
   {
      sprintf(buf, "-huse16 %c86", opt_O);
      command_opt(buf);
   }
   command_opt(optim_rules);

   command_opt("rules.start");
   command_opts('o');

   if (opt_O) {
      if (opt_arch == 0)
	 sprintf(buf, "rules.%c86", opt_O);
      else
	 sprintf(buf, "rules.lv_%c", opt_O);
      command_opt(buf);
   }

   switch(opt_arch) {
   case 0: command_opt("rules.86"); break;
   case 1: 
   case 2: command_opt("rules.i386"); break;
   case 4: command_opt("rules.6809"); break;
   default:command_opt("rules.mid"); break;
   }

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
      command_opt("-r");
      command_opt("-N");
      newfilename(file, !do_link, 'o', 1);
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
   if (executable_name) {
      command_opt("-o");
      command_opt(executable_name);
   }

   if (opt_arch < 2) 
      command_opt("-y");

   command_opts('l');
   if (opt_arch != 2) 
   {
      if (opt_arch == 0 && !opt_i)
	 command_opt("-i");

      if (!opt_L)
	 command_opt(default_libdir);
      command_arch();

      if (!opt_x)
	 command_opt("-C0");
   }
   /* Current Debian compilers only work in with this: */
   else command_opt("--static");

   for(next_file = files; next_file; next_file = next_file->next) 
      command_opt(next_file->file);

   if (opt_arch != 2) 
      command_opt(libc);
   run_command(0);
}

void
validate_link_opt(char * option)
{
   int err = 0;
   if (option[0] != '-') return;

   switch(option[1]) {
   default:
	 err = 1;
      break;
   case '0':           /* use 16-bit libraries */
   case '3':           /* use 32-bit libraries */
   case 'M':           /* print symbols linked */
   case 'i':           /* separate I & D output */
   case 'm':           /* print modules linked */
   case 's':           /* strip symbols */
   case 't':           /* trace modules linked */
   case 'z':           /* unmapped zero page */
   case 'N':           /* Native format a.out */
   case 'd':           /* Make a headerless outfile */
   case 'c':           /* Write header in CP/M-86 format */
   case 'y':           /* Use a newer symbol table */
      if (option[2] != 0 && option[2] != '-')
	 err = 1;
      break;
   case 'C':           /* startfile name */
   case 'L':           /* library path */
   case 'O':           /* library file name */
   case 'T':           /* text base address */
   case 'D':           /* data base address */
   case 'H':           /* heap top address */
   case 'l':           /* library name */
   case 'o':           /* output file name */
      break;
   }
   if (err) {
      if (do_link)
	 fprintf(stderr, "warning: unknown option %s passed to linker.\n",
	                 option);
      else
	 fprintf(stderr, "warning: option %s not recognised.\n", option);
   } else if (!do_link)
      fprintf(stderr, "warning: linker option %s unused.\n", option);
}

void
validate_link_opts()
{
   struct opt_list * ol;
   struct file_list * next_file;
   if (opt_arch>1) return; /* Only check ld86 options */

   for(ol=options; ol; ol=ol->next)
      if (ol->opttype == 'l')
	 validate_link_opt(ol->opt);

   for(next_file = files; next_file; next_file = next_file->next) 
      validate_link_opt(next_file->file);

   if (!do_link) {
      if (opt_i)
	 fprintf(stderr, "warning: linker option -i unused.\n");
      if (opt_x)
	 fprintf(stderr, "warning: linker option -x unused.\n");
      if (opt_L)
	 fprintf(stderr, "warning: linker option -L unused.\n");
   }
}

void
command_reset()
{
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif
   char buf[MAXPATHLEN];
   char ** prefix;
   char * saved_cmd;

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
   saved_cmd = command.cmd;
   for(;;)
   {
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

	    strcat(buf, prefix_path);
	    strcat(buf, p+1);
	 }
	 strcat(buf, command.cmd);

	 if (!*command.cmd)
	    fprintf(stderr, "PATH%d=%s\n", prefix-exec_prefixs, buf);
	 else if (access(buf, X_OK) == 0)
	 {
	    command.fullpath = copystr(buf);
	    break;
	 }
      }
      if (command.fullpath || !command.altcmd) break;
      command.cmd = command.altcmd;
      command.altcmd = 0;
   }

   if (!command.fullpath) {
      command.cmd = saved_cmd;
      command.fullpath = copystr(command.cmd);
   }
   command.altcmd = 0;
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
#ifdef __CYGWIN__
      if ( executable_name == 0 ) {
	 char * p = strrchr(argv[ar], '.');
	 if (p && p == argv[ar] + strlen(argv[ar]) - 2) 
	 {
	    /* This will actually create a COM file, but windows doesn't
	     * care and cygwin will only do PATH searches for EXE files.
	     */
	    *p=0;
	    executable_name = catstr(argv[ar], ".exe");
	    *p = '.';
	 }
      }
#endif
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
      case 'a': case 'L': case 'I': case 'M': case 'O': case 'P': case 'Q': 
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
#if 0
	    /* NOTE I'm setting this to zero, this isn't a _real_ Ansi cpp. */
	    prepend_option("-D__STDC__=0", 'p');
#else
	    prepend_option("-D__STDC__", 'p');
#endif
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
      case 'u':
	 append_option(opt_arg, 'u');
	 break;

      case 'L':
         append_option(argv[ar], 'l');
	 break;

      case 'Q':
         append_option(argv[ar], 'c');
	 break;

      case 'O':
         do_optim=1;
	 if (!opt_arg[1] && ( opt_arg[0] >= '1' && opt_arg[0] <= '9' ))
	    opt_O = opt_arg[0];
	 else if (opt_arg[0] == '-')
	    append_option(opt_arg, 'o');
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
	 if (opt_arg[0] == '/') {
	    localprefix = copystr(opt_arg);
	    break;
	 }
	 if (opt_arg[1]) Usage();
	 if (opt_arg[0] == '-') {
	    localprefix = "";
	    break;
	 }
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

      case 'G': opt_M = 'g'; break;

      case 'v': opt_v++; break;
      case 'V': opt_V++; break;
      case 'e': opt_e++; break;
      case 'x': opt_x++; break;
      case 'I': opt_I++; break;
      case 'L': opt_L++; break;
      case 'i': opt_i++; break;
      case 'f': opt_f++; break;

      case 'W': opt_W++; break;

      case '0': opt_arch=0; break;
      case '3': opt_arch=1; break;
         
      case 'w': /*IGNORED*/ break;
      case 'g': /*IGNORED*/ break;
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
#ifdef __CYGWIN__
   if (opt_M==0) opt_M = 'd';
#endif
   if (opt_M==0) opt_M = (opt_arch==1 ?'l':'n');
   switch(opt_M)
   {
   case 'n': 		/* Normal Elks */
      prepend_option("-D__unix__", 'p');
      prepend_option("-D__ELKS__", 'p');
      libc="-lc";
      break;
   case 'f': 		/* Fast Call Elks */
      prepend_option("-D__unix__", 'p');
      prepend_option("-D__ELKS__", 'p');
      append_option("-c", 'C');
      append_option("-f", 'C');
      libc="-lc_f";
      break;
   case 'c': 		/* Caller saves Elks */
      prepend_option("-D__unix__", 'p');
      prepend_option("-D__ELKS__", 'p');
      append_option("-c", 'C');
      libc="-lc";
      break;
   case 's': 		/* Standalone 8086 */
      prepend_option("-D__STANDALONE__", 'p');
      libc="-lc_s";
      break;
   case 'd': 		/* DOS COM file */
      prepend_option("-D__MSDOS__", 'p');
      if (do_link) {
	 libc="-ldos";
	 append_option("-d", 'l');
	 append_option("-T100", 'l');
      }
      break;
   case 'l': 		/* 386 Linux a.out */
      opt_arch=1;
      prepend_option("-D__unix__", 'p');
      prepend_option("-D__linux__", 'p');
      if (do_link) {
	 libc="-lc";
	 append_option("-N", 'l');
      }
      break;
   case 'g':		/* 386 Linux object using gcc as linker */
      opt_arch = 2;
      prepend_option("-D__unix__", 'p');
      prepend_option("-D__linux__", 'p');

      /* This is a more traditional libc, it also gives a 20k executable
       * for hello world vs. 400k with glibc2 and --static. 
       * NB: DLL libc no longer seems to work.
       */
      add_prefix("/usr/bin/i386-uclibc-");
      break;
   case '8':		/* Use 'c386' program as compiler */
      opt_arch = 3;
      prepend_option("-D__unix__", 'p');
      prepend_option("-D__c386__", 'p');
      break;
   case '9':		/* 6809 compiler */
      opt_arch = 4;
      prepend_option("-D__6809__", 'p');
      break;
   case '0':		/* Plain old Unix V7 style */
      opt_arch = 5;
      opt_I = 1;
      opt_L = 1;
      opt_x = 1;
      append_option("/lib/crt0.o", 'l');
      break;
   default:
      fatal("Unknown model specifier for -M valid are: n,f,c,s,d,l,g,8,9,0");
   }

   if (do_optim)
   {
      append_option("-O", 'C');
      append_option("-O", 'a');
   }

   if (opt_arch == 0) {
      if (opt_f) {
	 /* append_option("--enable-floats", 'c'); */
	 libc = catstr(libc, "+f");
      } else
	 append_option("-D__HAS_NO_FLOATS__", 'p');
   }

   if (opt_arch == 1) libdir_suffix = "/i386";
   if (opt_arch == 4) libdir_suffix = "/m09";

#ifdef VERSION
   {
      char verbuf[64];
      sprintf(verbuf, "-D__BCC_VERSION__=0x%02x%02x%02xL",
	    VER_MAJ, VER_MIN, VER_PAT);
      append_option(verbuf, 'p');
   }
#endif
}

void
build_prefix(path1, path2, path3)
char * path1, * path2, * path3;
{
   char * newstr;
   int l;
   newstr = xalloc(strlen(path1)+strlen(path2)+strlen(path3)
	  + strlen(prefix_path)+2);

   strcpy(newstr, prefix_path);
   strcat(newstr, path1);
   strcat(newstr, path2);
   strcat(newstr, path3);
   l = strlen(newstr);
   if (l>1 && newstr[l-1] != '/')
      strcat(newstr, "/");

   add_prefix(newstr);
}

void
add_prefix(path)
char * path;
{
   char ** p; 
   if (!path || !*path) return;

   for(  p=exec_prefixs; 
	 p<exec_prefixs+(sizeof(exec_prefixs)/sizeof(*p))-1; 
	 p++) {

      if( !*p )
      {
	 *p = path;
	 return;
      }
      if (strcmp(*p, path) == 0) return;
   }
   fatal("Too many -B options");
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

void 
prepend_option (option, otype)
char * option;
int otype;
{
   struct opt_list * newopt = xalloc(sizeof(struct opt_list));

   newopt->opt = copystr(option);
   newopt->opttype = otype;

   newopt->next = options;
   options = newopt;
}

char * build_libpath(opt, str, suffix)
char * opt, * str, * suffix;
{
   char * newstr;
   newstr = xalloc(strlen(opt)+strlen(str)+strlen(prefix_path)+strlen(suffix)+1);
   strcpy(newstr, opt);
   strcat(newstr, prefix_path);
   strcat(newstr, str);
   strcat(newstr, suffix);
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
#ifdef VERSION
#ifdef __AS386_16__
   if (opt_v)
      fprintf(stderr, "%s: version %s (16bit)\n", progname, VERSION);
#else
   if (opt_v)
      fprintf(stderr, "%s: version %s\n", progname, VERSION);
#endif
#endif
   fprintf(stderr,
	 "Usage: %s [-ansi] [-options] [-o output] file [files].\n", progname);
   exit(1);
}

void fatal(str)
char * str;
{
   fprintf(stderr, "%s: Fatal error: %s.\n", progname, str);
   exit(1);
}

#ifdef MSDOS
void reset_prefix_path()
{
   char *ptr, *temp;

   if (*localprefix && localprefix[1]) {
      prefix_path = localprefix;
      return;
   }

   temp = copystr(progname);
   if( (ptr = strrchr(temp, '\\')) != 0
         && temp<ptr-4 && strncmp(ptr-4, "\\BIN", 4) == 0 )
   {
      ptr[-4] = 0;
      prefix_path = temp;
   }
   else
      free(temp);
}
#else

void reset_prefix_path()
{
   char *ptr, *temp;

   if (*localprefix && localprefix[1]) {
      prefix_path = localprefix;
      return;
   }

   if ( *localprefix == '/' && !localprefix[1]) {
      prefix_path = "";
      return;
   }

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
#ifndef __BCC__
         if( realpath(temp, buf) != 0 )
	 {
	    free(temp);
	    temp = copystr(buf);
         }
#endif
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
      prefix_path = temp;
   }
   else
      free(temp);
}
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
      if (command.fullpath[0] =='/')
	 execv(command.fullpath, command.arglist);
      else
	 execvp(command.fullpath, command.arglist);
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
   if (status)
   {
      if (file) file->filetype = '~';
      error_count++;
   }
}

