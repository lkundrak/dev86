/* Is this BIOS keyboard io ? */

getch()
{
#asm
  xor   ax,ax
  int   $16
#endasm
}

getche()
{
   int i = getch();
   if( i & 0xFF) putch(i);
   return i;
}

kbhit()
{
#asm
  mov   ah,#1
  int   $16
  jz    nokey
  cmp   ax,#0
  jnz   dort
  mov   ax,#3
dort:
  ret
nokey:
  xor   ax,ax
#endasm
}

putch()
{
#asm
#if !__FIRST_ARG_IN_AX__
  mov   bx,sp
  mov   ax,[bx+2]
#endif
  mov   ah,#$0E
  mov   bx,#7
  int   $10
#endasm
}

cputs(str)
char * str;
{
   while(*str) putch(*str++);
}

cgets()
{
}

cprintf()
{
}

cscanf()
{
}

getpass()
{
}

gotoxy()
{
}


