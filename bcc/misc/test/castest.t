/* castest.c */

/*
#define signedchar
#define bigints
*/
#define unsigchar
#define unsigshort
#define unsiglong

#define TEST(u,c) if ( (u) != (c) )\
	{ printf( "u != c\n" ); ++bad; } ++possible

main()
{
	int bad;
	int possible;
	char c;
#ifdef signedchar
	signed char sc;
#endif
#ifdef unsigchar
	unsigned char uc;
#endif
	short s;
#ifdef unsigshort
	unsigned short us;
#endif
	int i;
	unsigned u;
	long l;
#ifdef unsiglong
	unsigned long ul;
#endif

	bad = possible = 0;
	u = 0x1ff;
	c = (char) u;
	if ( c < 0 )
		printf( "characters are signed\n" );
	TEST((char)u,c);
	i = (int)(char) u;
	TEST(i,(int)c);
	TEST((int)(char)u,i);

#ifdef signedchar
	sc = (signed char) u;
	if ( sc >= 0 )
		printf( "??? signed char not signed\n" );
	TEST((signed char)u,sc);
	TEST((int)(signed char)u,(int)sc);
	i = (int) sc;
	TEST((int)(signed char)u,i);
#endif

#ifdef bigints
	u = 0x1ffff;
	s = (short) u;
	TEST((short)u,s);
	TEST((int)(short)u,(int)s);
	i = (int) s;
	TEST((int)(short)u,i);

#ifdef unsigshort
	us = (unsigned short) u;
	TEST((unsigned short)u,us);
	TEST((int)(unsigned short)u,(int)us);
	i = (int) us;
	TEST((int)(unsigned short)u,i);
#endif
#endif

	printf( "%d bad out of a possible %d\n", bad, possible );
}
