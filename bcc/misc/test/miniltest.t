long x;
int i;
unsigned u;

long test1(x)
long x;
{
	return x;
}

long test2(x)
int x;
{
	return x;
}

main()
{
	printlong( "x=1 is ", x = 1 );
	printlong( "test1(x) is ", test1(x) );
	printlong( "test1(1L) is ", test1(1L) );
	printlong( "x = test1(1L) is ", x = test1(1L) );
	printlong( "x = test1(x=1) is ", x = test1(x=1) );
	printlong( "i=1 is ", (long) (i = 1) );
	printlong( "test2(i) is ", test2(i) );
	printlong( "test2(1) is ", test2(1) );
	printlong( "i = test2(1) is ", (long) (i = test2(1)) );
	printlong( "i = test2(i=1) is ", (long) (i = test2(i=1)) );
	printlong( "(long) (i = -1) is ", (long) (i=-1) );
	printlong( "(long) (u = -1) is ", (long) (u=-1) );
	printlong( "x = -1 is ", x = -1 );
}

printlong( s, x )
char *s;
long x;
{
	printf( "%s", s );
	if ( x & 0x80000000 )
	{
		putchar( '-' );
		x = -x;
	}
	printf( "%08lx", (x >> 16) + (x << 16) );
	putchar( '\n' );
}

outulhex( pnum )
char *pnum;
{
	int i;

	for ( i = 3; i >=0; --i )
		p2( pnum[i] );
}

p2(i)
int i;
{
	p1(i>>4);
	p1(i);
}

p1(num)
int num;
{
	int digit;

	if ( (digit = num & 0xf) >= 10 )
		digit += 'A' - ('9' + 1);
	putchar( digit + '0' );
}
