/* cfsetspeed.c - emulate BSD cfsetspeed with cfset[io]speed - rick sladkey */

#include <termios.h>

struct {
	int flag, val;
} xref[] = {
	B0,	0,
	B50,	50,
	B75,	75,
	B110,	110,
	B134,	134,
	B150,	150,
	B200,	200,
	B300,	300,
	B600,	600,
	B1200,	1200,
	B1800,	1800,
	B2400,	2400,
	B4800,	4800,
	B9600,	9600,
	B19200,	19200,
	B38400,	38400,
#ifdef B57600
	B57600, 57600,
#endif
#ifdef B115200
	B115200,115200,
#endif
	0,	-1,
};

int cfsetspeed(p,speed)
struct termios *p;
int speed;
{
	int i;

	for (i = 0; xref[i].val != -1; i++) {
		if (xref[i].val == speed) {
			speed = xref[i].flag;
			cfsetispeed(p, speed);
			cfsetospeed(p, speed);
			return 0;
		}
	}
	return -1;
}

