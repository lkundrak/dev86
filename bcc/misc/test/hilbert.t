/* hilbert.c */

/* S1 stuff

#define PIFBUF ( *(struct IFBUF **) 0xf010 )
#define PSET 2

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
end of S1 stuff */

/* L3 stuff */

#define PIFBUF ( (struct IFBUF *) 0xa1 )
#define PSET 1

struct IFBUF
{
	char PLOTOPTION;
	char junk1[0xa8-0xa2];
	int X1;
	int Y1;
	int X2;
	int Y2;
	char junk2[0xf0-0xb0];
	char ERR;				/* this to BFFLAG are dummies to keep S1 code */
	char COMMAND;
	int LINESTYLE;
	char BFFLAG;
	char junk3[0x3ea-0xf5];
	char COLOR;
};

#define H0 512				/* square size */
#define XOFFS 80			/* offset to centre square */
#define XNUM 15				/* scale 512 * 15/16 = 480 */
#define XDENOM 16
#define YNUM 25				/* scale 512 * 25/64 = 200 */
#define YDENOM 64			/* to give max height, dot ratio 480/200 = 2.4 */

int h, x, y;

main()
{
	int i, x0, y0;
	char color;

	PIFBUF->PLOTOPTION = PSET;
	PIFBUF->LINESTYLE = 			/* normal */
	PIFBUF->COMMAND =				/* ?? */
	PIFBUF->BFFLAG = 0;				/* not a box */
	color = i = 0;
	x0 = y0 = (h = H0)/2;
	while ( h > 1 )
	{
		++i;
		h = h/2;
		if ( ++color > 7 )
			color = 1;
		gcolor( color );
		x = x0 += h/2;
		y = y0 += h/2;
		glocate();
		a( i );
	}
}

a( i )
int i;
{
	if ( --i >= 0 )
	{
		d( i ); x -= h; plot();
		a( i ); y -= h; plot();
		a( i ); x += h; plot();
		b( i );
	}
}

b( i )
int i;
{
	if ( --i >= 0 )
	{
		c( i ); y += h; plot();
		b( i ); x += h; plot();
		b( i ); y -= h; plot();
		a( i );
	}
}

c( i )
int i;
{
	if ( --i >= 0 )
	{
		b( i ); x += h; plot();
		c( i ); y += h; plot();
		c( i ); x -= h; plot();
		d( i );
	}
}

d( i )
int i;
{
	if ( --i >= 0 )
	{
		a( i ); y -= h; plot();
		d( i ); x -= h; plot();
		d( i ); y += h; plot();
		c( i );
	}
}

glocate()
{
	PIFBUF->X2 = x - x * (XDENOM - XNUM) / XDENOM + XOFFS;
	PIFBUF->Y2 = (y * YNUM) / YDENOM;
}

/* S1 gcolor and plot

gcolor( color )
int color;
{
	PIFBUF->COLOR = color;
}

plot()
{
	PIFBUF->X1 = PIFBUF->X2;
	PIFBUF->Y1 = PIFBUF->Y2;
	glocate();
#asm
	SWI2
	FDB	$4201		call LINEMA
#endasm
}

end S1 plot */

gcolor( color )
int color;
{
	PIFBUF->COLOR = color | 0x10;
}

plot()
{
	PIFBUF->X1 = PIFBUF->X2;
	PIFBUF->Y1 = PIFBUF->Y2;
	glocate();
#asm
	JSR	$D709
	JSR	$D79A
#endasm
}
