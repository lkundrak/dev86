
#include <stdio.h>
#include <dos.h>
#include <bios.h>
#include <signal.h>

#define KEY_SPECIAL 0x8000
#define KEY_PAD     0x4000
#define KEY_OTHER   0x2000
#define KEY_DIRECT  0x1000
#define KEY_ALT     0x0800
#define KEY_ALTGR   0x0400
#define KEY_CTRL    0x0200
#define KEY_SHIFT   0x0100

#ifndef OBJ_FILE

#define KEYBD       0x09
#define cprintf     printf

int get_key(void);
void ( interrupt far *orig_keybd)();
void interrupt far keybd();

int scrcol = 0;
int keymode = 1;

int bios_key = -1, bios_flags = -1;

#ifdef __BORLANDC__
static int kbd_type = -1;
#else
static int kbd_type = 0;
#endif

main(argc, argv)
int argc;
char ** argv;
{
   int ch, by;
   union REGS regs;
   short far * ptr = (short far * ) 0x0040001AL;
   int i;

   if( argc > 1 ) keymode = atoi(argv[1]);

   if( keymode & 4 ) { keymode &= -5; kbd_type = 0 ; }

   signal(SIGINT, SIG_IGN);

   cprintf("Keyboard testing: Press Escape to exit\r\n");
   orig_keybd = _dos_getvect(KEYBD);
   _dos_setvect(KEYBD, keybd);

   do
   {
      ch = get_key();

      if( (ch&0xFF00) == KEY_SPECIAL + KEY_DIRECT + KEY_OTHER )
      {
         if( bios_flags != -1 )
         {
	    cprintf(">\r\n");
            if( bios_key != -1 )
               cprintf( "BIOS %04x, FLGS: %04x\n", bios_key, bios_flags);
            else if( bios_flags != -1 )
               cprintf( "BIOS ----, FLGS: %04x\n", bios_flags);
	    scrcol=2;
	    cprintf(" <");
         }
	 if( scrcol > 76 ) { cprintf("\r\n"); scrcol = 0; }
         if( scrcol == 0 ) { cprintf("<"); scrcol += 1; }
	 write_scan(ch&0xFF);
      }
      else if( ch != -1 )
      {
	 cprintf(">\r\n");
         if( bios_key != -1 )
            cprintf( "BIOS %04x, FLGS: %04x: ", bios_key, bios_flags);
         else if( bios_flags != -1 )
            cprintf( "BIOS ----, FLGS: %04x: ", bios_flags);
         else
            cprintf( "BIOS ----, FLGS: ----: ");
         cprintf("Got key = 0x%04x is: ", ch);
         
	 if( ch & KEY_SPECIAL )
	 {
	    cprintf("Special-");
	    if( ch & KEY_CTRL )    cprintf("Ctrl-");
	    if( ch & KEY_SHIFT )   cprintf("Shift-");
	    if( ch & KEY_ALT )     cprintf("Alt-");
	    if( ch & KEY_ALTGR )   cprintf("AltGr-");
	    if( ch & KEY_DIRECT )  cprintf("Scan-");
	    by = (ch & 0xFF);
	 }
	 else 
	 {
	    cprintf("ASCII-");
	    by = (ch & 0xFFF);
	    if( ch & KEY_DIRECT )  cprintf("Code-");
	 }
	 if( ch & KEY_PAD )     cprintf("KeyPad-");
	 if( ch & KEY_OTHER )   cprintf("Other-");

         switch( ch & KEY_SPECIAL )
	 {
	 case KEY_SPECIAL:
            if( ch & KEY_DIRECT )
	       cprintf("0x%02x\r\n", by);
	    else if((by&0xF0)==0x80 )
	       cprintf("F%d\r\n", by&0xF);
	    else if((by&0xF0)==0x90 )
	       cprintf("K%d\r\n", by&0xF);
	    else
	    {
	 case 0:
	       if( by >= ' ' && by <= '~' )
	          cprintf("'%c'\r\n", by);
	       else
	          cprintf("0x%02x\r\n", by);
	    }
	 }

	 scrcol=5;
	 cprintf(": ");
         if( keymode ) write_dosemu(ch); else write_xenix(ch);
	 cprintf(" <");
      }
   }
   while( (ch &0x80FF) != '\033' );

   _dos_setvect(KEYBD, orig_keybd);

   while( get_key() != -1 ) ;

   if( scrcol ) { cprintf("\r\n"); scrcol = 0; }
}

void interrupt far keybd()
{
#ifdef __BORLANDC__
register int i;

   i = inp(0x60);

   _CX = 0xFE00 + (i&0xFF);
   _AH = 0x05; geninterrupt(0x16);

   if( keymode&2 )
   {
      _AH = 0x12; geninterrupt(0x16); i = _AX;
      _CX = 0xFF00 + (i&3) + ((i&0xF00)>>6) + ((i>>9)&0x40);
      _AH = 0x05; geninterrupt(0x16);
   }
#endif

   _chain_intr(orig_keybd);
}
#endif

xmit(ch)
{
   ch &= 0xFF;
   if( ch < ' ' )
   {
      cprintf("^%c", ch+'@');
      scrcol++;
   }
   else
      cprintf("%c", ch);
   scrcol++;
}

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

   int ch, flgs, scan, flgs2;
   int scand;
   int i;

   bios_key = bios_flags = -1;
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
      cprintf("Keyboard is %sT\n", kbd_type?"A":"X");
   }

   _AH = kbd_type+1; geninterrupt(0x16);
   i = (_FLAGS&0x40);

   _AH = kbd_type+2; geninterrupt(0x16); flgs = _AX;
   if( !kbd_type ) flgs &= 0xFF;

#else
   i = !_bios_keybrd(_KEYBRD_READY);

   flgs = _bios_keybrd(_KEYBRD_SHIFTSTATUS);
#endif

   if( keymode == 3 ) { flgs = 0; this_flags= -1; }

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
            bios_key = -1; bios_flags = flgs;
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

   if( ( ch & 0xFF00 ) == 0xFF00 ) { this_flags = ch; goto try_again; }
   if( ( ch & 0xFF00 ) == 0xFE00 ) 
      return KEY_SPECIAL + KEY_DIRECT + KEY_OTHER + (ch&0xFF);

   last_flgs = flgs;
   bios_key = ch; bios_flags = flgs;

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
write_scan(scan)
int scan;
{
static int lastscan = -1;
   int by;

   scan &= 0xFF;

   if( scan == lastscan+0x80 )
      { xmit('~'); xmit('\b'); }
   else if( scan == 0xE0 || scan == 0xE1 )
      xmit('E'+scan-0xE0);
   else if( by = scan_desc[scan&0x7F] )
   {
      if( scan & 0x80 )
	 { xmit('!'); xmit(by); }
      else
	 xmit(by);
   }
   else
      { xmit('~'); xmit(hextbl[scan>>4]); xmit(hextbl[scan&0xF]); }
   lastscan = scan;
}

/*
 * This keyboard gives a pattern of control sequences that makes just about
 * every keypress distinct. It does need a compatible host program but its
 * useable as a generic terminal type unlike the scancodes.
 */
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
 * This generates key sequences compatible with the SCO-Xenix console.
 * In addition many extra key combinations generate distinct codes.
 * But there are normally several ways of generating each standard key.
 *
 * This key mapping is designed for easy use with terminfo nevertheless
 * it has potentially 96 function key + 104 alt/ctrl/shift&letter codes.
 */
write_xenix(ch)
int ch;
{
   int by;
                    /*  012345678901234567890123456   789012 */
static char normal[] = " MNOPQRSTUVWX   LFBGDECHAI\177      ";
static char shift [] = " YZabcdefghij   0123456789\177      ";
static char ctrl  [] = " klmnopqrstuv             \177      ";
static char ctrshf[] = " wxyz@[\\]^_`{             \177      ";

   /* This seems to be a pain if it stays */
   if( ch == KEY_SPECIAL+KEY_SHIFT+'\r' ) ch = '\r';
   if( ch == KEY_SPECIAL+KEY_SHIFT+' '  ) ch = ' ';

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
   else
      xmit(ch&0xFF);
}
