
/*
 * Display a page from help.txt, the argument is the scan code of a function
 * key. F1..F10 display pages 1..10, HOME is page zero, PGUP and PGDN are
 * previous and next page.
 */

#include "monitor.h"

#ifndef NOCOMMAND

struct keys {
   int key;
   int rel;
   int abs;
} keys[] = {
   {0xC7, 0, 0},	/* HOME page 0*/
   {0xBB, 0, 1},	/* F1   page 1 */
   {0xBC, 0, 2},	/* F2   page 2 */
   {0xBD, 0, 3},	/* F3   page 3 */
   {0xBE, 0, 4},	/* F4   page 4 */
   {0xBF, 0, 5},	/* F5   page 5 */
   {0xC0, 0, 6},	/* F6   page 6 */
   {0xC1, 0, 7},	/* F7   page 7 */
   {0xC2, 0, 8},	/* F8   page 8 */
   {0xC3, 0, 9},	/* F9   page 9 */
   {0xC4, 0, 10},	/* F10  page 10 */

   {0xC9, -1,0},	/* PGUP page-- */
   {0xD1,  1,0},	/* PGDN page++ */

   {0,0,1}
};

cmd_help(ptr)
char * ptr;
{
   int helpkey = 1;

   getnum(&ptr, &helpkey);

   return help_key(helpkey);
}

help_key(helpkey)
int helpkey;
{ 
static int lastpage = 0;
   int i;

   for(i=0; keys[i].key; i++)
      if( keys[i].key == helpkey || i == helpkey )
         break;

   if( keys[i].key == 0 )
   {
      printf("Unbound key, press F1 for general help\n");
      return -1;
   }
   
   if( keys[i].rel ) lastpage += keys[i].rel;
   else	             lastpage = keys[i].abs;

   if( lastpage < 0 ) { lastpage=0; return 0; }

   return display_help(lastpage);
}

display_help(page)
int page;
{
   char buffer[1024];
   long length= -1;
   int left = 0;
   int ch,lastch = '\n';
   int flg = 0;

   if( open_file("help.txt") < 0 )
   {
      if( page == 1 )
         printf("Help file 'help.txt' is not available, sorry.\n");
      return -1;
   }

   for(length = file_length(); length>0; length--)
   {
      if( left==0 )
      {
         if( read_block(buffer) < 0 ) break;
	 left = 1024;
      }
      ch = buffer[1024-left]; left--;
      if( ch == '%' && lastch == '\n' ) { flg = 1; page--; }
      if( page < 0 ) break;
      if( page == 0 && flg == 0 ) putchar(ch);
      if( ch == '\n' ) flg = 0;
      lastch = ch;
   }
   return 0;
}

#endif
