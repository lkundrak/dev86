/* puzzle.t - from C puzzle book */

#define puzzle(which) ( puts( "-----which-----\n" ), which() )

main()
{
	puzzle( op1 );
	puzzle( op2 );
	puzzle( op3 );
	puzzle( op4 );
	puzzle( op5 );
	puzzle( op6 );
	puzzle( prep1 );
	puzzle( prep2 );
}

op1()
{
	int x;

	x = - 3 + 4 * 5 - 6; printf("%d\n",x);
	x = 3 + 4 % 5 - 6; printf("%d\n",x);
	x = - 3 * 4 % - 6 / 5; printf("%d\n",x);
	x = ( 7 + 6 ) % 5 /2; printf("%d\n",x);
}

#define PRINTX printf( "%d\n", x )

op2()
{
	int x, y, z;

	x = 2;
	x *= 3 + 2; PRINTX;
	x *= y = z = 4; PRINTX;
	x = y == z; PRINTX;
	x == (y == z); PRINTX;
}

#define PRINT(int) printf( "int = %d\n", int )

op3()
{
	int x, y, z;

	x = 2; y = 1; z = 0;
	x = x && y || z; PRINT(x);
	PRINT( x || ! y && z );

	x = y = 1;
	z = x ++ - 1; PRINT(x); PRINT(z);
	z += - x ++ + ++ y;PRINT(x);PRINT(z);
	z = x / ++ x; PRINT(z);
}

op4()
{
	int x, y, z;

	x = 03; y = 02; z = 01;
	PRINT( x | y & z );
	PRINT( x | y & ~ z );
	PRINT( x ^ y & ~ z );
	PRINT( x & y && z );

	x = 1; y = -1;
	PRINT( ! x | x );
	PRINT( ~ x | x );
	PRINT( x ^ x );
	x <<= 3; PRINT(x);
	y <<= 3; PRINT(y);
	y >>= 3; PRINT(y);
}

op5()
{
	int x, y, z;

	x = 1; y = 1; z = 1;

	x += y += z;
	PRINT( x < y ? y : x );

	PRINT( x < y ? x ++ : y ++ );
	PRINT(x); PRINT(y);

	PRINT( z += x < y ? x ++ : y ++ );
	PRINT(y); PRINT(z);

	x = 3; y=z=4;
	PRINT( (z >= y >= x) ? 1 : 0 );
	PRINT( z >= y && y >= x );
}

#define PRINT3(x,y,z) printf("x=%d\ty=%d\tz=%d\n",x,y,z)

op6()
{
	int x, y, z;

	x = y = z = 1;
	++x || ++y && ++z; PRINT3(x,y,z);

	x = y = z = 1;
	++x && ++y || ++z; PRINT3(x,y,z);

	x = y = z = 1;
	++x && ++y && ++z; PRINT3(x,y,z);

	x = y = z = -1;
	++x && ++y || ++z; PRINT3(x,y,z);

	x = y = z = -1;
	++x || ++y && ++z; PRINT3(x,y,z);

	x = y = z = -1;
	++x && ++y && ++z; PRINT3(x,y,z);
}

#define FUDGE(k) k+3
#define PR(a) printf("a= %d\t",(int)(a))
#define PRINTNEW(a) PR(a); putchar( '\n' );
#define PRINT2(a,b) PR(a); PRINT(b)
#define PRINT3NEW(a,b,c) PR(a); PRINT2(b,c)
#define MAX(a,b) (a<b ? b : a )

prep1()
{
	{
		int x;
		x = 2;
		PRINTNEW( x*FUDGE(2) );
	}
	{
		int cel;
		for( cel=0; cel<=100; cel+=50 )
			PRINT2( cel, 9/5*cel+32 );
	}
	{
		int x, y;
		x=1; y=2;
		PRINT3NEW( MAX(x++,y),x,y );
		PRINT3NEW( MAX(x++,y),x,y );
	}
}

#define NEG(a)-a
#define weeks(mins) (days(mins)/7)
#define days(mins) (hours(mins)/24)
#define hours(mins) (mins/60)
#define mins(secs) (secs/60)
#define TAB(c,i,oi,tx) if(c=='\t')\
						for(tx=8-(i-oi-1)%8,oi=i; tx; tx--)\
							putchar(' ')

static char *input = "\twhich\tif?";

prep2()
{
	{
		int x;
		x=1;
		PRINT( -NEG(x) );
	}
	{
		PRINT( weeks(10080) );
		PRINT( days(mins(86400)) );
	}
	{
		char c;
		int i, oldi, temp;

		for( oldi= -1,i=0; (c=input[i])!='\0'; i++ )
			if( c<' ' ) TAB(c,i,oldi,temp);
			else putchar(c);
			putchar('\n');
	}
}
