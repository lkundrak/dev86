
#include <sys/types.h>
#include <fcntl.h>

#if defined(__STDC__) && !defined(__FIRST_ARG_IN_AX__)
#include <stdarg.h>
#define va_strt      va_start
#else
#include <varargs.h>
#define va_strt(p,i) va_start(p)
#endif

#if defined(__STDC__) && !defined(__FIRST_ARG_IN_AX__)
int dbprintf(const char * fmt, ...)
#else
int dbprintf(fmt, va_alist)
__const char *fmt;
va_dcl
#endif
{
   va_list ptr;
   int rv;
   va_strt(ptr, fmt);
   rv = vdbprintf(fmt,ptr);
   va_end(ptr);
   return rv;
}

static unsigned char * __numout (long i, int base);
static void putch(int ch) { static char buf[2]; *buf = ch; write(2,buf,1); }

int
vdbprintf(fmt, ap)
register __const char *fmt;
register va_list ap;
{
   int c;
   int count = 0;
   int type, base;
   long val;
   char * cp;
   char padch=' ';
   int  minsize, maxsize;

   while(c=*fmt++)
   {
      count++;
      if(c!='%')
	 putch(c);
      else
      {
	 type=1;
	 padch = *fmt;
	 maxsize=minsize=0;
	 if(padch == '-') fmt++;

	 for(;;)
	 {
	    c=*fmt++;
	    if( c<'0' || c>'9' ) break;
	    minsize*=10; minsize+=c-'0';
	 }

	 if( c == '.' )
	    for(;;)
	    {
	       c=*fmt++;
	       if( c<'0' || c>'9' ) break;
	       maxsize*=10; maxsize+=c-'0';
	    }

	 if( padch == '-' ) minsize = -minsize;
	 else
	 if( padch != '0' ) padch=' ';

	 if( c == 0 ) break;
	 if(c=='h')
	 {
	    c=*fmt++;
	    type = 0;
	 }
	 else if(c=='l')
	 {
	    c=*fmt++;
	    type = 2;
	 }

	 switch(c)
	 {
	    case 'x': base=16; type |= 4;   if(0) {
	    case 'o': base= 8; type |= 4; } if(0) {
	    case 'u': base=10; type |= 4; } if(0) {
	    case 'd': base=-10; }
	       switch(type)
	       {
		  case 0: /* Promoted: val=va_arg(ap, short); break; */
		  case 1: val=va_arg(ap, int);   break;
		  case 2: val=va_arg(ap, long);  break;
		  case 4: /* Promoted: val=va_arg(ap, unsigned short); break; */
		  case 5: val=va_arg(ap, unsigned int);   break;
		  case 6: val=va_arg(ap, unsigned long);  break;
		  default:val=0; break;
	       }
	       cp = __numout(val,base);
	       if(0) {
	    case 's':
	          cp=va_arg(ap, char *);
	       }
	       count--;
	       c = strlen(cp);
	       if( !maxsize ) maxsize = c;
	       if( minsize > 0 )
	       {
		  minsize -= c;
		  while(minsize>0) { putch(padch); count++; minsize--; }
		  minsize=0;
	       }
	       if( minsize < 0 ) minsize= -minsize-c;
	       while(*cp && maxsize-->0 )
	       {
		  putch(*cp++);
		  count++;
	       }
	       while(minsize>0) { putch(' '); count++; minsize--; }
	       break;
	    case 'c':
	       putch(va_arg(ap, int));
	       break;
	    case 'C':
	       c = va_arg(ap, int);
	       if (c>0x7F) {
		  c &=0x7F;
		  putch('M'); putch('-');
	       }
	       if (c<' ' || c == '\177') {
		  putch('^'); putch(c^'@');
	       } else
		  putch(c);
	       break;
	    default:
	       putch(c);
	       break;
	 }
      }
   }
   return count;
}

static char nstring[]="0123456789ABCDEF";

#ifndef __AS386_16__
#define NUMLTH 11

static unsigned char *
__numout(long i, int base)
{
   static unsigned char out[NUMLTH+1];
   int n;
   int flg = 0;
   unsigned long val;

   if (base<0)
   {
      base = -base;
      if (i<0)
      {
	 flg = 1;
	 i = -i;
      }
   }
   val = i;

   out[NUMLTH] = '\0';
   n = NUMLTH-1;
   do
   {
      out[n--] = nstring[val % base];
      val /= base;
   }
   while(val);
   if(flg) out[n--] = '-';
   return &out[n+1];
}

#else

#asm
! numout.s
!
.bss
___out	lcomm	$C

.text
___numout:
push	bp
mov	bp,sp
push	di
push	si
add	sp,*-4
mov	byte ptr -8[bp],*$0	! flg = 0
mov	si,4[bp]	; i or val.lo
mov	di,6[bp]	; i or val.hi
mov	cx,8[bp]	; base
test	cx,cx			! base < 0 ?
jge 	.3num
neg  cx				! base = -base
or	di,di			! i < 0 ?
jns	.5num
mov	byte ptr -8[bp],*1	! flg = 1
neg	di			! i = -i
neg	si
sbb	di,0
.5num:
.3num:
mov	byte ptr [___out+$B],*$0	! out[11] = nul
mov	-6[bp],*$A		! n = 10

.9num:
!!!         out[n--] = nstring[val % base];
xor  dx,dx
xchg ax,di
div  cx
xchg ax,di
xchg ax,si
div  cx
xchg ax,si			! val(new) = val / base

mov  bx,dx			! dx = val % base

mov	al,_nstring[bx]
mov	bx,-6[bp]
dec	word ptr -6[bp]
mov	___out[bx],al

mov  ax,si
or   ax,di			! while (val)
jne	.9num

cmp	byte ptr -8[bp],*$0	! flg == 0 ?
je  	.Dnum

mov	bx,-6[bp]
dec	word ptr -6[bp]
mov	byte ptr ___out[bx],*$2D	! out[n--] = minus

.Dnum:
mov	ax,-6[bp]
add	ax,#___out+1

add	sp,*4
pop	si
pop	di
pop	bp
ret
#endasm

#endif
