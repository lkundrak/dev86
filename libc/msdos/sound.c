
#include <bios.h>

static int port_val = -1;

sound(freq)
unsigned freq;	/* freq is in hertz */
{
   if(port_val == -1 )
      port_val = inp(0x61);

   freq = 1193180L / freq;

   outp(0x61, port_val|3);
   outp(0x43, 0xb6);
   outp(0x42, freq&0xFF);
   outp(0x42, (freq>>8)&0xFF);
}

nosound()
{
   if( port_val )
      outp(0x61, port_val);
   else
      outp(0x61, inp(0x61)&~3);
}

