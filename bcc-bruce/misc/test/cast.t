/* cast.t */

char c;
unsigned char uc;
short s;
unsigned short us;
int i;
unsigned int ui;
long l;
unsigned long ul;
float f;
double d;
int *p;

cast()
{
	c = (char) c;
	uc = (unsigned char) c;
	s = (short) c;
	us = (unsigned short) c;
	i = (int) c;
	ui = (unsigned int) c;
	l = (long) c;
	ul = (unsigned long) c;
	f = (float) c;
	d = (double) c;
	p = (int *) c;

	c = (char) uc;
	uc = (unsigned char) uc;
	s = (short) uc;
	us = (unsigned short) uc;
	i = (int) uc;
	ui = (unsigned int) uc;
	l = (long) uc;
	ul = (unsigned long) uc;
	f = (float) uc;
	d = (double) uc;
	p = (int *) uc;

	c = (char) s;
	uc = (unsigned char) s;
	s = (short) s;
	us = (unsigned short) s;
	i = (int) s;
	ui = (unsigned int) s;
	l = (long) s;
	ul = (unsigned long) s;
	f = (float) s;
	d = (double) s;
	p = (int *) s;

	c = (char) us;
	uc = (unsigned char) us;
	s = (short) us;
	us = (unsigned short) us;
	i = (int) us;
	ui = (unsigned int) us;
	l = (long) us;
	ul = (unsigned long) us;
	f = (float) us;
	d = (double) us;
	p = (int *) us;

	c = (char) i;
	uc = (unsigned char) i;
	s = (short) i;
	us = (unsigned short) i;
	i = (int) i;
	ui = (unsigned int) i;
	l = (long) i;
	ul = (unsigned long) i;
	f = (float) i;
	d = (double) i;
	p = (int *) i;

	c = (char) ui;
	uc = (unsigned char) ui;
	s = (short) ui;
	us = (unsigned short) ui;
	i = (int) ui;
	ui = (unsigned int) ui;
	l = (long) ui;
	ul = (unsigned long) ui;
	f = (float) ui;
	d = (double) ui;
	p = (int *) ui;

	c = (char) l;
	uc = (unsigned char) l;
	s = (short) l;
	us = (unsigned short) l;
	i = (int) l;
	ui = (unsigned int) l;
	l = (long) l;
	ul = (unsigned long) l;
	f = (float) l;
	d = (double) l;
	p = (int *) l;

	c = (char) ul;
	uc = (unsigned char) ul;
	s = (short) ul;
	us = (unsigned short) ul;
	i = (int) ul;
	ui = (unsigned int) ul;
	l = (long) ul;
	ul = (unsigned long) ul;
	f = (float) ul;
	d = (double) ul;
	p = (int *) ul;

	c = (char) f;
	uc = (unsigned char) f;
	s = (short) f;
	us = (unsigned short) f;
	i = (int) f;
	ui = (unsigned int) f;
	l = (long) f;
	ul = (unsigned long) f;
	f = (float) f;
	d = (double) f;

	c = (char) d;
	uc = (unsigned char) d;
	s = (short) d;
	us = (unsigned short) d;
	i = (int) d;
	ui = (unsigned int) d;
	l = (long) d;
	ul = (unsigned long) d;
	f = (float) d;
	d = (double) d;

	c = (char) p;
	uc = (unsigned char) p;
	s = (short) p;
	us = (unsigned short) p;
	i = (int) p;
	ui = (unsigned int) p;
	l = (long) p;
	ul = (unsigned long) p;
	p = (int *) p;
}
