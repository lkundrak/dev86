
#include "monitor.h"
#include "version.h"

char command_buf[256];

typedef int (*proc)();

int cmd_quit(), cmd_dump(), cmd_seg(), cmd_rel(), cmd_bzimage(), cmd_help();
int cmd_nop(), cmd_memdump(), cmd_set_base(), cmd_dir(), cmd_type(), cmd_more();
int cmd_regs(), cmd_monhelp();

void init_prog();

extern struct t_cmd_list {
   char * command;
   proc func;
} cmd_list[];

unsigned int current_address;
int number_base = 16;

main()
{
static char minibuf[2] = " ";
   int   ch, i;
   char *cmd, *args, *ptr;
   struct t_cmd_list * cptr;

#ifdef __STANDALONE__
   printf("\r\n");
#endif

   init_prog();

   if (!x86_test)
   {
#ifdef __STANDALONE__
#ifndef NOCOMMAND
#if 0
      /* First out of the way. */
      relocator(3);
      /* Then align DS to 64k boundry -> DMA is simple. */
      relocator(0x1000-__get_ds()+__get_cs());
#else
      relocator(-1);	/* Top of available memory */
#endif
      printf("Relocated to CS=$%04x DS=$%04x\n", __get_cs(), __get_ds());
#endif

      disk_drive = __argr.h.dl;
#endif
#ifdef NOCOMMAND
      cmd_type("help.txt");
#else
      display_help(0);
#endif
      cmd_bzimage((void*)0);
   }

#ifdef NOCOMMAND
   printf("Unable to boot, sorry\nreboot:");
   fflush(stdout);
   read(0, command_buf, sizeof(command_buf)-1) ;
#else
   for (;;)
   {
#ifdef COLOUR
      printf("\033S \033R+\033Sa\033Rc");
#endif
      printf(">");
      fflush(stdout);
      ch = read(0, command_buf, sizeof(command_buf)-1) ;
      if( ch <= 0 ) break;
      command_buf[ch] = '\0';
      if( ch == 1 && command_buf[0] != '\n' )
      {
	 putchar('\n');
	 help_key(command_buf[0]&0xFF);
	 continue;
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
#ifdef COLOUR
      printf("\033S \033Sa\033Rg");
#endif
      fflush(stdout);
      if( cptr->command )
	 (void) (*cptr->func)(args);
      else
         printf("Command not found.\n");
   }
#endif
}

/****************************************************************************/

void init_prog()
{
   char offt;
#ifdef COLOUR
   vt52_putch(0);
   printf("\033E\033Rg\033Sa\033J");
#endif
   printf("Linux x86");
#ifdef NOCOMMAND
#ifdef TARFLOPPY
   printf(" TAR floppy booter");
#else
#ifdef DOSFLOPPY
   printf(" DOS floppy booter");
#else
   printf(" floppy booter");
#endif
#endif
#else
   printf(" boot monitor");
#endif
   printf(", Version %s", VERSION);
   printf(".\n");

   cpu_check();
   mem_check();

#ifndef NOMONITOR
   printf("Processor: %s", x86_name);
   if(x86_fpu) printf(" with FPU");
   if(x86_emu) printf(" in protected mode");
   printf("\n");
#endif

   printf("There is %u bytes available", &offt-sbrk(0));
   printf(", %dk of boot memory", boot_mem_top/64);
   if( main_mem_top )
   {
      printf(", %d.%dM of main memory",
	      (int)(main_mem_top/1024),
	      (int)((10*main_mem_top)/1024%10));
   }
   printf(".\n");
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

#ifndef NOCOMMAND
struct t_cmd_list cmd_list[] = 
{
   {"zimage", cmd_bzimage}, /* Load and run 386 zimage file */
   {"bzimage",cmd_bzimage}, /* Load and run 386 bzimage file */
   {"=",      cmd_bzimage}, /* Load and run 386 bzimage file */

   {"exit",   cmd_quit}, {"quit",  cmd_quit}, {"q",  cmd_quit},
   {"#",      cmd_nop},
   {"help",   cmd_help},    /* Display from help.txt */
   {"dir",    cmd_dir},     /* Display directory */
   {"cat",    cmd_type},    /* Cat/Type a file to the screen */
   {"type",   cmd_type},    /* Cat/Type a file to the screen */
   {"more",   cmd_more},    /* More a file to the screen */

#ifndef NOMONITOR
   /* Debugger/monitor commands */
   {"?",      cmd_monhelp}, /* Display builtin help */

   {"memdump",cmd_memdump}, {"mem",cmd_memdump}, {"m",  cmd_memdump},
                            /* Display bytes */
   {"seg",    cmd_seg},     /* Set default segment */
   {"rel",    cmd_rel},	    /* Relocate self */
   {"base",   cmd_set_base},
   {"n",      cmd_set_base},

   {"init",   init_prog},
   {"regs",   cmd_regs},
   {"reg",    cmd_regs},
   {"r",      cmd_regs},

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
#endif

   {0,0}
};
#endif
