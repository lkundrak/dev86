
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <dos.h>
#include "i86_funcs.h"
#include "readfs.h"
#include "version.h"

#ifdef __STANDALONE__
#define NOT_VT52COLOUR
#define NOT_ANSICOLOUR
#endif

char command_buf[256];

typedef int (*proc)();

int cmd_quit(), cmd_dump(), cmd_seg(), cmd_rel();
void init_prog();

extern struct t_cmd_list {
   char * command;
   proc func;
} cmd_list[];

static unsigned int current_address;
static int number_base = 16;
#ifdef __STANDALONE__
   extern union REGS __argr;
#endif

#ifdef __STANDALONE__
main()
#else
main(argc, argv)
int   argc;
char ** argv;
#endif
{
static char minibuf[2] = " ";
   int   ch, i;
   char *cmd, *args, *ptr;
   struct t_cmd_list * cptr;

#ifdef __STANDALONE__
   printf("\r");
#else
   if( argc > 1 && strcmp(argv[1], "-t") == 0 ) x86_test=0; else x86_test=1;
#endif

   init_prog();
   if( __get_ds() != 0x1000 )
   {
      relocator(-1);
      relocator(1);
      if( __get_ds() > 0x1000 ) relocator(2);
      printf("Relocated to CS=$%04x DS=$%04x\n", __get_cs(), __get_ds());
   }

#ifdef __STANDALONE__
   if( (__argr.x.dx & 0xFF) == 0 )
#endif
   {
      display_help(0);

      if( x86 > 2 && !x86_emu )	/* Check some basics */
         cmd_bzimage((void*)0);
#if 0
      else
         printf("System is not an 80386 compatible in real mode, load aborted.\nUse 'bzimage' command to attempt load.\n");
#endif
   }

   for (;;)
   {
#ifdef ANSICOLOUR
      printf("\033[36m");
#endif
#ifdef VT52COLOUR
      printf("\033S \033R+\033Sa\033Rc");
#endif
      printf(">");
      fflush(stdout);
      ch = read(0, command_buf, sizeof(command_buf)-1) ;
      if( ch <= 0 ) break;
      command_buf[ch] = '\0';
      if( ch == 1 && command_buf[0] != '\n' )
      {
         sprintf(command_buf, "?$%02x\n", command_buf[0]&0xFF);
	 printf("%s", command_buf);
      }
      if( command_buf[ch-1] == '\n' ) command_buf[ch-1] = 0;

      for(ptr=command_buf; *ptr; ptr++)
      {
         if(*ptr=='"') do { ptr++; } while(*ptr && *ptr != '"');
	 if( isupper(*ptr) ) *ptr = tolower(*ptr);
      }

      cmd = command_buf;
      while(*cmd == ' ') cmd++;
      if( !isalpha(cmd[0]) || !isalpha(cmd[1]) )
      {
         minibuf[0] = cmd[0];
	 args = cmd+1;
	 cmd = minibuf;
      }
      else
      {
         args = strchr(cmd, ' ');
	 if( args ) *args++ = 0;
	 else       args = cmd + strlen(cmd);
      }

      if( *cmd == 0 ) continue;

      for(cptr = cmd_list; cptr->command; cptr++)
      {
         if( strcmp(cptr->command, cmd) == 0 )
	    break;
      }
#ifdef ANSICOLOUR
      printf("\033[37m");
#endif
#ifdef VT52COLOUR
      printf("\033S \033Sa\033Rg");
#endif
      fflush(stdout);
      if( cptr->command )
	 (void) (*cptr->func)(args);
      else
         printf("Command not found.\n");
   }
}

/****************************************************************************/

void init_prog()
{
#ifdef ANSICOLOUR
   printf("\033[H\033[0;44;37m\033[2J");
#endif
#ifdef VT52COLOUR
   printf("\033E\033Rg\033Sa\033J");
#endif
#ifdef VERSION
   printf("Linux x86 boot monitor, Version %s.\n", VERSION);
#else
   printf("Linux x86 boot monitor.\n");
#endif

   cpu_check();
   mem_check();

   printf("Processor: %s", x86_name);
   if(x86_fpu) printf(" with FPU");
   if(x86_emu) printf(" in protected mode");
   if(!x86_test && x86 > 1)
   {
      printf(", A20 gate ");
      if( a20_closed() )
      {
         open_a20();
         if( a20_closed() )
	    printf("won't open!!");
         else
	    printf("is now open");
      }
      else printf("is already open.");
   }
   printf("\n");

   printf("There is %dk of boot memory", boot_mem_top/64);
   if( main_mem_top )
   {
      printf(" %ld.%ldM %sof main memory",
	      main_mem_top/1024,
	      (10*main_mem_top)/1024%10,
              main_mem_top >= 0xFC00L ?"(perhaps more) ":""
	      );
   }
   printf("\n");
}

#ifdef __STANDALONE__
reg_line()
{
   printf("REGS: AX=%04x BX=%04x CX=%04x DX=%04x SI=%04x DI=%04x\n",
	  __argr.x.ax, __argr.x.bx, __argr.x.cx, __argr.x.dx,
	  __argr.x.si, __argr.x.di);
}
#endif

#ifdef VT52COLOUR
colour_line()
{
   printf("Colours: \033S  ");
   printf("\033R_UNL\033S  ");
   printf("\033R*BLK\033S  ");
   printf("\033R+BLD\033S  ");
   printf("\033R!REV\033S  ");
   printf("\033Sa\033Rg ");
   printf("\033R@@\033Raa\033Rbb\033Rcc\033Rdd\033Ree\033Rff\033Rgg");
   printf("\033Rhh\033Rii\033Rjj\033Rkk\033Rll\033Rmm\033Rnn\033Roo");
   printf("\033Sa\033Rg\n");

   printf("\033S \033Sa\033Rg");
}
#endif

/****************************************************************************/

int
getnum(numptr, valptr)
char ** numptr;
unsigned int * valptr;
{
   char * ptr = *numptr;
   unsigned int val = 0;
   int base = number_base;
   int flg = 0;

   while( *ptr && *ptr <= ' ' ) ptr++;
   switch(*ptr)
   {
   case '$': case '&': case 'X':
      base=16; ptr++; break;
   case '#':
      base=10; ptr++; break;
   case '%':
      base=2; ptr++; break;
   case '0':
      if( ptr[1] == 'X' ) { base=16; ptr+=2; }
      break;
   case '-':
      *numptr = ptr+1;
      return 0;
   }

   while(*ptr)
   {
      int d = -1, ch;
      ch = *ptr;
      if( ch >= '0' && ch <= '9' ) d = ch - '0';
      if( ch >= 'a' && ch <= 'z' ) d = ch - 'a' + 10;
      if( ch >= 'A' && ch <= 'Z' ) d = ch - 'A' + 10;

      if( d>=0  && d<base )
      {
         val = val * base + d;
	 ptr++;
	 flg=1;
      }
      else
         break;
   }
   if( flg )
   {
      *numptr = ptr;
      if(valptr) *valptr = val;
      else return val;
   }
   return flg;
}

more_char(ch)
int ch;
{
static int line_number = 0;

   if( ch == -1 ) { line_number = 0; return 0; }

   if( (ch & 0xE0 ) || ch == '\n' )
      putchar(ch);
   if( ch == '\n' && ++line_number == 24)
   {
      char buf[4];
      printf("More ?"); fflush(stdout);
      if( read(0, buf, 1) <= 0 )            return -1;
      if( buf[0] == 3 || buf[0] == '\033'
       || buf[0] == 'q' || buf[0] == 'Q' )  return -1;
      if( buf[0] == '\r' ) line_number--;
      if( buf[0] == ' ' ) line_number=2;
      printf("\r      \r");
   }
   return 0;
}

more_strn(str, len)
char * str;
int len;
{
   for(; len>0 && *str ; len--,str++)
      if( more_char( *str & 0xFF ) < 0 ) return -1;
   return 0;
}

/****************************************************************************/

int cmd_quit(args)
char * args;
{
   printf("Bye\n");
   exit(0);
}

cmd_memdump(ptr)
char * ptr;
{
   int count = 128;
   int i,j;
   int es = __get_es();

#define rmem(x) __peek_es( (x)+current_address )

   getnum(&ptr, &current_address);
   getnum(&ptr, &count);

   for(i=0; i<count; i+=16)
   {
      printf("%04x:%04x:", es, current_address);
      for(j=0; j<16; j++)
         printf(" %s%02x", (j==8)?" ":"", rmem(j));
      printf("  ");
      for(j=0; j<16; j++)
	 if( rmem(j) >= ' ' && rmem(j) <= '~' )
            putchar(rmem(j));
	 else
	    putchar('.');
      putchar('\n');
      current_address += 16;
      current_address &= 0xFFFF;
   }

#undef rmem
}

int cmd_nop(ptr)
char * ptr;
{
}

int cmd_seg(ptr)
char * ptr;
{
   int es = __get_es();

   if( getnum(&ptr, &es) )
   {
      __set_es(es);
      current_address = 0;
   }
   else
      printf("Current segment 0x%04x\n", es);
   return 0;
}

int cmd_set_base(ptr)
{
   int obase = number_base;
   int nbase;
   number_base = 10;

   if( getnum(&ptr, &nbase) )
   {
      if( nbase < 2 || nbase > 36 )
         printf("Can't use that base\n");
      else
         obase = nbase;
   }
   else printf("Current base is %d\n", obase);

   number_base = obase;
   return 0;
}

int cmd_rel(ptr)
char * ptr;
{
   int nseg = 0xFFFF;
   int cs = __get_cs();

   getnum(&ptr, &nseg);

   relocator(nseg);
   if( __get_cs() == cs )
      printf("Didn't relocate; CS=$%04x DS=$%04x\n", __get_cs(), __get_ds());
   else
      printf("Relocated to CS=$%04x DS=$%04x\n", __get_cs(), __get_ds());
}

int cmd_dir(ptr)
char * ptr;
{
   open_file(".");
   return 0;
}

int cmd_type(ptr)
char * ptr;
{
   char * fname;
   char buffer[1024];
   long len;

   while(*ptr == ' ') ptr++;
   if( (fname=ptr) == 0 ) return 0;
   while(*ptr & *ptr != ' ') ptr++;

   if( open_file(fname) >= 0 ) for(len=file_length(); len>0; len-=1024)
   {
      if( read_block(buffer) < 0 ) break;
      if( len > 1024 )
         write(1, buffer, 1024);
      else
         write(1, buffer, len);
   }
   else
      printf("Cannot open file '%s'\n", fname);
   close_file();
   return 0;
}

int cmd_more(ptr)
char * ptr;
{
   char * fname;
   char buffer[1024];
   long len;
   int cc;
   char * sptr;

   while(*ptr == ' ') ptr++;
   if( (fname=ptr) == 0 ) return 0;
   while(*ptr & *ptr != ' ') ptr++;

   more_char(-1);

   if( open_file(fname) >= 0 ) for(len=file_length(); len>0; len-=1024)
   {
      if( read_block(buffer) < 0 ) break;
      if( len > 1024 ) cc = 1024; else cc = len;
      for(sptr=buffer; cc>0 ; cc--,sptr++)
      {
	 if( more_char(*sptr & 0xFF) < 0 ) goto break_break;
      }
   }
   else
      printf("Cannot open file '%s'\n", fname);
break_break:;
   close_file();
   return 0;
}

/****************************************************************************/

/* Others */
extern int cmd_bzimage();
extern int cmd_help();

/****************************************************************************/

struct t_cmd_list cmd_list[] = 
{
   {"exit",   cmd_quit}, {"quit",  cmd_quit}, {"q",  cmd_quit},
   {"#",      cmd_nop},
   {"help",   cmd_help},    /* Display from help.txt */
   {"?",      cmd_help},    /* Display from help.txt */
   {"zimage", cmd_bzimage}, /* Load and run 386 zimage file */
   {"bzimage",cmd_bzimage}, /* Load and run 386 bzimage file */
   {"=",      cmd_bzimage}, /* Load and run 386 bzimage file */
   {"dir",    cmd_dir},     /* Display directory */
   {"cat",    cmd_type},    /* Cat/Type a file to the screen */
   {"more",   cmd_more},    /* More a file to the screen */

   /* Debugger/monitor commands */
   {"memdump",cmd_memdump}, {"mem",cmd_memdump}, {"m",  cmd_memdump},
                            /* Display bytes */
   {"seg",    cmd_seg},     /* Set default segment */
   {"rel",    cmd_rel},	    /* Relocate self */
   {"base",   cmd_set_base},
   {"n",      cmd_set_base},

#ifdef VT52COLOUR
   {"colour", colour_line},
#endif
   {"init",   init_prog},
#ifdef __STANDALONE__
   {"reg",    reg_line},
   {"r",      reg_line},
#endif

/*
   {"edit",   cmd_edit},    Alter memory
   {"move",   cmd_move},    Move memory contents

   {"load",   cmd_load},    Load file of inode
   {"stat",   cmd_stat},    Stat info of inode

   {"image",  cmd_image},   Load and run 8086 image file

   {"read",   cmd_read},    Read sector
   {"write",  cmd_write},   Write sector

   {"call",   cmd_call},    load and run a bcc linux-8086 program.
*/
   {0,0}
};
