
#include "monitor.h"

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

#ifndef NOCOMMAND
extern unsigned int current_address;
extern int number_base;

int cmd_quit(args)
char * args;
{
   printf("Bye\n");
   exit(0);
}

int cmd_nop(ptr)
char * ptr;
{
}

int cmd_dir(ptr)
char * ptr;
{
   open_file(".");
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
#endif

/**************************************************************************/

#ifndef NOMONITOR

cmd_monhelp() {
static char * helps[] = {
   "Monitor help\n",
   "zimage filename [args]     Start Linux-386\n",
   "bzimage filename [args]    Start Linux-386\n",
   "=filename [args]           Start Linux-386\n",
   "quit                       Reboot\n",
   "help                       Display help file\n",
   "dir                        Display directory\n",
   "type                       Display file to screen\n",
   "more                       Display file to screen\n",
   "m[em] [address]            Hexdump memory\n",
   "seg [val]                  Set/show default segment\n",
   "rel [segment]              Relocate this program\n",
   "base [val]                 Set default number base (or 'n' cmd)\n",
   "init                       Re-init program\n",
   "r[egs]                     Display register values\n",
   
   0
};
   char **p = helps;
   more_char(-1);

   while(*p)
      if( more_strn(*p, strlen(*p)) <0 )
         break;
      else
         p++;
   printf("\n");
   return 0;
}

cmd_regs()
{
#ifdef __STANDALONE__
   printf(": AX=%04x BX=%04x CX=%04x DX=%04x SI=%04x DI=%04x",
	  __argr.x.ax, __argr.x.bx, __argr.x.cx, __argr.x.dx,
	  __argr.x.si, __argr.x.di);
   printf(" CF=%x", __argr.x.cflag);
   printf(" CS=%04x DS=%04x ES=%04x\n", __get_cs(), __get_ds(), __get_es());
#else
   printf("Only in standalone\n");
#endif
}

int cmd_memdump(ptr)
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
#endif
