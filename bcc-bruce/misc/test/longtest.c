int i,j;
long m,n;

main()
{
	while ( 1 )
	{
		if ( scanf( "%ld %ld", &m, &n ) <= 0 )
			exit( 0 );
		printf( "m = %ld n = %ld m + n = %ld m - n = %ld m * n = %ld\n",
				 m, n, m + n, m - n, m * n );
		printf( "m = %ld n = %ld m / n = %ld m %% n = %ld\n",
				 m, n, m / n, m % n );
		printf( "m = %ld n = %ld m&n = %ld m | n = %ld m^n = %ld\n",
				 m, n, m & n, m | n, m ^ n );
		printf( "m = %ld n = %ld m << n = %ld m >> n = %ld\n",
				 m, n, m << (int) n, m >> (int) n );
		printf(
		"m = %ld n = %ld m < n is %d m == n is %d m > n is %d m == 0 is %d\n",
				 m, n, m < n, m == n, m > n, m == 0 );
		printf( "m = %ld n = %ld -m = %ld ~m = %ld ++m = %ld --n = %ld\n",
				 m, n, -m, ~m, ++m, --n );
	}
}
