/*
 * Terminal emulator.
 *
 * This program is a terminal emulator for the IBM PC.
 * It's basic emulation is as very close to the SCO-Xenix ANSI console
 * but it's had a few additions that seemed a good idea at the time.
 *
 * Performance: I use this on a 16Mhz 286 at 115k2 baud with RTS flow
 *              control and experience NO lost characters. The PC cannot
 *              quite keep up if flow control is disabled though.
 *
 * This program can run at 19200 on a 4.77 Mhz XT with flow control.
 * (Upto 57600 is possible if the BIOS doesn't disable interrupts)
 */

/*
 * TODO:
 *
 * - Add file operations (open, read/write block, close)
 * - Add dubious hack to attempt to make keyboard & timer RTS-Flow active.
 * - Adjust extra CSI codes to be in small range (CSI $A .. CSI $Z ?)
 * - Add ZMODEM challange string.
 * - Add local echo mode. (Only ascii chars ?)
 * - Add magic echo mode - like local echo but display tempoary.
 * - Merge BIOS serial with HW serial.
 */
/*****************************************************************************

      ANSI Emulation.
      ===============

   Special characters

   ^G      Beep
   ^H      Backspace
   ^I      Tab
   ^J      Linefeed
   ^L      Clear screen
   ^M      Carriage return
   ^V      Avatar code prefix.
   ^X      Cancel Escape code or string                    (XXX But ZMODEM)
   ^[      Escape code prefix

   Escape codes

   ^[ [    CSI code - see below.
   ^[ P    DCS Code, as CSI followed by string and ST
   ^[ \    ST - String terminator.

   ^[ (    Font select
	    U  IBM CP 437
	    u  IBM CP 437 with 0x00-0x7f replicated to 0x80-0xFF
	    B  ISO 8859-1         + DIRECT FONT 0x00-0x1F in 0x80-0x9F.
	    0  VT100 Graphics     + DIRECT FONT 0x80-0xFF
	    A  UK ASCII

   ^[ 7    Save cursor position                            (XXX Attribute?)
   ^[ 8    Restore cursor position                         (XXX Attribute?)

   ^[ M    Reverse LF                                      (Not implemented)

   ^[ Q    SCO Function key redefinition             (Only if !AT_BIOS_KEY)

   ^[ >    Keyboard normal				   (if AT_BIOS_KEY)
   ^[ =    Keyboard in dosemu/keypad mode.		   (if AT_BIOS_KEY)

   Simple ANSI CSI codes

   CSI  A  Cursor Up
   CSI  B  Cursor Down
   CSI  C  Cursor Right
   CSI  D  Cursor Left
   CSI  H  Cursor to position.
   CSI  J  Screen area clear (SCO honours attributes)
   CSI  K  Line area clear (SCO honours attributes)
   CSI  m  Attributes (See detail)
   CSI  s  Save cursor position
   CSI  u  Restore cursor position

   SCO ANSI CSI codes

   CSI  @  Insert chars
   CSI  E  Col 0, Cursor Down
   CSI  F  Col 0, Cursor Up
   CSI  H  Cursor to (r,c) position.
   CSI  L  Insert lines
   CSI  M  Delete lines
   CSI  P  Delete characters
   CSI  S  Scroll up                                               (XXX)
   CSI  T  Scroll Down                                             (XXX)
   CSI  X  Clear characters
   CSI  Z  Back tab
   CSI  a  Cursor Right
   CSI  e  Col 0, Cursor Down
   CSI  f  Cursor to (r,c) position.
   CSI  g  Write Specific font char (AM mode always = 0, font always Raw)
   CSI =B  Beep type
   CSI =C  Cursor type
   CSI =E  Background bold or blink.
   CSI =F  Default foreground colour (NB Colour is in IBM VGA order)
   CSI =G  Default background colour (NB Colour is in IBM VGA order)
   CSI =H  Reverse foreground colour (NB Colour is in IBM VGA order)
   CSI =I  Reverse background colour (NB Colour is in IBM VGA order)
   CSI =J  Graphic foreground colour (NB Colour is in IBM VGA order)
   CSI =K  Graphic background colour (NB Colour is in IBM VGA order)

   Additional ANSI CSI codes

   CSI  }  Page flipping & Macros.
   CSI ?lh Set/reset mode (am if arg == 7)
   CSI  i  Printer control.
   DCS $|  Message line (DCS code)

	   DCS for full setup (Keyboard, font, am, zapmsg, default attrs)
	       - (This page or all pages and default)

	   CSI&DCS for open/read/write/close file.
	   CSI for inquire session hash key.

           Dangerous codes to have 4 digit PIN number (generated for session)


   VT 52 codes - (Not implemented)

      if c in "ABCDHJK" do_ansi of same code.
      if c == Y -> cursor position.
      if c == I -> ^[M
      if c == Z -> xmits("\033\\Z") or something

      if c == .. -> Add attribute and charset extras.


*****************************************************************************

*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <dos.h>
#include <bios.h>
#include <process.h>
#include <signal.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#define LINUX_DEFAULTS

#define RS8250		/* Hardware RS-232 driver */
#define BEEPER		/* Interrupt driven beeper */
#define MEM_SCR		/* Use Hardware screen memory */
#define KBDRTS		/* Flow control on keyboard interrupt */
#define AT_BIOS_KEY	/* AT keyboard + DOSemu keymapping */
#define SCO_KEYMAP	/* Make keyboard look SCOish */

#define XFORKDOS 	/* Key to spawn shell */
#define XPRINTER	/* Almost useless but does (just) work */
#define XZMODEM		/* Beware DSZ has an 16bit for the speed! */
#define XAVATAR		/* Decode dos 'AVATAR' screen codes */

#ifdef MEM_SCR /* All the following are possible only if MEM_SCR is set */

#define SCO_ANSI	/* Decode SCO ansi control codes. */
#define MAXPAGE 10	/* Multiple screens */
#define SAVEBLOCK 3072	/* Screen macros */
#define XSCO_REMAP	/* Allow SCO key remap functions (!AT_BIOS_KEY) */

#define XTSRPROG	/* Currently only works with MSC 5.1 */
#define XAVATAR_PLUS	/* More avatar functions */
#endif

/* Old or non-working stuff */
#define XOLDBEEPER
#define XFLDATTR

#if defined(__STDC__) || defined(__BORLANDC__)
#define _(x) x
#else
#define _(x) ()
#endif

#ifdef OLDBEEPER
int duration = 0x800;
int freq     = 0x1000;
int warble   = 1;

int soundval = -1;
int soundcnt = -1;
int soundfreq= -1;
int sounddir = -1;
#endif
#ifdef BEEPER
int duration = 5;
int freq     = 0x600;
int lastfreq;

int soundval = -1;
int soundcnt = -1;

#define TIMER    0x1C
void ( interrupt far *orig_timer_int)();
void interrupt far timer_int();
#endif

#ifdef KBDRTS
#define KBDINT 0x09
void ( interrupt far *orig_kbd_int) _((void));
void interrupt far kbd_int _((void));
#endif

#ifdef PRINTER
int in_transprt = 0;
int print_dev = 0;
#endif

#define FALSE    0
#define TRUE     !FALSE

#define RS232    0x14
#define INITRS   0
#define WRITECH  1
#define READCH   2
#define STATUS   3
#define FOSINIT  4

#define DTARDY   0x100
#define ZFLAG    0x40

#define CHAROUT  2
#define CHARIN   6

#ifdef AT_BIOS_KEY
#define KEY_SPECIAL (int)0x8000
#define KEY_PAD     0x4000
#define KEY_OTHER   0x2000
#define KEY_DIRECT  0x1000
#define KEY_ALT     0x0800
#define KEY_ALTGR   0x0400
#define KEY_CTRL    0x0200
#define KEY_SHIFT   0x0100

#define DOS_KEY  (KEY_SPECIAL+KEY_ALT+'d')
#define EXITKEY  (KEY_SPECIAL+KEY_SHIFT)
#define SYSLKEY  (KEY_SPECIAL+KEY_SHIFT)

int sysline_warn = 0;
#else
#define EXITKEY  0x2D00
#define DOS_KEY  0x2000
#define SYSLKEY  0x1F00
#endif

void main _((int argc, char ** argv));
int keybd _((void));
int xmit _((int ch));
void putstr _((char * str));
void init_vid _((void));
int test_svga_mode _((void));
void end_vid _((void));
void putscrn _((int c));

void init_beep _((void));
void beep_on _((void));
void sounder _((void));
void beep_set _((int f, int p));
void clear_beep _((void));

#ifdef MEM_SCR
void putansi _((int c));
void litput _((int c));
void rawput _((int c));
void set_pos _((int line, int col));
void update_page _((void));
void updateline _((int to));
void fetch_page _((void));
void fetch_line _((int from));
void do_ansi _((int cmd, int subcmd, int * args, int argc));
void scroll _((int from, int to, int lines));
void clrline _((int to));
int reset_page _((void));
void clr_eol _((void));
void clr_sol _((void));
void delch _((int chars));
void inch _((int chars));
void zapch _((int chars));
void set_blink _((int flg));
void move_cur _((int row, int col));
void set_curtype _((int a, int top, int bot));
#endif
#ifdef SAVEBLOCK
void read_block _((int blk));
void save_block _((int blk));
#endif

void setup _((int port, int flags));
void clearup _((void));
void do_xmit _((void));
void serial _((void));

#ifndef RS8250
int chrdy _((void));
#else
int get_c _((void));
void chfetch _((void));
void setints _((void));
void clrints _((void));
void rtsflow _((int stop));
void init_kbd _((void));
void clear_kbd _((void));
#endif

int get_key _((void));
void write_scan _((int scan));
void write_dosemu _((int ch));
void write_unix _((int ch));

/*xYx*/
void message();

int isfossil = 0;
int fossilfunc = 0;
int mnp_fossil = 0;

int mask   = 0xFF;

union REGS iregs, oregs;

int nsize = 0;
char    empty[4096];
#define full (empty+sizeof(empty))
char *  putptr = empty;
char *  getptr = empty;

char xmit_buf[512];
char *xmit_put=xmit_buf, *xmit_get=xmit_buf;

/***************************************************************************/

#ifdef MEM_SCR

#define intvid(reg) int86(0x10, &reg, &reg)
#ifndef MAXPAGE
#define MAXPAGE 1
#endif

int page_no;
int dos_cur;
int dos_mode;
int dos_line;
int dos_col;
int line_len;
int scr_len;
int is_ega_plus;

struct physical_scr
{
   int far * scrptr;
   int dumb   ;
   int this_page ;
   int line;
   int col;
}
   phy_scr;

struct page_descr
{
   int ** lines;
   int col;
   int line;
   int charflip;
   int chartran;
   int attr;
   int dirtypg;
   int * dirty;

   int * page;
   int scol, sline;
   int std_col;
   int rev_col;
   int gr_col;
   int clr_col;

#ifdef AT_BIOS_KEY
   int key_mode;	/* 0 = Unix, 1 = dosemu */
#endif
#ifdef SCO_ANSI
   int am;		/* 1 = can't use col 80, 2 = no autowrap */
#endif
   int honour_attr;	/* Area clears honour attribute */
   int reset_done;
}
   pages[MAXPAGE];

#if MAXPAGE == 1
#define curpage pages
#else
struct page_descr * curpage = pages;
#endif

int args[10];
int argc;
int tchar;
int echar;

char dcs[256], *dcs_ptr;

int sysline[80];
int syslineno = -1;
int syslinetmp = 0;
int syslinedirty = 0;
int syslineold = -1;

#endif

/***************************************************************************/

#ifdef SCO_REMAP
extern int kbtab[];
extern int keyxlate[];
#define KEYTABSIZE 64
#endif

#ifdef SAVEBLOCK

#define max_blk 100

int save_flag = 0;
int read_flag = 0;
char save_buf[SAVEBLOCK];
char * save_ptr;
char * blocks[max_blk];

#endif

/***************************************************************************/

#ifdef TSRPROG
#define TSR		0x31
#define KEYBD		0x16
#define OLDKB		0x65

void ( interrupt far *orig_key_int)();
void interrupt far key_int();
int tsrflag = 0;
char far * dos_screen;

#ifndef __BORLANDC__
extern
struct irq_stack
{
   int es;
   int ds;
   int di;
   int si;
   int bp;
   int sp;
   int bx;
   int dx;
   int cx;
   int ax;
   int pc;
   int cs;
   int flgs;
} far * sp_save;
#endif

#endif

/***************************************************************************/

long speeds[] = {  110,  150,  300,  600,
                  1200, 2400, 4800, 9600,
#ifdef RS8250
                 19200, 38400, 115200, 57600,
                 28800, 23040, 14400, 75,
                 50, 134, 200, 1800,
#endif
                 0 };

/***************************************************************************
 * Main, here we go
 */
void main(argc, argv)
int argc;
char ** argv;
{
   int ar;
#if defined(RS8250) && defined(LINUX_DEFAULTS)
   int speed  = 0x143; /* 115200, np, 1s, 8b */
#else
   int speed  = 0xE3; /* 9600, np, 1s, 8b */
#endif
   int port   = 0;

   signal(SIGINT, SIG_IGN); /* Ignore ^C */

   init_vid();

   for(ar=1; ar<argc; ar++)
   {
      if(argv[ar][0] == '-' || argv[ar][0] == '/')
	 argv[ar]++;

      if(strcmp(argv[ar], "8") == 0)
	 mask = 0xFF;
      else if(strcmp(argv[ar], "7") == 0)
	 mask = 0x7F;
      else if(argv[ar][0] == 's' )
      {
         long sp; int i;
         sp = atol(argv[ar]+1);
         for(i=0; speeds[i] && speeds[i] != sp ; i++);
         if( speeds[i] == 0 )
         {
            printf("Speed unsupported by BIOS\n");
            exit(1);
         }
         speed = (speed & 0x1F) | (i<<5);
      }
#ifdef TSRPROG
      else if(strcmp(argv[ar], "t") == 0)
	 tsrflag = 1;
#endif
#ifdef MEM_SCR
      else if(strcmp(argv[ar], "d") == 0)
	 phy_scr.dumb = 1;
#endif
      else if(argv[ar][0] == 'p')
      {
         port = argv[ar][1] - '1';
         if( port > 3 || port < 0 || argv[ar][2] != '\0' )
         {
            printf("Illegal Port number\n");
            exit(1);
         }
      }
      else
      {
	 printf("Illegal argument\n");
	 exit(1);
      }
   }

#ifdef TSRPROG
   tsrinit();
#endif

   setup(port, speed);

#ifdef KBDRTS
   init_kbd();
#endif
#ifdef BEEPER
   init_beep();
#endif

#ifdef MEM_SCR
   fetch_page();
   for(ar=0; ar<80; ar++) sysline[ar] = 0x7020;
#endif

   putstr("Terminal emulator.\r\n");

   if( isfossil && mnp_fossil )
      putstr("Fossil communications driver with MNP support found\r\n");
   else if( isfossil )
      putstr("Fossil communications driver found\r\n");
   putstr("Connected\r\n");

   while(keybd())
   {
#ifdef OLDBEEPER
      sounder();
#endif
      serial();
   }

#ifdef OLDBEEPER
   clear_beep();
#endif
#ifdef TSRPROG
   tsrexit(0);
#endif
#ifdef BEEPER
   clear_beep();
#endif
#ifdef KBDRTS
   clear_kbd();
#endif
   clearup();
   end_vid();
   exit(0);
}

/***************************************************************************/

#ifdef TSRPROG

/***************************************************************************
 * Check the tsr isn't already running and install our IRQ handler.
 */
tsrinit()
{
   orig_key_int = _dos_getvect(OLDKB);

   if( orig_key_int != 0 )
   {
      printf("ALREADY RUNNING USE HOTKEY\n");
      exit(0);
   }

   if( tsrflag )
   {
      save_stack(100);

      orig_key_int = _dos_getvect(KEYBD);
      _dos_setvect(OLDKB, orig_key_int);

      dos_screen = (char far *) malloc(line_len*scr_len*2);
      movedata( FP_SEG(phy_scr.scrptr), FP_OFF(phy_scr.scrptr),
                FP_SEG(dos_screen), FP_OFF(dos_screen),
                scr_len*line_len*2);
   }
}

/***************************************************************************
 * Actually go TSR.
 */
tsrexit(rtn)
int rtn;
{
   extern unsigned int _psp;
   extern unsigned int _abrktb, _asizds, _atopsp, _abrkp;
   unsigned int pgmsz;
   struct SREGS srg;

   if( tsrflag )
   {
      int i;

      movedata( FP_SEG(dos_screen), FP_OFF(dos_screen),
                FP_SEG(phy_scr.scrptr), FP_OFF(phy_scr.scrptr),
                scr_len*line_len*2);
      move_cur(dos_line, dos_col);

      _dos_setvect(KEYBD, key_int);

      /* pgmsz Should be Data seg size + DS - PSP */
      segread(&srg);
      pgmsz  = (_asizds>>4);
      pgmsz += 1;
      pgmsz += srg.ds;
      pgmsz -= _psp;

      _dos_keep(rtn, pgmsz);
   }
   /* Not tsring return */
}

/***************************************************************************
 * Check for keycodes to bring TSR in and out
 */
void interrupt far key_int()
{
static int c;
static union REGS regs;
static int flg = 0;

   set_stack();
   c = (((sp_save->ax)>>8) & 0xFF);
   if( c == 0 || c == 0x10 )
   {
      regs.h.ah = c;
      int86(OLDKB, &regs, &regs);
      c = regs.x.ax;
      sp_save->ax = c;

      if( flg == 0 && c == EXITKEY )
      {
         flg=1;
         _enable();
         do_looper();
         _disable();
         flg=0;
      }
      unset_stack();
   }
   else
   {
      unset_stack();
      _chain_intr(orig_key_int);
   }
}

/***************************************************************************
 * This is the routine to run the TSR terminal in the foreground.
 */
do_looper()
{
   int c;
   union REGS regs;

   regs.h.ah = 3; regs.h.bh = page_no; intvid(regs);
   dos_line = regs.h.dh;
   dos_col = regs.h.dl;
   movedata( FP_SEG(phy_scr.scrptr), FP_OFF(phy_scr.scrptr),
             FP_SEG(dos_screen), FP_OFF(dos_screen),
             scr_len*line_len*2);

   curpage->dirtypg = 1; /* Whole screen will be updated 'soon' */
   phy_scr.line = -1;     /* Cursor probably isn't in the right place */

   while(keybd())
   {
#ifdef OLDBEEPER
      sounder();
#endif
      serial();
   }

#ifdef BEEPER
   soundcnt = -1;   /* Turn off beep asap */
#endif

   movedata( FP_SEG(dos_screen), FP_OFF(dos_screen),
             FP_SEG(phy_scr.scrptr), FP_OFF(phy_scr.scrptr),
             scr_len*line_len*2);
   move_cur(dos_line, dos_col);
}

#endif

/***************************************************************************
 * Beeper routines, standard version
 */

#ifdef BEEPER
void
init_beep()
{
   orig_timer_int = _dos_getvect(TIMER);
   _dos_setvect(TIMER, timer_int);
}

void
beep_on()
{
   _disable();
   if(soundval == -1 )
   {
      soundval = inp(0x61);
      lastfreq = freq;
   }
   else if( lastfreq > 128 )
      lastfreq = lastfreq/2;
   else
      lastfreq = freq;
   outp(0x61, soundval|3);
   outp(0x43, 0xb6);
   outp(0x42, lastfreq&0xFF);
   outp(0x42, (lastfreq>>8)&0xFF);
   soundcnt = duration;
   _enable();
}

void interrupt far timer_int()
{
   if(soundval != -1)
   {
      if(soundcnt <= 0)
      {
         outp(0x61, soundval);
         soundval = -1;
      }
      soundcnt--;
   }
   _chain_intr(orig_timer_int);
}

void
beep_set(f,p)
int f,p;
{
   freq = f;
   duration = (p%100)*182/100+1;
}

void
clear_beep()
{
   _dos_setvect(TIMER, orig_timer_int);
   if( soundval != -1 )
      outp(0x61, soundval);
}
#endif

/***************************************************************************
 * Beeper routines, older non-interrupt version.
 */
#ifdef OLDBEEPER
void
beep_on()
{
   if(soundval == -1 )
      soundval = inp(0x61);
   outp(0x61, soundval|3);
   outp(0x43, 0xb6);
   outp(0x42, freq&0xFF);
   outp(0x42, (freq>>8)&0xFF);
   soundcnt = duration;
   soundfreq= freq;
   sounddir = -warble;
}

void
sounder()
{
   if( soundcnt > 0 )
   {
      soundcnt--;
      outp(0x42, soundfreq&0xFF);
      outp(0x42, (soundfreq>>8)&0xFF);
      soundfreq += sounddir;
      if( soundfreq < freq-500 || soundfreq > freq )
         sounddir = -sounddir;
   }
   else if( soundcnt == 0 )
   {
      soundcnt--;
      outp(0x61, soundval);
      soundval = -1;
      sounddir = -warble;
   }
}

void
beep_set(f,p)
int f,p;
{
   freq = f;
   duration = (p%100+1)*700;
   if( p > 99 ) warble=1;
   else warble = 0;
}

void
clear_beep()
{
   if( soundval != -1 )
      outp(0x61, soundval);
}
#endif

/***************************************************************************
 * Fetch a key and forward it to the serial port.
 * While doing this look for the EXIT key and return FALSE if it is pressed
 */

int
keybd()
{
   int c;
   if( (c=get_key()) == -1 ) return TRUE;
#if SYSLKEY == EXITKEY
   if( sysline_warn && (c&0xFF) == '\033' )
      return FALSE;
#else
   if( c == EXITKEY )
      return FALSE;
#endif
#ifdef FORKDOS
#ifdef TSRPROG
   else if(c == DOS_KEY && !tsrflag)
#else
   else if(c == DOS_KEY)
#endif
   {
      int err;
      char * cmd = getenv("COMSPEC");
      if( !cmd || !*cmd ) cmd = "command";
      err = spawnlp(P_WAIT, cmd, "command.com", NULL);
      fetch_page();
      if( err == -1 )
         message("Error trying to run command.com");
      curpage->dirtypg = 1;
      return TRUE;
   }
#endif
#ifdef MEM_SCR
   else if( c == SYSLKEY )
   {
#if SYSLKEY == EXITKEY
      if(syslineno < 0 || syslineno >= scr_len)
      {
	 syslineno = 0;
	 message("Press Escape to exit terminal emulator");
	 sysline_warn = 1;
      }
      else
      {
	 syslineno = -1;
	 sysline_warn = 0;
      }
#else
      if(syslineno < 0 || syslineno >= scr_len)
	 syslineno = 0;
      else
	 syslineno = -1;
#endif
   }
#endif
#ifdef AT_BIOS_KEY
   else switch(curpage->key_mode)
   {
   default: write_unix(c); break;
   case 1:  write_dosemu(c); break;
   case 2:  write_scan(c); break;
   }
#else
#ifdef SCO_KEYMAP
   else if( bios_keymap(c) )
      ;
#endif
   else if( c&0xFF )
      xmit(c & mask);
   else
   {
      xmit(0);
      xmit((c>>8)&mask);
   }
#endif
   return TRUE;
}

int
xmit(ch)
int ch;
{
   if( xmit_put >= xmit_buf+sizeof(xmit_buf) && xmit_get != xmit_buf )
   {
      int count = xmit_put-xmit_get;
      memcpy(xmit_buf, xmit_get, count);
      count = xmit_get - xmit_buf;
      xmit_put -= count;
      xmit_get -= count;
   }

   if( xmit_put >= xmit_buf+sizeof(xmit_buf) )
      return -1;

   *xmit_put++ = ch;

   do_xmit();
   return 0;
}

/***************************************************************************
 * Simple routine to forward output to LPT1 or the print device set as the
 * second argument of the '<ESC> [ 5 ; <dev> i'
 */
#ifdef PRINTER
putprnt(ch)
{
union REGS prt_regs;

   if( in_transprt )
   {
      switch(in_transprt)
      {
      case 2: if( ch == '[' ) { in_transprt++; return; }
	      in_transprt = 0;
              putprnt('\033'); break;
      case 3: if( ch == '4' ) { in_transprt++; return; }
	      in_transprt = 0;
              putprnt('\033'); putprnt('['); break;
      case 4: if( ch == 'i' ) { in_transprt=0; return; }
	      in_transprt = 0;
              putprnt('\033'); putprnt('['); putprnt('4'); break;
      }
      in_transprt = 1;
      if( ch == '\033' ) { in_transprt++; return; }
   }

   prt_regs.x.ax = 0x0200;
   prt_regs.x.dx = print_dev;
   int86(0x17, &prt_regs, &prt_regs);
   if( ( prt_regs.x.ax & 0x2A00 ) != 0 )
   {
      message("Printer Not Ready ... Please correct");
      return;
   }
   prt_regs.x.ax = ch;
   prt_regs.x.dx = print_dev;
   int86(0x17, &prt_regs, &prt_regs);
}
#endif

/***************************************************************************
 * Quick routine to print a string to the screen.
 */
void
putstr(str)
char * str;
{
   while(*str)
      putscrn(*str++);
}


#ifndef MEM_SCR

/***************************************************************************
 * Dummy init and end for BIOS screen.
 */
void init_vid() { }
void end_vid() { }

/***************************************************************************
 * BIOS screen message line; just print the text
 */
void
message(str, val1, val2)
char * str;
int val1, val2;
{
   static char buf[128];
   char * p;
   int i=0;

   sprintf(buf, str, val1, val2);
   strcat(buf, "\r\n");

   putstr(buf);
}

/***************************************************************************
 * BIOS screen put char.
 */
void
putscrn(c)
int c;
{
#ifdef ZMODEM
static lastchar = ' ';
#endif
#ifdef AVATAR
   extern int avcnt;
   if( avcnt > 0 || ( avcnt == 0 && ( c == '\026' || c == '\031')))
   {
      do_avatar(c);
      return;
   }
   if( c == '\f' )
   {
      putstr("\033[2J");
      return;
   }
#endif
#ifdef OLDBEEPER
   if( c == '\007' )
   {
      beep_on();
      return;
   }
#endif /* OLDBEEPER */
#ifdef BEEPER
   if( c == '\007' )
   {
      beep_on();
      return;
   }
#endif /* BEEPER */
#ifdef ZMODEM
   else if( c == 24 && lastchar == '*' ) /* CTRL-X */
   {
#ifdef TSRPROG
       if( !tsrflag )
#endif
         system("dsz d handshake slow rz -rr");
   }

   lastchar = c;
#endif

   if( (c&-32)==0
     && c != '\r'
     && c != '\n'
     && c != '\b'
     && c != '\007'
		  ) return;

   iregs.h.ah = CHAROUT;
   iregs.h.dl = c;
   intdos(&iregs, &oregs);
}

#else

/***************************************************************************
 * Hardware screen, message line is mobile between TOS and BOS.
 */
void
message(str, val1, val2)
char * str;
int val1, val2;
{
   static char buf[128];
   int i;
   int flg=0;

   memset(buf, '\0', sizeof(buf));
   sprintf(buf, str, val1, val2);

   for(i=0; i<80; i++)
   {
      if( buf[i] == '\0' )
      {
         if( sysline[i] == 0x7000 )
            break;
         else
            sysline[i] = 0x7000;
      }
      else
      {
         if( flg == 1 || buf[i] != ' ' )
            sysline[i] = buf[i]|(0x7000);
         else
            sysline[i] = 0x7000;
         if( buf[i] != ' ' ) flg = 1;
      }
   }
   syslinedirty = 1;
   if( syslineno == -1 ) syslinetmp = 1;
#if SYSLKEY == EXITKEY
   sysline_warn = 0;
#endif
}

/***************************************************************************
 * Hardware screen, print character.
 */
void
putscrn(c)
int c;
{ 
#ifdef AVATAR
   extern int avcnt;
#endif
#ifdef ZMODEM
static lastchar = ' ';
#endif

#ifdef SAVEBLOCK
   if( save_flag && c != '\0' )
   {
      *save_ptr++ = c;
      if( save_ptr >= save_buf+sizeof(save_buf) ) save_ptr = save_buf;
   }
#endif

   if(0);
#ifdef AVATAR
   else if( avcnt > 0 || ( avcnt == 0 && ( c == '\026' || c == '\031')))
      do_avatar(c);
#endif
#ifdef ZMODEM
   else if( c == 24 && lastchar == '*' ) /* CTRL-X */
   {
#ifdef TSRPROG
      if( !tsrflag )
#endif
      {
         system("dsz d handshake slow rz -rr");
         fetch_page();
         curpage->dirtypg = 1;
      }
   }
#endif
   else
      putansi(c);

#ifdef ZMODEM
   lastchar = c;
#endif

#ifdef SAVEBLOCK
   if( read_flag == 0 )
#endif
   {
static int cnt = 1000;
      if( c == '\n' ) cnt -= 80;
      if( --cnt < 0 )
      {
         cnt=1000;
         update_page();
      }
   }
}

/***************************************************************************
 * Hardware screen, decode ansi sequence.
 */
void
putansi(c)
int c;
{
static int ansi_state=0;
   int i;
   c &= 0xFF;

   if( ansi_state <= 3 ) switch ( c )
   {
   case '\t': i = (8 - (curpage->col&7) );
	      curpage->col += i;
	      if( curpage->col > line_len )
		 curpage->col     = line_len;
	      return;

   case '\r': curpage->col = 0;
	      return;

   case '\b': if( curpage->col )
		 curpage->col--;
	      return;

   case '\n': curpage->line++;
	      if( curpage->line == scr_len ) scroll(0, scr_len-1, 1);
	      return;

#if defined(SCO_ANSI) || defined(AVATAR)
   case '\f': scroll( scr_len-1, 0, scr_len);
	      curpage->line=0;
	      curpage->col=0;
	      return;
#endif

#ifdef OLDBEEPER
   case '\007':beep_on();
	       return;
#endif /* OLDBEEPER */
#ifdef BEEPER
   case '\007':beep_on();
	       return;
#endif /* BEEPER */

   case '\030':ansi_state = 0; /* Ctrl-X */
               return;

   case '\032':ansi_state = 0; /* Ctrl-Z */
               rawput(0xA8);
               return;

   case '\033':if( phy_scr.dumb == 0 )
	      {
		 ansi_state = 1;

	         for(argc=0; argc<10; argc++) args[argc] = 0;
	         argc    = 0;
	         tchar   = 0;
	         echar   = 0;

		 return;
	      }
	      /*FALLTHROUGH*/
   }

   switch(ansi_state)
   {
   default:ansi_state =0;

   case 0: if( c < ' ' && phy_scr.dumb == 0 ) break;
	   litput(c);
	   break;

   case 1: switch(c)
           {
	   case '\\': /* NO-OP if not in DCS string */
              ansi_state = 0;
              break;
	   case 'P':
	      dcs_ptr = dcs;
	      ansi_state = 2;
	      break;
           case '[':
              dcs_ptr = 0;
	      ansi_state = 2;
	      break;
           case '(':
	      ansi_state = 8;
              break;
	   case '7':
              curpage->sline = curpage->line;
              curpage->scol  = curpage->col;
              ansi_state = 0;
              break;
	   case '8':
              set_pos(curpage->sline, curpage->scol);
              ansi_state = 0;
              break;
#ifdef AT_BIOS_KEY
	   case '>':
              curpage->key_mode = 0;
              ansi_state = 0;
              break;
	   case '=':
              curpage->key_mode = 1;
              ansi_state = 0;
              break;
	   case '<':
              curpage->key_mode = 2;
              ansi_state = 0;
              break;
#endif
#ifdef SCO_REMAP
	   case 'Q':
	      ansi_state = 10;
              break;
#endif
           default:
	      ansi_state = 0;
              rawput('\033');
              rawput(c);
              break;
           }
	   break;

   case 2: if(( c >= ' ' && c < '0' ) || c == ':' || ( c > ';' && c <= '?' ))
	   {
	      tchar = c;
	      ansi_state = 3;
	      break;
	   }
   case 3: if( c >= '0' && c <= '9' )
	   {
	      args[argc] = args[argc]*10 + c - '0';
	      break;
	   }
	   else if(c == ';' )
	   {
	      if( ++argc >= 10 )
	      {
		 ansi_state = 0;
		 break;
	      }
	      args[argc] = 0;
	      break;
	   }
           else if( c >= '@' && c <= '~' )
	   {
	      echar = c;
	      if( dcs_ptr == 0 )
	      {
	         ansi_state = 0;
	         do_ansi( echar, tchar, args, argc+1 );
	      }
	      else
	      {
	         ansi_state = 4;
		 break;
	      }
	   }
	   ansi_state = 0;
	   break;

   /* DCS ... ST processing */
   case 4: if( c == '\030' || dcs_ptr >= dcs+sizeof(dcs)-1 )
              ansi_state = 0;
           else if( c == '\033' )
              ansi_state = 5;
	   else
	      *dcs_ptr++ = c;
	   break;

   case 5: if( c != '\\' )
           {
              if( dcs_ptr >= dcs+sizeof(dcs)-2 || c == '\030' )
	         ansi_state = 0;
              else
	      {
	         *dcs_ptr++ = '\033';
                 if( c != '\033' ) { *dcs_ptr++ = c; ansi_state = 4; }
	      }
	      break;
	   }
	   *dcs_ptr = '\0';
	   ansi_state = 0;

	   if( tchar == '$' && echar == '~' )
	   {
              dcs[80] = 0;
              if( *dcs ) message("%s", dcs);
              else
              {
	         syslineno = -1;
	         sysline_warn = 0;
              }
	   }
	   break;

   /* ESC ( <n>  Screen language processing */
   case 8: ansi_state =0;
           switch(c)
	   {
	   case 'X': curpage->chartran = 0; break;
	   case 'B': curpage->chartran = 1; break;
	   case '0': curpage->chartran = 2; break;
	   case 'A': curpage->chartran = 3; break;

	   case 'U': curpage->chartran = 0; break;
	   case 'u': curpage->chartran = 4; break;

	   default:  curpage->chartran = -1; break;
	   }
	   break;

   /* SCO Function key remapping */
#ifdef SCO_REMAP
   case 10: echar = c-'0'; /* FUNCTION KEY NUMBER 0=F1 .. 9=F10 */
	    message("Define function key %d", echar+1);
	    ansi_state = 11;
	    break;

   case 11: tchar = c;
	    ansi_state = 12;
	    break;

   case 12: if( c == tchar ) ansi_state = 0;
	    if( echar >=0 && echar < KEYTABSIZE )
	    {
	       if( c == tchar )
	       {
	 	 set_key(echar);
	 	 kbtab[keyxlate[echar]] = (0x300|echar);
	       }
	       else if( c != '^' )
                  add_fnchar(c);
	       else
		  ansi_state = 13;
	    }
	    break;
   case 13: if( c == tchar ) ansi_state = 0; else ansi_state = 12;
	    if( echar >=0 && echar < KEYTABSIZE )
	    {
	       if( c == tchar )
	       {
		  set_key(echar);
		  kbtab[keyxlate[echar]] = (0x300|echar);
	       }
	       else if( c != '^' )
                  add_fnchar(c& 0x1F);
	       else
		  add_fnchar(c);
	    }
	    break;
#endif

   }
}

/***************************************************************************
 * Hardware screen, Write any byte to screen through the translation table.
 */

unsigned char iso8859_1[] = {
  4, 177,  -1,  -1,  -1,  -1, 248, 241, 176,  -1, 217, 191, 218, 192, 197,  -1,
 -1, 196,  -1,  95, 195, 180, 193, 194, 179, 243, 242, 227,  -1, 156, 250,  -1,

255, 173, 155, 156,   9, 157, 124,  21,  34,  67, 166, 174, 170,  45,  82, 196,
248, 241, 253,  51,  39, 230,  20, 250,  44,  49, 167, 175, 172, 171,  51, 168,
 65,  65,  65,  65, 142, 143, 146, 128,  69, 144,  69,  69,  73,  73,  73,  73,
 68, 165,  79,  79,  79,  79, 153, 120, 237,  85,  85,  85, 154,  89,  80, 225,
133, 160, 131,  97, 132, 134, 145, 135, 138, 130, 136, 137, 141, 161, 140, 139,
 11, 164, 149, 162, 147, 111, 148, 246, 237, 151, 163, 150, 129, 121, 112, 152
};
/* Yes I know, a good compiler should complain about that array */

void
litput(c)
int c;
{
   c &= 0xFF;
   if( curpage->charflip ) c ^= 0x80;
   if( curpage->chartran ) switch( curpage->chartran )
   {
   case 1:
      if( (c&0x80) )
      {
         if( c&0x60 ) c  = iso8859_1[c&0x7F];
	 else         c &= 0x1F;
      }
      break;
   case 2:
      if( c >= 0x60 && c < 0x80 ) c = iso8859_1[c&0x1F];
      break;
   case 3:
      if( c == '#' ) { c = 156; break; }
      c &= 0x7F;
   default:
      if( c < ' ' || c > '~' ) c = -1;
      break;

   case 4:
      c &= 0x7F;
      break;
   }
   rawput(c);
}

void
rawput(c)
int c;
{
   c &= 0xFF;
   if( curpage->col >= line_len )
   {
      curpage->line++;
      curpage->col=0;
      if( curpage->line == scr_len ) scroll(0, scr_len-1, 1);
   }
   curpage->lines[curpage->line][curpage->col] = c + (curpage->attr<<8);
   curpage->dirty[curpage->line] = 1;
   curpage->col++;
#ifdef SCO_ANSI
   if( curpage->am && curpage->col == line_len )
   {
      if( curpage->am & 2 )
         curpage->col--;
      else
      {
         curpage->line++; curpage->col=0;
      }
      if( ( curpage->am & 1 ) && curpage->line == scr_len )
         scroll(0, scr_len-1, 1);
   }
#endif
}

/***************************************************************************
 * Hardware screen, Set the virtual cursor position.
 */
void
set_pos(line, col)
int line;
int col;
{
   curpage->line = line;
   curpage->col = col;
   if( curpage->line < 0 ) curpage->line = 0;
   if( curpage->line >= scr_len ) curpage->line = scr_len-1;
   if( curpage->col  < 0 ) curpage->col = 0;
   if( curpage->col >=line_len ) curpage->col = line_len-1;
}

/***************************************************************************
 * Hardware screen, Write screen image to physical screen.
 */
void
update_page()
{
   int i;

   if( curpage->col != phy_scr.col || curpage->line != phy_scr.line )
   {
      int c;
      phy_scr.col = curpage->col;
      phy_scr.line = curpage->line;

      if( curpage->col == line_len )
	 c = line_len-1;
      else
	 c = curpage->col;
      move_cur(curpage->line, c);
   }

   if( syslineno == curpage->line )
      syslineno = scr_len -1 -syslineno;

   if( curpage->dirtypg )
   {
      syslineold = syslineno;
      syslinedirty = 0;
   }

   if( syslinetmp && syslineold == -1 )
   {
      if( curpage->line == 0 )
         syslineno = scr_len-1;
      else
         syslineno = 0;
      syslineold = syslineno;
   }

   if( syslineold != syslineno )
   {
      if( syslinetmp )
      {
         syslinetmp = 0;
	 syslineno = -1;
      }
      if( syslineold != -1 )
         curpage->dirty[syslineold] = 1;
      if( syslineno != -1 )
         syslinedirty = 1;
      syslineold = syslineno;
   }

   for(i=0; i<scr_len; i++)
      if( curpage->dirtypg || curpage->dirty[i] )
      {
#ifdef FLDATTR
         copyattr(i);
#endif
         updateline(i);
      }

   if( syslineno != -1 && syslinedirty )
      updateline(syslineno);
   curpage->dirtypg = 0;
}

#ifdef FLDATTR
copyattr(lno)
int lno;
{
int i;
int lastattr = -1;

   for(i=0; i<line_len; i++)
   {
      if((curpage->lines[lno][i]&0xFF) == 0xFF)
         lastattr = (curpage->lines[lno][i]&0xFF00);
      if( lastattr != -1 )
         curpage->lines[lno][i] = ((curpage->lines[lno][i]&0xFF)|lastattr);
   }
}
#endif

void
updateline(to)
int to;
{
   int ll = line_len*2;
   char far * ptr;

   curpage->dirty[to] = 0;
   if( to == syslineno )
   {
      ptr = (char far *) sysline;
      syslinedirty = 0;
   }
   else
      ptr = (char far *)(curpage->lines[to]);
   movedata(FP_SEG(ptr), FP_OFF(ptr),
            FP_SEG(phy_scr.scrptr), ll*to,
             ll);
}

/***************************************************************************
 * Hardware screen, Fetch physical screen into image memory.
 */
void
fetch_page()
{
   int i;

   iregs.h.ah = 3;
   iregs.h.bh = page_no;
   intvid(iregs);
   curpage->line = iregs.h.dh;
   curpage->col  = iregs.h.dl;

   for(i=0; i<scr_len; i++)
      fetch_line(i);
}

void
fetch_line(from)
int from;
{
   int ll = line_len*2;
   char far * ptr;
   if( from == syslineno ) return;
   ptr = (char far *)(curpage->lines[from]);
   movedata(FP_SEG(phy_scr.scrptr), ll*from,
            FP_SEG(ptr), FP_OFF(ptr),
             ll);
}

/***************************************************************************
 * Hardware screen, apply an ansi sequence to the current screen image.
 */
void
do_ansi( cmd, subcmd, args, argc )
int cmd;
int subcmd;
int * args;
int argc;
{
   int ar;
static int colconv[] = { 000, 004, 002, 006, 001, 005, 003, 007,
                         010, 014, 012, 016, 011, 015, 013, 017 };
   switch( subcmd )
   {
   case 0:   switch( cmd )
             {
#ifdef SCO_ANSI
	     case '@': inch(args[0]);
		       break;
#endif
	     case 'A': if( args[0] < 1 ) args[0] = 1;
                       set_pos(curpage->line-args[0], curpage->col);
		       break;
#ifdef SCO_ANSI
             case 'E': curpage->col = 0;
                       /* FALLTHROUGH */
             case 'e':
#endif
	     case 'B': if( args[0] < 1 ) args[0] = 1;
		       set_pos(curpage->line+args[0], curpage->col);
		       break;
#ifdef SCO_ANSI
             case 'a':
#endif
	     case 'C': if( args[0] < 1 ) args[0] = 1;
		       curpage->col    += args[0];
		       if( curpage->col >= line_len )
			  curpage->col     = line_len -1;
		       break;
	     case 'D': if( args[0] < 1) args[0] = 1;
		       curpage->col    -= args[0];
		       if( curpage->col < 0 )
			  curpage->col     = 0;
		       break;
#ifdef SCO_ANSI
	     case 'F': if( args[0] < 1 ) args[0] = 1;
                       set_pos(curpage->line-args[0], 0);
		       break;
#endif
             case 'f':
             case 'H': set_pos(args[0]-1, args[1]-1);
		       break;
	     case 'J': /* cls */
		       if( args[0] == 2)
		       {
		          scroll( scr_len-1, 0, scr_len);
		          curpage->line=0;
		          curpage->col=0;
		       }
		       else if( args[0] == 0 )
                       {
                          clr_eol();
			  if( curpage->line != scr_len-1 )
                             scroll( scr_len-1, curpage->line+1, scr_len);
                       }
		       else if( args[0] == 1 )
                       {
                          clr_sol();
			  if( curpage->line != 0 )
			  scroll( curpage->line-1, 0, scr_len);
                       }
		       break;
	     case 'K': /* Clear eol */
		       if(args[0] == 0 )
                          clr_eol();
		       else if(args[0] == 1 )
                          clr_sol();
		       else if(args[0] == 2 )
                          clrline(curpage->line);
		       break;

#ifdef SCO_ANSI
             case 'L': scroll(scr_len-1, curpage->line, args[0]);
		       break;

             case 'M': scroll(curpage->line, scr_len-1, args[0]);
		       break;

             case 'P': delch(args[0]);
		       break;

             case 'S': scroll(0, scr_len-1, args[0]);
		       break;

             case 'T': scroll(scr_len-1, 0, args[0]);
		       break;

             case 'X': zapch(args[0]);
                       break;

             case 'Z': if( args[0] == 0 ) args[0] = 1;
                       set_pos(curpage->line,((curpage->col+7-8*(args[0]))&-8));
                       break;
#endif
#ifdef PRINTER
	     case 'i': if( args[0] == 5 )
                       {
		          if( args[1] > 0 && args[1] < 3 ) print_dev = args[1];
			  else print_dev = 0;
			  in_transprt = 1;
		       }
		       break;
#endif

             case 'm': /* Attributes */
		       for(ar=0; ar<argc; ar++)
		       switch(args[ar])
		       {
		       case 0: curpage->attr  = curpage->std_col;
			       break;
		       case 1: curpage->attr |= 0x08;
			       break;
#ifdef SCO_ANSI
		       case 2: if( dos_mode != 2 && dos_mode != 7 && ar+2<argc )
                               {
                                  curpage->std_col = colconv[args[ar+1]&0xF]
                                               + (colconv[args[ar+2]&0xF] << 4);
			          curpage->attr  = curpage->std_col;
                                  if( args[ar+2]&0x8 )
                                     set_blink(0);
                                  ar+=2;
                               }
			       break;
		       case 3: if( dos_mode != 2 && dos_mode != 7 && ar+1<argc )
                               {
                                  set_blink(args[++ar]);
                               }
                               break;
#endif
		       case 4: if( dos_mode == 7 )
                                  curpage->attr = ((curpage->attr & 0xF0)|0x01);
			       break;
		       case 5: curpage->attr |= 0x80;
                               set_blink(1);
		               break;
		       case 7:
#ifdef SCO_ANSI
		               if( dos_mode != 2 && dos_mode != 7 && ar+2<argc )
                               {
                                  curpage->rev_col = colconv[args[ar+1]&0xF]
                                               + (colconv[args[ar+2]&0xF] << 4);
                                  if( args[ar+2]&0x8 )
                                     set_blink(0);
                                  ar+=2;
                               }
#endif
			       curpage->attr  = curpage->rev_col;
			       break;
		       case 8: curpage->attr  = 0;
			       break;
#ifdef SCO_ANSI
		       case 10:curpage->charflip = 0;
                               curpage->attr = curpage->std_col;
			       break;
		       case 12:curpage->charflip = 1;
                               curpage->attr = curpage->gr_col;
			       break;
#endif
		       case 30:case 31:case 32:case 33:
                       case 34:case 35:case 36:case 37:
                               if( dos_mode != 2 && dos_mode != 7 )
			          curpage->attr = ((curpage->attr & 0xF8)
                                                    |(colconv[args[ar]-30]));
			       break;
		       case 40:case 41:case 42:case 43:
                       case 44:case 45:case 46:case 47:
                               if( dos_mode != 2 && dos_mode != 7 )
			          curpage->attr
                                          = ((curpage->attr & 0x8F)
                                              |((colconv[args[ar]-40]&0x7)<<4));
			       break;
		       }
		       if( curpage->honour_attr )
                          curpage->clr_col = curpage->attr;
		       break;
#ifdef SCO_ANSI
             case 'g': if( curpage->col == line_len )
	               {
		          curpage->line++;
		          curpage->col=0;
		          if( curpage->line == scr_len )
                             scroll(0, scr_len-1, 1);
	               }
                       curpage->lines[curpage->line][curpage->col] =
		                          args[0] + (curpage->gr_col<<8);
                       curpage->dirty[curpage->line] = 1;
	               curpage->col++;
                       break;
#endif
	     case '}':
#if MAXPAGE != 1
	               if( args[0] == 0 )
                       {
		          args[1] %= MAXPAGE;
		          if( args[1] == phy_scr.this_page ) break;
		          if( pages[args[1]].page == 0 )
		          {
			     phy_scr.this_page = args[1];
                             curpage= pages+phy_scr.this_page;
			     if( reset_page() < 0 )
			        phy_scr.this_page = 0;
		          }
		          else
		          {
		             phy_scr.this_page = args[1];
                             curpage= pages+phy_scr.this_page;
                             curpage->dirtypg = 1;
		          }
                          curpage= pages+phy_scr.this_page;
		       }
#endif
#ifdef SAVEBLOCK
                       if(args[0] == 1)
		       {
                          if( read_flag ) break;
		          read_flag = 1; save_flag = 0;
			  read_block(args[1]);
			  read_flag = 0;
		       }
		       else if( args[0] == 2 && argc == 1 )
		       {
                          if( read_flag ) break;
			  save_flag = 1;
			  save_ptr = save_buf;
		       }
		       else if( args[0] == 2)
		       {
                          if( read_flag ) break;
			  save_block(args[1]);
		       }
#endif
		       break;
	       case 's':
                 curpage->sline = curpage->line;
                 curpage->scol  = curpage->col;
                 break;
	       case 'u':
                  set_pos(curpage->sline, curpage->scol);
                  break;
             }
             break;

#ifdef SCO_ANSI
   case '=': switch( cmd )
	     {
#ifdef OLDBEEPER
             case 'B': beep_set(args[0], args[1]);
                       break;
#endif
#ifdef BEEPER
             case 'B': beep_set(args[0], args[1]);
                       break;
#endif
	     case 'C': /* set cursor (0=>norm, 1=>off, 2=>big, x;y=>setit */
		       if( argc <= 1 ) set_curtype(args[0]-1,0,15);
		       if( argc == 2 ) set_curtype(2,args[0],args[1]);
		       break;
	     case 'E': /* set cursor */
		       set_blink(args[0]);
		       break;
             case 'F': if( dos_mode != 2 && dos_mode != 7 )
			  curpage->std_col = ((curpage->std_col & 0xF0)
                                              |(args[0]&0xF));
                       break;
             case 'G': if( dos_mode != 2 && dos_mode != 7 )
			  curpage->std_col = ((curpage->std_col & 0x0F)
                                              |((args[0]&0xF)<<4));
                       break;
             case 'H': if( dos_mode != 2 && dos_mode != 7 )
			  curpage->rev_col = ((curpage->rev_col & 0xF0)
                                              |(args[0]&0xF));
                       break;
             case 'I': if( dos_mode != 2 && dos_mode != 7 )
			  curpage->rev_col = ((curpage->rev_col & 0x0F)
                                              |((args[0]&0xF)<<4));
                       break;
             case 'J': if( dos_mode != 2 && dos_mode != 7 )
			  curpage->gr_col = ((curpage->gr_col & 0xF0)
                                              |(args[0]&0xF));
                       break;
             case 'K': if( dos_mode != 2 && dos_mode != 7 )
			  curpage->gr_col = ((curpage->gr_col & 0x0F)
                                              |((args[0]&0xF)<<4));
                       break;
	     }
	     break;

   case '?': if( cmd == 'l' || cmd == 'h' )
	     {
                int flg = (cmd == 'h');
		switch(args[0])
		{
	        case 7: curpage->am = flg; break;
		}
	     }
	     break;
#endif
   }
}

/***************************************************************************
 * Hardware screen, scroll a set of lines on the cur screen.
 */
void
scroll(from, to, lines)
int from, to, lines;
{
   int win, tomove, toclr, dir, line, c;
   int * tmp;

   if( curpage->line == scr_len) curpage->line--;
   if( lines <= 0 ) lines = 1;

   dir    = 1;
   if( from > to ) { dir = from ; from = to ; to = dir ; dir = -1 ;}
   win    = to-from+1;
   if( lines > win ) lines = win;
   tomove = win - lines;
   toclr  = lines;

   if( dir == 1 )
   {
      for(line=from; tomove>0; tomove--, line++)
      {
	 tmp = curpage->lines[line];
	 curpage->lines[line] = curpage->lines[line+lines];
	 curpage->lines[line+lines] = tmp;
	 curpage->dirty[line] = 1;
	 curpage->dirty[line+lines] = 1;
      }
      for(line=to; toclr>0; toclr--, line--)
      {
	 clrline(line);
      }
   }
   else
   {
      for(line=to; tomove>0; tomove--, line--)
      {
	 tmp = curpage->lines[line];
	 curpage->lines[line] = curpage->lines[line-lines];
	 curpage->lines[line-lines] = tmp;
	 curpage->dirty[line] = 1;
	 curpage->dirty[line+lines] = 1;
      }
      for(line=from; toclr>0; toclr--, line++)
      {
	 clrline(line);
      }
   }
}

/***************************************************************************
 * Hardware screen, clear one line.
 */
void
clrline(to)
int to;
{
   int * toptr = curpage->lines[to];
   int chars   = line_len;
   int pattern = ' ' + (curpage->clr_col << 8 );

   curpage->dirty[to] = 1;

   do
   {
      *toptr++ = pattern;
      chars--;
   }
   while( chars );
}

/***************************************************************************
 * Hardware screen, clear one page, reset dynamic values.
 */
int
reset_page()
{
static int *sys_page = 0, **sys_lines, *sys_dirty;
   int i;

   if( sys_page == 0 )
   {
      sys_page = (int*) malloc(scr_len*line_len*2);
      sys_lines = (int**) malloc(scr_len*sizeof(int*));
      sys_dirty = (int*) malloc(scr_len*sizeof(int));
      if( sys_page == 0 || sys_lines == 0 || sys_dirty == 0 )
         return -1;
   }

   if( !curpage->reset_done )
   {
      curpage->charflip = 0;
      curpage->chartran = 0;
      curpage->attr     = 0x07;
      curpage->std_col  = 0x07;
      curpage->rev_col  = 0x70;
      curpage->gr_col   = 0x07;
      curpage->clr_col  = 0x07;
      curpage->line     = 0;
      curpage->col      = 0;
      curpage->dirtypg  = 1;

#ifdef AT_BIOS_KEY
      curpage->key_mode = 0;
#endif
      curpage->am       = 1;
      curpage->honour_attr = 1;

#ifdef LINUX_DEFAULTS
      curpage->honour_attr = 0;
      curpage->chartran = 1;
#endif
      curpage->reset_done = 1;
   }

#if MAXPAGE != 1
   if( pages[phy_scr.this_page].page == 0
   || pages[phy_scr.this_page].lines == 0
   || pages[phy_scr.this_page].dirty == 0 )
   {
      pages[phy_scr.this_page].page = (int*) malloc(scr_len*line_len*2);
      pages[phy_scr.this_page].lines = (int**) malloc(scr_len*sizeof(int*));
      pages[phy_scr.this_page].dirty = (int*) malloc(scr_len*sizeof(int));
   }
#endif

   /* Out of memory, use the default page */
   if( pages[phy_scr.this_page].page == 0
   || pages[phy_scr.this_page].lines == 0
   || pages[phy_scr.this_page].dirty == 0 )
   {
      if(pages[phy_scr.this_page].page ) free(pages[phy_scr.this_page].page);
      if(pages[phy_scr.this_page].lines) free(pages[phy_scr.this_page].lines);
      if(pages[phy_scr.this_page].dirty) free(pages[phy_scr.this_page].dirty);
      
      for(i=0; i<MAXPAGE; i++) if( pages[phy_scr.this_page].page == sys_page )
      {
         pages[i].page  = 0;
         pages[i].lines = 0;
         pages[i].dirty = 0;
      }

      pages[phy_scr.this_page].page  = sys_page;
      pages[phy_scr.this_page].lines = sys_lines;
      pages[phy_scr.this_page].dirty = sys_dirty;
   }

   for(i=0; i<scr_len; i++)
      pages[phy_scr.this_page].lines[i]
          = pages[phy_scr.this_page].page+line_len*i;

   scroll( scr_len-1, 0, scr_len);

   return 0;
}

/***************************************************************************
 * Hardware screen, clear to end of line
 */
void
clr_eol()
{
   int pattern = ' ' +(curpage->clr_col << 8 );
   int * toptr = curpage->lines[curpage->line]+curpage->col;
   int x = curpage->col;

   curpage->dirty[curpage->line] = 1;
   while( x < line_len )
   {
      *toptr++ = pattern;
      x++;
   }
}

/***************************************************************************
 * Hardware screen, clear to start of line
 */
void
clr_sol()
{
   int pattern = ' ' +(curpage->clr_col << 8 );
   int * toptr = curpage->lines[curpage->line]+curpage->col;
   int x = curpage->col;

   curpage->dirty[curpage->line] = 1;
   while( x >= 0 )
   {
      *toptr-- = pattern;
      x--;
   }
}

/***************************************************************************
 * Hardware screen, delete some characters.
 */
void
delch(chars)
int chars;
{
   int pattern = ' ' +(curpage->clr_col << 8 );
   int * toptr = curpage->lines[curpage->line]+curpage->col;
   int x = curpage->col;
   int last;

   curpage->dirty[curpage->line] = 1;
   if( chars <=0 ) chars = 1;
   last = line_len-chars;

   while( x < last )
   {
      *toptr = toptr[chars];
      toptr++; x++;
   }

   while( x < line_len )
   {
      *toptr++ = pattern;
      x++;
   }
}

#ifdef SCO_ANSI
/***************************************************************************
 * Hardware screen, insert some characters.
 */
void
inch(chars)
int chars;
{
   int pattern = ' ' +(curpage->attr << 8 );
   int * toptr = curpage->lines[curpage->line]+line_len-1;
   int x = line_len-1;
   int last;

   curpage->dirty[curpage->line] = 1;
   if( chars <= 0 ) chars = 1;

   last = curpage->col+chars;
   chars = -chars;

   while( x >= last )
   {
      *toptr = toptr[chars];
      toptr--; x--;
   }

   while( x >= curpage->col )
   {
      *toptr-- = pattern;
      x--;
   }
}

/***************************************************************************
 * Hardware screen, clear some characters.
 */
void
zapch(chars)
int chars;
{
   int pattern = ' ' +(curpage->clr_col << 8 );
   int * toptr = curpage->lines[curpage->line]+curpage->col;
   int x = curpage->col;
   int last;

   curpage->dirty[curpage->line] = 1;
   if( chars <= 0 ) chars = 1;

   last = curpage->col+chars;
   if( last > line_len ) last = line_len;

   while( x < last )
   {
      *toptr++ = pattern;
      x++;
   }
}
#endif

/***************************************************************************
 * Hardware screen, init video, detect physical screen size and location
 * if it's in a graphic mode complain.
 */
void
init_vid()
{
   union REGS ioregs;
   int phy_mode=0;
   char *p;

   phy_scr.scrptr = (int far *) 0xB0000000;
   phy_scr.dumb   = 0;
   phy_scr.line   = -1;
   phy_scr.col    = -1;

   ioregs.x.ax = 0x0500;
   intvid(ioregs);

   ioregs.h.ah = 15;
   intvid(ioregs);
   dos_mode = ioregs.h.al;
   line_len = ioregs.h.ah;
   page_no = ioregs.h.bh;

   ioregs.h.ah = 3;
   ioregs.h.bh = page_no;
   intvid(ioregs);
   dos_cur = ioregs.x.cx;
   dos_line = ioregs.h.dh;
   dos_col  = ioregs.h.dl;

   ioregs.x.ax = 0x1130;  /* EGA bios info */
   ioregs.x.dx = 0;
   ioregs.h.bh = 0;
   intvid(ioregs);
   if( ioregs.x.dx )
   {
      is_ega_plus = 1;
      scr_len = ioregs.x.dx+1;

      if( ioregs.x.cx < 13 )
         dos_cur = 0x0405;
      else if( dos_mode == 7 )
         dos_cur = 0x0C0D;
      else
         dos_cur = 0x0607;
   }
   else
   {
      is_ega_plus = 0;
      scr_len = 25;
   }

   if( dos_mode == 7 )
      phy_mode=1;
   else
      if(dos_mode == 2 || dos_mode == 3)
      {
         phy_scr.scrptr = (int far * ) 0xB8000000;
         phy_mode=1;
      }
   else if( dos_mode > 0x13 )
     phy_mode = test_svga_mode();

   if( line_len < 80 )
   {
      printf("Screen needs to be at least 80 cols\r\n");
      exit(0);
   }
   if(phy_mode==0) { printf("Unusable display mode\n"); exit(1); }

   phy_scr.this_page = 0;
   if( reset_page() < 0 )
   {
      printf("Memory failure, cannot allocate screen memory\n");
      exit(1);
   }

   putscrn('\r');
   putscrn('\n');
   set_curtype(-1,0,0);
   set_blink(1);
}

/***************************************************************************
 * Hardware screen, test for an svga text mode.
 */
int
test_svga_mode()
{
   union REGS ioregs;
   int ch;

   phy_scr.scrptr = (int far * ) 0xB8000000;

   ioregs.h.ah = 8;
   ioregs.h.bh = page_no;
   intvid(ioregs);

   ch = phy_scr.scrptr[dos_line*line_len+dos_col];
   if( ch != ioregs.x.ax ) return 0;
   phy_scr.scrptr[dos_line*line_len+dos_col] = ~ch;

   ioregs.h.ah = 8;
   ioregs.h.bh = page_no;
   intvid(ioregs);

   phy_scr.scrptr[dos_line*line_len+dos_col] = ch;
   if( ( ~ch ) != ioregs.x.ax ) return 0;
   return 1;
}

/***************************************************************************
 * Hardware screen, reset the physical screen back to normal.
 */
void end_vid()
{
   move_cur(scr_len-1, 0);
   set_blink(1);
   set_curtype(-1,0,0);
}

/***************************************************************************
 * Hardware screen, set the screen blink mode (blink or bg bold).
 */
void
set_blink(flg)
int flg;
{
   union REGS regs;
static int last = -1;
static int first = 1;

/* This takes far too long (~ 8ms ) on some bioses.
 * It apparently waits for a screen retrace and even then it isn't reliable.
 * Will have to go directly to the HW only problem is how to do it
 */

   if( !is_ega_plus ) return;

   flg = (flg != 0);
   if( flg == last ) return;

   regs.h.ah = 0x10;
   regs.h.al = 0x03;
   regs.h.bl = last = flg;

   int86(0x10, &regs, &regs);

   if( first && flg == 0 )  /* Workaround on some EGA's */
   {
      regs.h.ah = 0x06;
      regs.h.al = 0x00;
      regs.h.bh = 0x00;
      regs.h.ch = 1;
      regs.h.cl = 1;
      regs.h.dh = 1;
      regs.h.dl = 1;
      int86(0x10, &regs, &regs);
      curpage->dirty[1] = 1;
      first = 0;
   }
}

/***************************************************************************
 * Hardware screen, set the physical cursor position.
 */
void
move_cur(row, col)
int row, col;
{
   union REGS ioregs;
   ioregs.h.ah = 2;
   ioregs.h.bh = page_no;
   ioregs.h.dh = row;
   ioregs.h.dl = col;
   intvid(ioregs);
}

/***************************************************************************
 * Hardware screen, set the physical cursor type.
 */
void
set_curtype(a,top,bot)
int a, top, bot;
{
   union REGS ioregs;
   ioregs.h.ah = 1;
   ioregs.x.cx = dos_cur;
   if( a == 0 ) ioregs.h.ch = 0x10;
   if( a == 1 ) ioregs.h.ch = 0;
   if( a == 2 ) { ioregs.h.ch = top; ioregs.h.cl = bot; }
   intvid(ioregs);
}

#endif

#ifdef SAVEBLOCK
/***************************************************************************
 * Routines to save a chunk of data for a screen macro.
 */

void
read_block(blk)
int blk;
{
   char * p;
   if( blk < 0 || blk > max_blk ) return;

   p = blocks[blk];
   if( p ) while( *p ) putscrn(*p++ & 0xFF);
}

void
save_block(blk)
int blk;
{
   if( blk < 0 || blk > max_blk ) return;
   if( save_flag == 0 ) return;
   *save_ptr++ = '\0';
   if( blocks[blk] ) free(blocks[blk]);
   blocks[blk] = malloc(save_ptr-save_buf);
   if( blocks[blk] )
      strcpy(blocks[blk], save_buf);
}

#endif

#ifndef RS8250
/***************************************************************************
 * BIOS serial routines, also fossil driver compatible.
 */

int port_no;

void
setup(port, flags)
int port, flags;
{
   int c;
   port_no = port;

   iregs.h.ah = FOSINIT;
   iregs.x.dx = port_no;
   iregs.x.bx = 0;
   int86(RS232, &iregs, &oregs);
   if( oregs.x.ax == 0x1954 )
   {
      if( oregs.h.bh >= 4 )
      {
         isfossil = oregs.h.bh;
	 fossilfunc = oregs.h.bl;
      }
   }

   iregs.h.ah = INITRS;
   iregs.x.dx = port_no;
   iregs.h.al = flags;

   int86(RS232, &iregs, &oregs);
}

void
clearup()
{
}

/***************************************************************************
 * BIOS serial routines, transmit character
 */
void
do_xmit()
{
   int ch;
   ch = *xmit_get++;
   if( xmit_get == xmit_put ) xmit_get = xmit_put = xmit_buf;

   iregs.h.ah = WRITECH;
   iregs.x.dx = port_no;
   iregs.h.al = ch;
   int86(RS232, &iregs, &oregs);

   if( oregs.h.ah & 0x80 )
      printf("Write Failed (0x%x)\r\n", oregs.h.ah);
}

/***************************************************************************
 * BIOS serial routines, receive character
 *
 * Note the _long_ loop for successful collection of bytes so this can
 * actually work moderatly reliably upto about 9600 bps even with a 
 * bios that doesn't use interrupts.
 */
void
serial()
{
   int c;
   int cnt = 2048; /* Lots but eventually will break */

   do_xmit();

   while((c=chrdy()) >=0)
   {
#ifdef PRINTER
      if( in_transprt )
         putprnt(c);
      else
#endif
      putscrn(c);
      if( cnt-- < 0 ) break;
   }
#ifdef MEM_SCR
   update_page();
#endif
}

/***************************************************************************
 * BIOS serial routines, get character from bios is ready.
 */
int
chrdy()
{
   iregs.h.ah = STATUS;
   iregs.x.dx = port_no;
   int86(RS232, &iregs, &oregs);

   if( oregs.x.ax & DTARDY )
   {
      iregs.h.ah = READCH;
      iregs.x.dx = port_no;
      int86(RS232, &iregs, &oregs);

      if( oregs.h.ah & 0x80)
      {
         message("Read Failed 0x%2x", oregs.h.ah); 
         return -1;
      }
      else
         return oregs.h.al & mask;
   }
   return -1;
}
#endif

/****************************************************************************/

#ifdef RS8250
/***************************************************************************
 * Hardware serial routines.
 */

int com_phy[] = { 0x3F8, 0x2F8, 0x3E8, 0x2E8 };
int com_irq[] = {     4,     3,     4,     3 };

#define IRQ_MASK /* 0x88 */ 0x00

int save_bitflgs;
int port_no;
int port_addr;
int port_int;
int rtsflag = 2;

/***************************************************************************
 * Hardware serial routines, setup port and identify fossil port.
 */
void
setup(port, flags)
int port, flags;
{
   long i = -1;
   port_no   = port;

   iregs.h.ah = FOSINIT;
   iregs.x.dx = port_no;
   iregs.x.bx = 0;
   int86(RS232, &iregs, &oregs);
   if( oregs.x.ax == 0x1954 )
   {
      if( oregs.h.bh >= 4 )
      {
         isfossil = oregs.h.bh;
	 fossilfunc = oregs.h.bl;
         iregs.h.ah = 0xE0;
	 iregs.h.al = 6;
	 iregs.x.bx = 0;
	 int86(RS232, &iregs, &oregs);
	 if( oregs.x.bx == 0x4d58 )
	 {
	    mnp_fossil = 1;
	 }
      }
   }

   if( isfossil == 0 )
   {
      port_addr = com_phy[port];
      port_int  = 8 + com_irq[port];

      save_bitflgs = inp(port_addr+4);

      if(flags > 255)
      {
         i = speeds[flags>>5];
         flags &= 0x1F;
         flags |= 0xE0;
      }
   }
   /* else possible message about speed problem */

   iregs.h.ah = INITRS;
   iregs.x.dx = port_no;
   iregs.h.al = flags;

   int86(RS232, &iregs, &oregs);

   if( isfossil == 0 )
   {
      if( i > 0 )
      {
         long brd = 115200L / i;
         outp(port_addr+3, inp(port_addr+3) | 0x80);
         outp(port_addr+0, brd & 0xFF);
         outp(port_addr+1, (brd >> 8) & 0xFF);
         outp(port_addr+3, inp(port_addr+3) & 0x7F);
      }

      outp(port_addr+2, 0x47); /* 16550A's FIFO cleared & at 4 char timeout */
      setints();
      outp(port_addr+4, inp(port_addr+4) | 0xB );
   }
}

/***************************************************************************
 * Hardware serial routines, reset hardware.
 */
void
clearup()
{
   if( isfossil == 0 )
   {
      outp(port_addr+4, save_bitflgs);
      clrints();
   }
}

/***************************************************************************
 * Hardware serial routines, get character from buffer.
 */
int
get_c()
{
   int c;
   if( isfossil && putptr == empty)
      chfetch();

   if(putptr == empty )
      c = -1;
   else
   {
      _disable();
      if(getptr >= full) getptr = empty;
      c = *getptr++;
      nsize--;
      if( nsize == 0 )
         getptr = putptr = empty;
      _enable();
      c &= mask;
   }

   if( isfossil == 0 )
   {
      if( rtsflag )
      {
         /* if( nsize>sizeof(empty)/2) rtsflow(1) */ ;
      }
      else
      {
         if( nsize<sizeof(empty)/4) rtsflow(0);
      }
   }

   return c;
}

/***************************************************************************
 * Hardware serial routines, get characters from FOSSIL driver.
 */
void
chfetch()
{
static struct SREGS sregs;

   if( fossilfunc >= 0x18 )
   {
      iregs.h.ah = 0x18;
      iregs.x.cx = sizeof(empty);
      iregs.x.di = (unsigned) empty;
      iregs.x.dx = port_no;
      segread(&sregs);
      sregs.es = sregs.ds;
      int86x(RS232, &iregs, &oregs, &sregs);
      putptr = empty + oregs.x.ax;
      nsize = oregs.x.ax;
      return;
   }

   iregs.h.ah = STATUS;
   iregs.x.dx = port_no;
   int86(RS232, &iregs, &oregs);

   if( oregs.x.ax & DTARDY )
   {
      iregs.h.ah = READCH;
      iregs.x.dx = port_no;
      int86(RS232, &iregs, &oregs);

      if( oregs.h.ah & 0x80)
         message("Read Failed 0x%2x", oregs.h.ah); 
      else
      {
         *putptr++ = oregs.h.al & mask;
         nsize++;
      }
   }
}

/***************************************************************************
 * Hardware serial routines, send characters to port.
 */
void
do_xmit()
{
   int cnt;
   int ch;
   if( xmit_put == xmit_buf ) return;

   if( isfossil )
   {
      ch = *xmit_get++;
      if( xmit_get == xmit_put ) xmit_get = xmit_put = xmit_buf;

      iregs.h.ah = WRITECH;
      iregs.x.dx = port_no;
      iregs.h.al = ch;
      int86(RS232, &iregs, &oregs);

      if( oregs.h.ah & 0x80 )
         message("Write Failed (0x%x)\r\n", oregs.h.ah);
   }
   else
   {
      if( !(inp(port_addr+5) & 0x20) ) return;

      ch = *xmit_get++;
      if( xmit_get == xmit_put ) xmit_get = xmit_put = xmit_buf;

      outp(port_addr+0, ch);
   }
}

/***************************************************************************
 * Hardware serial routines, characters from port to screen.
 */
void
serial()
{
   int c;

   do_xmit();

   if( (c= get_c()) >= 0 )
   {
#ifdef PRINTER
      if( in_transprt )
         putprnt(c);
      else
#endif
         putscrn(c);
   }
#ifdef MEM_SCR
   else
      update_page();
#endif
}

/***************************************************************************
 * Hardware serial routines, interrupt handling.
 */
void ( interrupt far * int_rs_orig) _((void));

void interrupt far int_rs _((void));

void
setints()
{
   int c;
   int c1;

   int_rs_orig = _dos_getvect(port_int);

   _dos_setvect(port_int, int_rs);

   outp(port_addr+1, 0x01);
   outp(port_addr+4, inp(port_addr+4)&0xF);

   inp(0x21);
   outp(0x21, IRQ_MASK);

   inp(port_addr+0);
}

void
clrints()
{
   int c;

   _disable();
   c = inp(port_addr+3);
   outp(port_addr+3, c&0x7F);
   outp(port_addr+1, 0);
   outp(port_addr+3, c);
   _enable();

   _dos_setvect(port_int, int_rs_orig);
}

/***************************************************************************
 * Hardware serial routines, interrupt routine.
 *
 * Normally this _only_ reads characters from the serial port, flow control
 * is only asserted if the interrupts are arriving too fast for the main
 * job to run.
 */
void interrupt far int_rs()
{
register int lsr, prt_addr = port_addr;

   while( (lsr=inp(prt_addr+5)) & 0x1F )
   {
      if( rtsflag )
      {
         /* Is buffer too full ? */
         if( nsize>sizeof(empty)/4*3 )
         {
            register int i;
            rtsflag = 0;
            i=prt_addr+4;
            outp(i, inp(i) & ~2 );
         }
      }
      else
         if( nsize>=sizeof(empty)-2 ) /* It didn't respond to RTS! */
            break;

      {
         register char * ptr = putptr;
         if( lsr & 0x1A )
         {
            if(ptr >= full) ptr = empty;
            if( lsr & 0x10 )
               *ptr++ = 0x07;	/* Flag break */
            else
               *ptr++ = '\032';	/* Flag overrun or frame error */
            nsize++;
         }
         if( lsr & 0x01 )
         {
            if(ptr >= full) ptr = empty;
            *ptr++ = inp(prt_addr+0);
            nsize++;
         }
         putptr = ptr;
      }
   }
   outp(prt_addr+1, 0x00);
   outp(0x20, 0x20);		/* Ensure late irq's retrigger */

   if( nsize<sizeof(empty)-2 )	/* But only if we ain't full */
      outp(prt_addr+1, 0x01);
}

/***************************************************************************
 * Hardware serial routines, assert RTS/CTS flow control
 */
void
rtsflow(stop)
int stop;
{
   rtsflag = (stop?0:2);

   if(stop)
      outp(port_addr+4, inp(port_addr+4) & ~2 );
   else
   {
      outp(port_addr+4, inp(port_addr+4) | 0xB );
      outp(port_addr+1, 0x01);
   }
}

#ifdef KBDRTS
/***************************************************************************
 * These routines force the RTS flow control on when in the keyboard
 * interrupt; this is sometimes needed to deal with nasty TSRs and
 * bad BIOSes.
 */
void
init_kbd()
{
   orig_kbd_int = _dos_getvect(KBDINT);
   _dos_setvect(KBDINT, kbd_int);
}

void interrupt far kbd_int()
{
register int i;
   outp(port_addr+4, inp(port_addr+4) & ~2 );

/* Flag overrides */
#ifdef AT_BIOS_KEY
#ifdef __BORLANDC__
   _AH = 0x12; geninterrupt(0x16); i = _AX;
   _CX = 0xFE00 + (i&3) + ((i&0xF00)>>6) + ((i>>9)&0x40);
   _AH = 0x05; geninterrupt(0x16);
#endif
#endif

   int_rs();
   orig_kbd_int();
   outp(port_addr+4, inp(port_addr+4) | rtsflag );
}

void
clear_kbd()
{
   _dos_setvect(KBDINT, orig_kbd_int);
}
#endif
#endif

/***************************************************************************
 * AVATAR -> ANSI conversion routines.
 *
 * Only the hardware screen version can use the avatar+ codes
 * (except insert mode which needs large changes)
 */
#ifdef AVATAR
#ifndef MEM_SCR

char avbuf[270];
int avcnt = 0;

               /* @  A  B  C  D  E  F  G  H  I  J  K  L  M  N */
int avsizes[] = { 0, 3, 2, 2, 2, 2, 2, 2, 4, 2, 7, 7, 5, 6, 2 };
int avzero [] = { 0, 3, 2, 2, 2, 2, 2, 2, 4 };
int colconv[] = { 0, 4, 2, 6, 1, 5, 3, 7 };

do_avatar(ch)
{
   char tbuf[270];
   int i, j;

   if( avcnt == 0 )
   {
      avbuf[0] = ch;
      avcnt=1;
      return;
   }
   else
   {
      avbuf[avcnt++] = ch;
      if( avbuf[0] == '\031' )
      {
	 if( avcnt < 3 ) return;
	 j = (avbuf[2]&0x7F);
	 avcnt = -1;
	 for(i=0; i<j; i++)
	    putscrn(avbuf[1]);
	 avcnt = 0;
         return;
      }
      avbuf[1] &= 0x3F;
      if( avbuf[1] == '\031' )
      {
         if( avcnt < 3 || avcnt < (avbuf[2]&0xFF)+4 )
	    return;
      }
      else if( avbuf[1] < 1 || avbuf[1] >= sizeof(avsizes)/sizeof(int) )
      {
	 avcnt = -1;
	 putscrn('\026');
	 putscrn(ch);
	 avcnt = 0;
	 return;
      }
      else
         if( avcnt < avsizes[avbuf[1]]) return;
      avcnt = -1;
      tbuf[0] = '\0';
      switch( avbuf[1] )
      {
      case 1: sprintf(tbuf, "\033[0;3%d;4%dm",
                            colconv[avbuf[2]&0x7], colconv[(avbuf[2]>>4)&0x7]);
	      if( avbuf[2]&0x08 ) sprintf(tbuf+strlen(tbuf), "\033[1m");
	      break;
      case 2: sprintf(tbuf, "\033[5m");
	      break;
      case 3: sprintf(tbuf, "\033[A");
	      break;
      case 4: sprintf(tbuf, "\033[B");
	      break;
      case 5: sprintf(tbuf, "\033[D");
	      break;
      case 6: sprintf(tbuf, "\033[C");
	      break;
      case 7: sprintf(tbuf, "\033[K");
	      break;
      case 8: if( avbuf[2] > 25 || avbuf[2] < 1 )
	      {
		 putscrn(avbuf[2]);
		 putscrn(avbuf[3]);
	      }
	      else if( avbuf[3] > 80 || avbuf[3] < 1 )
	      {
	         sprintf(tbuf, "\033[%dH", avbuf[2]);
		 j=strlen(tbuf);
		 tbuf[j] = avbuf[3];
		 tbuf[j+1] = '\0';
	      }
	      else
	         sprintf(tbuf, "\033[%d;%dH", avbuf[2], avbuf[3]);
	      break;
      case 25: memcpy(tbuf, avbuf, 270);
               j = (tbuf[tbuf[2]+3]&0xFF);
	       avcnt = 0;
	       for(;j>0; j--)
	          for(i=0; i<(tbuf[2]&0xff); i++)
		     putscrn(tbuf[i+3]);

	       tbuf[0] = '\0';
	       break;
      default: sprintf(tbuf, "Unsupported avatar CTRL-V CTRL-%c ",
                              '@'+ avbuf[1] );
	      break;
      }
      for(i=0; tbuf[i]; i++)
	 putscrn(tbuf[i]);
      avcnt = 0;
   }
}

#else  /* def MEM_SCR */

char avbuf[270];
int avcnt = 0;

               /* @  A  B  C  D  E  F  G  H  I  J  K  L  M  N */
#ifdef AVATAR_PLUS
int avsizes[] = { 0, 3, 2, 2, 2, 2, 2, 2, 4, 2, 7, 7, 5, 6, 2 };
#else
int avsizes[] = { 0, 3, 2, 2, 2, 2, 2, 2, 4 };
#endif
int colconv[] = { 0, 4, 2, 6, 1, 5, 3, 7 };

do_avatar(ch)
{
   char tbuf[270];
   int i, j;

   if( avcnt == 0 )
   {
      avbuf[0] = ch;
      avcnt=1;
      return;
   }
   else
   {
      avbuf[avcnt++] = ch;
      if( avbuf[0] == '\031' )
      {
	 if( avcnt < 3 ) return;
	 j = (avbuf[2]&0x7F);
	 avcnt = -1;
	 for(i=0; i<j; i++)
	    litput(avbuf[1]);
	 avcnt = 0;
         return;
      }
      avbuf[1] &= 0x3F;
      if( avbuf[1] == '\031' )
      {
         if( avcnt < 3 || avcnt < (avbuf[2]&0xFF)+4 )
	    return;
      }
      else if( avbuf[1] < 1 || avbuf[1] >= sizeof(avsizes)/sizeof(int) )
      {
	 avcnt = -1;
	 litput('\026');
	 litput(ch);
	 avcnt = 0;
	 return;
      }
      else
         if( avcnt < avsizes[avbuf[1]]) return;
      avcnt = -1;
      tbuf[0] = '\0';
      switch( avbuf[1] )
      {
      case 1: if( dos_mode != 2 && dos_mode != 7 )
                  curpage->attr = (avbuf[2]&0x7F);
              else
                  curpage->attr = ((avbuf[2]&0x08)|0x07);
	      if( curpage->honour_attr )
		  curpage->clr_col = curpage->attr;
	      break;
      case 2: curpage->attr |= 0x80;
	      if( curpage->honour_attr )
		  curpage->clr_col = curpage->attr;
	      break;
      case 3: set_pos(curpage->line-1, curpage->col);
	      break;
      case 4: set_pos(curpage->line+1, curpage->col);
	      break;
      case 5: curpage->col--;
	      if( curpage->col < 0 )
		 curpage->col     = 0;
	      break;
      case 6: curpage->col++;
	      if( curpage->col >= line_len )
		 curpage->col     = line_len -1;
	      break;
      case 7: clr_eol();
	      break;
      case 8: set_pos(avbuf[2]-1, avbuf[3]-1);
	      break;
#ifdef AVATAR_PLUS
/*    case 9: insert mode  (Gaud! this means lots of changes) */

      case 10: /* scroll up */
	       scroll_up(avbuf[2], avbuf[3], avbuf[4], avbuf[5], avbuf[6]);
	       break;

      case 11: /* scroll down */
	       scroll_down(avbuf[2], avbuf[3], avbuf[4], avbuf[5], avbuf[6]);
	       break;

      case 12: clear_area(curpage->line, curpage->col,
			  avbuf[3], avbuf[4], avbuf[2] , ' ');
	       break;
      case 13: clear_area(curpage->line, curpage->col,
			  avbuf[4], avbuf[5], avbuf[2], avbuf[3]);
	       break;

      case 14: /* del char */
               delch(1);
               break;
#endif
      case 25: memcpy(tbuf, avbuf, 270);
               j = (tbuf[tbuf[2]+3]&0xFF);
	       avcnt = 0;
	       for(;j>0; j--)
	          for(i=0; i<(tbuf[2]&0xff); i++)
		     putscrn(tbuf[i+3]);

	       tbuf[0] = '\0';
	       break;
      default: sprintf(tbuf, "Unsupported avatar CTRL-V CTRL-%c ",
                              '@'+ avbuf[1] );
	      break;
      }
      for(i=0; tbuf[i]; i++)
	 putscrn(tbuf[i]);
      avcnt = 0;
   }
}

#ifdef AVATAR_PLUS
clear_area(r, c, h, w, a, cc)
int w,h;
int r, c;
{
   int i, j;
   int ch;
   int attr;

   attr = a;

   ch = (cc&0xFF)+(attr<<8);

   if( r >= scr_len || c >= line_len 
      || r+h < 0 || c+w < 0 || h < 0 || w < 0 ) return;
   if( r < 0 ) { h -= r; r = 0; }
   if( c < 0 ) { w -= c; w = 0; }
   if( r+h > scr_len ) { h = scr_len-r; }
   if( c+w > line_len ) { w = line_len-c; }

   for(i=0; i<h; i++)
   {
      for(j=0; j<w; j++)
      {
	 curpage->lines[r+i][c+j] = ch;
      }
   }
   dirty_lines(r, h);
   curpage->attr = attr;
}

scroll_up(lines, tlr, tlc, brr, brc)
int lines, tlr, tlc, brr, brc;
{
   int slines;
   int rows, cols, i, k;
   if( tlr < 1 ) tlr = 1;
   if( tlc < 1 ) tlc = 1;
   if( brr > scr_len ) brr = scr_len;
   if( brc > line_len ) brc = line_len;
   if( tlr > brr ) return;
   if( tlc > brc ) return;

   rows = brr-tlr+1;
   cols = brc-tlc+1;
   tlr--; tlc--; brc--; brr--;

   if( lines <= 0 || lines >= rows )
   {
      clear_area(tlr, tlc, rows, cols, curpage->attr, ' ');
      return;
   }
   slines = rows - lines;
   for(i=0;i < slines;i++)
   {
      /* copy from line tlr+i+lines to line tlr+i */
      for(k=tlc;k<=brc;k++)
      {
	 curpage->lines[tlr+i][k] = curpage->lines[tlr+lines+i][k];
      }
   }
   for(i=slines; i<rows; i++)
   {
      /* zap line tlr+i */
      for(k=tlc;k<=brc;k++)
      {
	 curpage->lines[tlr+i][k] = (curpage->attr<<8)+' ';
      }
   }
   dirty_lines(tlr, rows);
}

scroll_down(lines, tlr, tlc, brr, brc)
int lines, tlr, tlc, brr, brc;
{
   int slines;
   int rows, cols, i, k;
   if( tlr < 1 ) tlr = 1;
   if( tlc < 1 ) tlc = 1;
   if( brr > scr_len ) brr = scr_len;
   if( brc > line_len ) brc = line_len;
   if( tlr > brr ) return;
   if( tlc > brc ) return;

   rows = brr-tlr+1;
   cols = brc-tlc+1;
   tlr--; tlc--; brc--; brr--;

   if( lines <= 0 || lines >= rows )
   {
      clear_area(tlr, tlc, rows, cols, curpage->attr, ' ');
      return;
   }
   slines = rows - lines;
   for(i=rows-1;i >= lines;i--)
   {
      /* copy from line tlr+i to line tlr+i+lines */
      for(k=tlc;k<=brc;k++)
      {
	 curpage->lines[tlr+i][k] = curpage->lines[tlr+i-lines][k];
      }
   }
   for(i=0; i<lines; i++)
   {
      /* zap line tlr+i */
      for(k=tlc;k<=brc;k++)
      {
	 curpage->lines[tlr+i][k] = (curpage->attr<<8)+' ';
      }
   }
   dirty_lines(tlr, rows);
}

dirty_lines(top, count)
int top, count;
{
   int i;
   for(i=0; i<count; i++)
   {
      curpage->dirty[top+i] = 1;
   }
}
#endif
#endif
#endif


#ifdef AT_BIOS_KEY

static int bioskeydecode[] = {
	0x0000, 0x3280, 0x3131, 0x3132, 0x3133, 0x3134, 0x3135, 0x3136,
	0x3137, 0x3138, 0x3139, 0x3130, 0x312d, 0x313d, 0x3308, 0x1209,
	0x3171, 0x3177, 0x3165, 0x3172, 0x3174, 0x3179, 0x3175, 0x3169,
	0x316f, 0x3170, 0x315b, 0x315d, 0x330d, 0x0000, 0x3161, 0x3173,
	0x3164, 0x3166, 0x3167, 0x3168, 0x316a, 0x316b, 0x316c, 0x313b,
	0x0100, 0x0100, 0x0000, 0x0400, 0x317a, 0x3178, 0x3163, 0x3176,
	0x3162, 0x316e, 0x316d, 0x312c, 0x312e, 0x312f, 0x0000, 0x0700,
	0x0000, 0x3520, 0x0000, 0x0481, 0x0482, 0x0483, 0x0484, 0x0485,
	0x0486, 0x0487, 0x0488, 0x0489, 0x048a, 0x0000, 0x0000, 0x0697,
	0x0698, 0x0699, 0x0700, 0x0694, 0x0695, 0x0696, 0x0700, 0x0691,
	0x0692, 0x0693, 0x0690, 0x069a, 0x1481, 0x1482, 0x1483, 0x1484,
	0x1485, 0x1486, 0x1487, 0x1488, 0x1489, 0x148a, 0x2481, 0x2482,
	0x2483, 0x2484, 0x2485, 0x2486, 0x2487, 0x2488, 0x2489, 0x248a,
	0x3481, 0x3482, 0x3483, 0x3484, 0x3485, 0x3486, 0x3487, 0x3488,
	0x3489, 0x348a, 0x0000, 0x2694, 0x2696, 0x2691, 0x2693, 0x2697,
	0x3131, 0x3132, 0x3133, 0x3134, 0x3135, 0x3136, 0x3137, 0x3138,
	0x3139, 0x3130, 0x002d, 0x003d, 0x2699, 0x048b, 0x048c, 0x148b,
	0x148c, 0x248b, 0x248c, 0x348b, 0x348c, 0x2698, 0x2700, 0x2695,
	0x2700, 0x2692, 0x2690, 0x269a, 0x2009, 0x2700, 0x2700, 0x3697,
	0x3698, 0x3699, 0x0000, 0x3694, 0x3695, 0x3696, 0x0000, 0x3691,
	0x3692, 0x3693, 0x3690, 0x369a, 0x3700, 0x3009, 0x3700, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0700, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

int get_key()
{
static int last_flgs = 0;
static int force_flgs[] = {0, 2, 4, 8};
static int this_flags = -1;

#ifdef __BORLANDC__
static int kbd_type = -1;
#else
static int kbd_type = 0;
#endif

   int ch, flgs, scan, flgs2;
   int scand;
   int i;

try_again:;	/* If we get a scan generator */

#ifdef __BORLANDC__
   if( kbd_type < 0 )
   {
      kbd_type = 0;

      for(i=0; i<16;i++)
      {
         _AH = 0x01; geninterrupt(0x16); if((_FLAGS&0x40)) break;
         _AH = 0x00; geninterrupt(0x16);
      }
      _AH = 0x05; _CX= 0xFFFF; geninterrupt(0x16);
      for(i=0; i<16;i++)
      {
         _AH = 0x11; geninterrupt(0x16); if((_FLAGS&0x40)) break;
         _AH = 0x10; geninterrupt(0x16);
	 if( _AX == 0xFFFF )
	 {
	     kbd_type = 0x10;
	     break;
	 }
      }
   }

   _AH = kbd_type+1; geninterrupt(0x16);
   i = (_FLAGS&0x40);

   _AH = kbd_type+2; geninterrupt(0x16); flgs = _AX;
   if( !kbd_type ) flgs &= 0xFF;

#else
   i = !_bios_keybrd(_KEYBRD_READY);

   flgs = _bios_keybrd(_KEYBRD_SHIFTSTATUS);
#endif
   if(!(flgs&0x8)) flgs &= ~0x0A00;

   if(i)
   {
      if( flgs != last_flgs )
      {
         ch = 0;
         last_flgs = flgs;
         i=0;
         if( (flgs & 0x500) == 0x500 ) ch |= KEY_CTRL;
         if( (flgs & 0xA00) == 0xA00 ) ch |= KEY_ALT|KEY_ALTGR;
         if(!i)
	 {
            if( (flgs & 0x001) ) i++;
            if( (flgs & 0x002) ) i++;
            if( (flgs & 0x004) ) i++;
            if( (flgs & 0x008) ) i++;
	 }

	 if( i>1 )
	 {
            if( flgs & 0x4 ) ch |= KEY_CTRL;
            if( flgs & 0x3 ) ch |= KEY_SHIFT;
            if( (flgs & 0x0A00) )
               ch |= ((flgs&0x0200)?KEY_ALT:0) + ((flgs&0x0800)?KEY_ALTGR:0) ;
            else
               ch |= ((flgs&0x08)?KEY_ALT:0);
	 }

         if( ch )
         {
#ifdef DEBUG
            cprintf( "BIOS ----, FLGS: %04x:", flgs);
#endif
            return (ch + KEY_SPECIAL);
         }
      }
      return -1;
   }
   else
   {
#ifdef __BORLANDC__
      _AH = kbd_type; geninterrupt(0x16);
      ch = (unsigned) _AX;
#else
      ch = _bios_keybrd(_KEYBRD_READ);
#endif
   }

   if( this_flags != -1 )
   {
      flgs = ( flgs & 0x70F0 )
           + ( this_flags & 3 )
	   + ((this_flags & 0x3C) << 6 )
	   + ((this_flags & 0x40) << 9 );
      if( flgs & 0x500 ) flgs |= 0x04;
      if( flgs & 0xA00 ) flgs |= 0x08;
      this_flags = -1;
   }

   if( ( ch & 0xFF00 ) == 0xFE00 ) { this_flags = ch; goto try_again; }

   last_flgs = flgs;
#ifdef DEBUG
   cprintf( "BIOS %04x, FLGS: %04x:", ch, flgs);
#endif

   flgs &= 0x0A0F;
   if( ch == 0 ) return KEY_OTHER + 0x03;

   scan = ((ch>>8)&0xFF);
   ch &= 0xFF;

   if( scan == 0 ) return ch + KEY_DIRECT;
   if( scan == 0xE0 ) scan = 0xA4 + 2*(ch < ' ');

   flgs2 = 0;
   if( ch == 0xE0 ) { flgs2 |= KEY_PAD; ch = 0; }

   /* Check for forced flags */
   if( ch == 0 && (flgs&0xF) == 0 )
      flgs |= force_flgs[(bioskeydecode[scan]>>12)&3];

   if( flgs & 0x4 ) flgs2 |= KEY_CTRL;
   if( flgs & 0x3 ) flgs2 |= KEY_SHIFT;

   if( (flgs & 0x0A00) )
      flgs2 |= ((flgs&0x0200)?KEY_ALT:0) + ((flgs&0x0800)?KEY_ALTGR:0) ;
   else
      flgs2 |= ((flgs&0x08)?KEY_ALT:0);

   if( ch ) switch(flgs2)
   {
   case 0: case KEY_CTRL: case KEY_SHIFT:
      switch((bioskeydecode[scan]>>8)&7)
      {
      case 1:
         return ch;
      case 2:
         if( flgs2 == 0 ) return ch + KEY_OTHER;
         break;
      case 3:
         if( (flgs2 & ~KEY_CTRL) == 0 ) return ch + KEY_OTHER;
         break;
      case 4:
         if( (flgs2 & ~KEY_SHIFT) == 0 ) return ch;
         if( flgs2 == KEY_CTRL ) return (ch&0x1F);
         break;
      case 5:
         if( flgs2 == 0 ) return ch;
         break;
      case 6:
         if( flgs2 == 0 ) return ch + KEY_PAD;
         break;
      case 7: return ch + KEY_PAD;
      }
   }

   flgs2 |= KEY_SPECIAL;

   if( ((bioskeydecode[scan]>>8)&6) == 6 && (flgs&0x08) == 0 && kbd_type )
      flgs2 ^= KEY_PAD;
   
   if( ((bioskeydecode[scan]>>8)&7) == 4 && ch )
      ;
   else if(bioskeydecode[scan]&0xFF)
      return flgs2 + (bioskeydecode[scan]&0xFF);

   return flgs2 + KEY_DIRECT + scan;
}

unsigned char scan_desc[] = {
 0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8,   0,  
 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 10,  'C', 'a', 's',
 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 0,   0,   'S', 0,   'z', 'x', 'c', 'v',
 'b', 'n', 'm', ',', '.', '/', 'T', 0,   'A', ' ', 0,   0,   0,   0,   0,   0,  
 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
 'E', 'F', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

char hextbl[] = "0123456789abcdef";

/* Write scancodes, in this mode you really need a compatible program on
 * the host, but you can just about type 'reset<cr>' and have it work
 */
void
write_scan(scan)
int scan;
{
static int lastscan = -1;
   int by;

   scan &= 0xFF;

   if( scan == lastscan+0x80 )
      { xmit('~'); xmit('\b'); }
   else if( by = scan_desc[scan] )
      xmit(by);
   else if( by = scan_desc[scan&0x7F] )
      { xmit('!'); xmit(by); }
   else
      { xmit('~'); xmit(hextbl[scan>>4]); xmit(hextbl[scan&0xF]); }
   lastscan = scan;
}

/*
 * This keyboard gives a pattern of control sequences that makes just about
 * every keypress distinct. It does need a compatible host program but its
 * useable as a generic terminal type unlike the scancodes.
 */
void
write_dosemu(ch)
int ch;
{
#define CT_KEY 0x1E
   int by = (ch & 0xFF);

   /* First we swap round ^M and Return, likewise BS, Tab and Esc  */
   if( ( ch & 0xFF00 ) == KEY_OTHER || ( ch & 0xFF00 ) == 0 )
   {
      /* Make it so we can detect the 'other' keys */
      if( strchr("\b\t\r\n\033\003\177", by) )
      {
	 if( ch & KEY_OTHER )
	    ch ^= KEY_OTHER;
	 else
	    by = ((ch = KEY_SPECIAL+KEY_CTRL+tolower(by+'@')) & 0xFF);
      }
   }

   if( ch & 0xFF00 )
   {
      if( ch & KEY_SPECIAL )
      {
         if( by == 0 ) return;

	 if( ch & KEY_ALT )     { xmit(CT_KEY); xmit('a'); }
	 if( ch & KEY_CTRL )    { xmit(CT_KEY); xmit('c'); }
	 if( ch & KEY_SHIFT )   { xmit(CT_KEY); xmit('s'); }
	 if( ch & KEY_ALTGR )   { xmit(CT_KEY); xmit('g'); }
	 if( ch & KEY_PAD )     { xmit(CT_KEY); xmit('k'); }
	 /* if( ch & KEY_OTHER )   { xmit(CT_KEY); xmit('o'); } */

	 if( ch & KEY_DIRECT )
	 {
	    xmit(CT_KEY); xmit('~');
            xmit(hextbl[by>>4]); xmit(hextbl[by&0xF]);
	 }
         else if((by&0xF0)==0x80 )
	 {
static char fkeys[] = "01234567890-=..";
	    if( by == 0x80 )
            {
	       xmit('\033');
	       xmit('\033');
            }
	    else
	    {
	       xmit(CT_KEY);
	       xmit(fkeys[by&0xF]);
	    }
	 }
	 else if((by&0xF0)==0x90 )
	 {
	    xmit(CT_KEY);
	    xmit('K');
	    xmit(hextbl[by&0xF]);
         }
	 else
	    xmit(by);
      }
      else
      {
	 if( ch & KEY_PAD )     { xmit(CT_KEY); xmit('k'); }
	 if( ch & KEY_OTHER )   { xmit(CT_KEY); xmit('o'); }
	 if( ch & KEY_DIRECT )  { xmit(CT_KEY); xmit('z'); }
	 xmit(by);
      }
   }
   else if( by == CT_KEY )
   {
      xmit(CT_KEY); xmit('c'); xmit(tolower(CT_KEY+'@'));
   }
   else
   {
      xmit(by);
      if( by == '\033' )
         xmit(by);
   }
}

/*
 * This key mapping is designed for easy use with terminfo nevertheless
 * it has potentially 96 function key + 104 alt/ctrl/shift&letter codes.
 *
#ifdef SCO_KEYMAP
 * This generates key sequences compatible with the SCO-Xenix console.
 * In addition many extra key combinations generate distinct codes.
 * But there are normally several ways of generating each standard key.
#endif
 */
void
write_unix(ch)
int ch;
{
   int by;
#ifdef SCO_KEYMAP
                    /*  012345678901234567890123456   789012 */
static char normal[] = " MNOPQRSTUVWX   LFBGDECHAI\177      ";
static char shift [] = " YZabcdefghij   0123456789\177      ";
static char ctrl  [] = " klmnopqrstuv             \177      ";
static char ctrshf[] = " wxyz@[\\]^_`{             \177      ";
#endif

   /* This seems to be a pain if it stays */
   if( ch == KEY_SPECIAL+KEY_SHIFT+'\r' ) ch = '\r';
   if( ch == KEY_SPECIAL+KEY_SHIFT+' '  ) ch = ' ';

#ifdef SCO_KEYMAP
   if( ch & KEY_SPECIAL )
   {
      if( ch & (KEY_ALT|KEY_ALTGR|KEY_DIRECT) )
         write_dosemu(ch);
      else if( (ch & 0xE0) != 0x80 )
         write_dosemu(ch);
      else
      {
         char * keys;
	 if( ch&KEY_SHIFT )
	 {
	    if( ch&KEY_CTRL ) keys = ctrshf; else keys = shift;
	 }
	 else
	 {
	    if( ch&KEY_CTRL ) keys = ctrl; else keys = normal;
	 }
         by = keys[ch&0x1F];
	 if( by == ' ' ) write_dosemu(ch);
	 else if( by == '\177' || by >= '0' && by <= '9' )
	    xmit(by);
         else
	 {
	    xmit('\033');
	    xmit('[');
	    xmit(by);
	 }
      }
   }
#else
   if( ch & KEY_SPECIAL )
      write_dosemu(ch);
#endif
   else
      xmit(ch&0xFF);
}

#else !AT_BIOS_KEY
/***************************************************************************
 * Get a key from the keyboard (-1 if none available)
 * Simple XT-BIOS version
 */

int get_key()
{
#ifdef KEY_COMPAT
   /* This is called so often that windows thinks we're idle; well we
    * are _but_ windows also seems to stop us.
    */

   if( _bios_keybrd(_KEYBRD_READY) == 0 ) return -1;
   return _bios_keybrd(_KEYBRD_READ);

#else

  /* So we have to hack it */

#define bios_ram   ((char far *)0x00400000L)
   static char last_flags = 0;
   int c;

   if( bios_ram[0x1C] != bios_ram[0x1A] )
      return _bios_keybrd(_KEYBRD_READ);
   else
      return -1;

#endif

}

/***************************************************************************
 * XT-BIOS Keymap tables for SCO console.
 */

#ifdef SCO_KEYMAP
#ifdef SCO_REMAP
char * keytab[95];
char   keybuf[33] = "";
int    keyxlate[KEYTABSIZE] =
                      { 0073, 0074, 0075, 0076, 0077,
                        0100, 0101, 0102, 0103, 0104, 0, 0, /* F11, F12 oops */
			0124, 0125, 0126, 0127, 0130,
			0131, 0132, 0133, 0134, 0135, 0, 0,
			0136, 0137, 0140, 0141, 0142,
			0143, 0144, 0145, 0146, 0147, 0, 0,
			0150, 0151, 0152, 0153, 0154,
			0155, 0156, 0157, 0160, 0161, 0, 0,
			0107, 0110, 0111, 0112, 0113,
			0114, 0115, 0116, 0117, 0120,
			0121, 0122
                      };
#endif

int kbtab[] =
{
/* 000 */   0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
/* 010 */   0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x25A,
/* 020 */   0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
/* 030 */   0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
/* 040 */   0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
/* 050 */   0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
/* 060 */   0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
/* 070 */   0x000, 0x000, 0x000, 0x24D, 0x24E, 0x24F, 0x250, 0x251,
/* 100 */   0x252, 0x253, 0x254, 0x255, 0x256, 0x000, 0x000, 0x248,
/* 110 */   0x241, 0x249, 0x000, 0x244, 0x000, 0x243, 0x000, 0x246,
/* 120 */   0x242, 0x247, 0x24C, 0x07F, 0x259, 0x25A, 0x261, 0x262,
/* 130 */   0x263, 0x264, 0x265, 0x266, 0x267, 0x268, 0x26B, 0x26C,
/* 140 */   0x26D, 0x26E, 0x26F, 0x270, 0x271, 0x272, 0x273, 0x274,
/* 150 */   0x277, 0x278, 0x279, 0x27A, 0x240, 0x25B, 0x25C, 0x25D,
/* 160 */   0x25E, 0x25F, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
};

bios_keymap(c)
int c;
{
   char * kptr;
   c >>= 8;

   if(c>=0 && c<0x78 && kbtab[c])
   {
      switch((kbtab[c] >> 8)&0xF)
      {
      case 1: xmit('\033');
	      break;
      case 2: xmit('\033');
	      xmit('[');
	      break;
#ifdef SCO_REMAP
      case 3: kptr = keytab[kbtab[c] & 0xFF];
	      if(kptr) while( *kptr ) xmit(*kptr++);
	      return TRUE;
#endif
      }
      xmit(kbtab[c] & 0xFF);
      return 1;
   }
   return 0;
}

/***************************************************************************
 * These functions are used for defining alternate function keys.
 */

#ifdef SCO_REMAP
add_fnchar(ch)
int ch;
{
   int s = strlen(keybuf);
   if( s == 32 ) return;
   keybuf[s] = ch;
   s++;
   keybuf[s] = 0;
}

set_key(kno)
int kno;
{
   if( keybuf[0] == 0 ) return message("Function key %d cleared", kno+1);
   if( keytab[kno] != 0 )
   {
      free(keytab[kno]);
      keytab[kno] = 0;
   }
   keytab[kno] = malloc(strlen(keybuf)+1);
   strcpy(keytab[kno], keybuf);
   keybuf[0] = 0;
}
#endif
#endif
#endif
