/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

static char buf[12];

extern char * ultoa();

char * ltoa(val)
long val;
{
   char *p;
   int flg = 0;
   if( val < 0 ) { flg++; val= -val; }
   p = ultoa(val);
   if(flg) *--p = '-';
   return p;
}

char * ultoa(val)
unsigned long val;
{
   char *p;

   p = buf+sizeof(buf);
   *--p = '\0';

   do
   {
      *--p = '0' + val%10;
      val/=10;
   }
   while(val);
   return p;
}

