
#define VERSION "0.0.0-ALPHA"
#define NOT_ANSICOLOUR
#define VT52COLOUR

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <i86_funcs.h>
#include <dos.h>

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

   printf("\n\n");
#ifdef ANSICOLOUR
   printf("\033[H\033[0;44;37m\033[2J");
#endif
#ifdef VT52COLOUR
   printf("\033E\033Rg\033Sa\033J");
#endif
   printf("Linux x86 boot monitor Version %s\n", VERSION);

#ifndef __STANDALONE__
   if( argc > 1 && strcmp(argv[1], "-t") == 0 ) x86_test=0; else x86_test=1;
#endif

   init_prog();
#ifdef __STANDALONE__
   {
      extern union REGS __argr;
      printf("REGS: AX=%04x BX=%04x CX=%04x DX=%04x SI=%04x DI=%04x\n",
             __argr.x.ax, __argr.x.bx, __argr.x.cx, __argr.x.dx,
             __argr.x.si, __argr.x.di);
   }
#endif
#ifdef VT52COLOUR
   printf("Colours: ");
   printf("\033S \033R_UNL ");
   printf("\033S \033R*BLK ");
   printf("\033S \033R+BLD ");
   printf("\033S \033R!REV");
   printf("\033Sa\033Rg  ");
   printf("\033R@@\033Raa\033Rbb\033Rcc\033Rdd\033Ree\033Rff\033Rgg");
   printf("\033Rhh\033Rii\033Rjj\033Rkk\033Rll\033Rmm\033Rnn\033Roo");
   printf("\033Sa\033Rg\n");
#endif

   printf("Type ^C to exit\n");

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
         sprintf(command_buf, "func 0x%02x\n", command_buf[0]&0xFF);
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
      if( cptr->command )
	 (void) (*cptr->func)(args);
      else
         printf("Command not found.\n");
   }
}

/****************************************************************************/

void init_prog()
{
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
      printf(" %ld.%ldM of main memory",
	      main_mem_top/1024,
	      (10*main_mem_top)/1024%10
	      );
   }
   printf("\n");

   if( main_mem_top >= 0xFC00L )
      printf("There may be more main memory available but the BIOS don't say\n");
}

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
         printf(" %02x", rmem(j));
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

   getnum(&ptr, &nseg);

   printf("Monitor code seg from 0x%04x ", __get_cs());
   fflush(stdout);
   relocator(nseg);
   printf("to 0x%04x\n", __get_cs());
}

/****************************************************************************/

struct t_cmd_list cmd_list[] = 
{
   {"exit",   cmd_quit}, {"quit",  cmd_quit}, {"q",  cmd_quit},
   {"memdump",cmd_memdump}, {"m",  cmd_memdump},
                            /* Display bytes */
   {"seg",    cmd_seg},     /* Set default segment */
   {"rel",    cmd_rel},	    /* Relocate self */
   {"base",   cmd_set_base},
   {"n",      cmd_set_base},
   {"#",      cmd_nop},
/*
   {"edit",   cmd_edit},    Alter memory
   {"move",   cmd_move},    Move memory contents

   {"dir",    cmd_dir},     Display dir of inode
   {"load",   cmd_load},    Load file of inode
   {"stat",   cmd_stat},    Stat info of inode

   {"zimage", cmd_zimage},  Load and run 386 zimage from inode or tar file
   {"bimage", cmd_bimage},  Load and run 386 bzimage from inode or tar file
   {"image",  cmd_image},   Load and run 8086 image from inode or tar file

   {"read",   cmd_read},    Read sector
   {"write",  cmd_write},   Write sector

   {"call",   cmd_call},    load and run a bcc linux-8086 program.
*/
   {0,0}
};
