
#define disable() _disable()
#define enable()  _enable()

_enable()
{
#asm
  sti
#endasm
}

_disable()
{
#asm
  cli
#endasm
}

geninterrupt(intr)
int intr;
{
}

inp(portno)
int portno;
{
}

inpw(portno)
int portno;
{
}

outp(...)
{
}

outpw(...)
{
}

peek(segment, offset)
unsigned segment, offset;
{
}

peekb(segment, offset)
unsigned segment, offset;
{
}

poke(segment, offset, value)
unsigned segment, offset, value;
{
}

pokeb(segment, offset, value)
unsigned segment, offset, value;
{
}
