/* sierpin.t */

#define PPIFBUF 0xf010		/* address of ptr to IFBUF */
#define PSET 2				/* use this plot option */

struct IFBUF
{
	char ERR;
	char COMMAND;
	char COLOR;
	char PLOTOPTION;
	int LINESTYLE;
	int X1;
	int Y1;
	int X2;
	int Y2;
	char BFFLAG;
};

#define H0 512				/* square size */
#define XOFFS 80			/* offset to centre square */
#define XNUM 15				/* scale 512 * 15/16 = 480 */
#define XDENOM 16
#define YNUM 25				/* scale 512 * 25/64 = 200 */
#define YDENOM 64			/* to give max height, dot ratio 480/200 = 2.4 */

struct IFBUF *pifbuf;
int h, x, y;

main()
{
	struct IFBUF **ppifbuf;			/* required since no casts */
	int i, x0, y0;
	char color;

	pifbuf = *(ppifbuf = PPIFBUF);	/* pifbuf = *(struct IFBUF **)PPIFBUF; */
	pifbuf->PLOTOPTION = PSET;
	pifbuf->LINESTYLE = 			/* normal */
	pifbuf->COMMAND =				/* ?? */
	pifbuf->BFFLAG = 0;				/* not a box */
	color = i = 0;
	x0 = 2 * (h = H0/4);
	y0 = 3 * (H0/4);
	while ( h > 1 )
	{
		++i;
		if ( ++color > 7 )
			color = 1;
		pifbuf->COLOR = color;
		x = x0 -= h;
		y = y0 += h /= 2;
		glocate();
		a( i ); x += h; y -= h; plot();
		b( i ); x -= h; y -= h; plot();
		c( i ); x -= h; y += h; plot();
		d( i ); x += h; y += h; plot();
	}
}

a( i )
int i;
{
	if ( --i >= 0 )
	{
		a( i ); x += h; y -=h; plot();
		b( i ); x += 2*h; plot();
		d( i ); x += h; y += h; plot();
		a( i );
	}
}

b( i )
int i;
{
	if ( --i >= 0 )
	{
		b( i ); x -= h; y -=h; plot();
		c( i ); y -= 2*h; plot();
		a( i ); x += h; y -= h; plot();
		b( i );
	}
}

c( i )
int i;
{
	if ( --i >= 0 )
	{
		c( i ); x -= h; y +=h; plot();
		d( i ); x -= 2*h; plot();
		b( i ); x -= h; y -= h; plot();
		c( i );
	}
}

d( i )
int i;
{
	if ( --i >= 0 )
	{
		d( i ); x += h; y +=h; plot();
		a( i ); y += 2*h; plot();
		c( i ); x -= h; y += h; plot();
		d( i );
	}
}

glocate()
{
	pifbuf->X2 = x - x * (XDENOM - XNUM) / XDENOM + XOFFS;
	pifbuf->Y2 = (y * YNUM) / YDENOM;
}

plot()
{
	pifbuf->X1 = pifbuf->X2;
	pifbuf->Y1 = pifbuf->Y2;
	glocate();
#asm
	SWI2
	FDB	$4201		call LINEMA
#endasm
}
