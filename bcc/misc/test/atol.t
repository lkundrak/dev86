/* atol.c - long atol( char *s ) */

/* atol  converts  s  to a long */
/* leading spaces and tabs are ignored, an optional sign is recognised, */
/* and the digits (0 to 9) following determine the long */

long atol( s )
register char *s;
{
	char signflag;
	long number;

	while ( *s == ' ' || *s == '\t')
		s++;
	signflag = 0;
	if ( *s == '+' )
		s++;
	else if ( *s == '-' )
	{
		signflag = 1;
		s++;
	}
	number = 0;
	while ( *s >= '0' && *s <= '9' )
		number = 10 * number + *s++ - '0';
	if ( signflag )
		return -number;
	return number;
}
