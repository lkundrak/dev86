/* sieve using pointers */

#define TRUE 1
#define FALSE 0
#define NITER 100
#define SIZE 8191					/* last prime <= 2*this+3 */
#define SQRSIZE 63					/* last divisor tested = 2*this+3 */

char flags[SIZE+2*SQRSIZE+3];		/* avoid ptr+=prime overflowing */

main()
{
	int i,count,iter;
	register char *ptr;
	char *endptr;
	int prime;

	for (iter=0;iter<NITER;iter++)
	{
		count=0;
		ptr=flags;
		endptr=flags+SIZE;
		while (ptr<endptr)
			*ptr++=TRUE;
		for (i=0;i<SQRSIZE;i++)
		{
			if (flags[i])
			{
				prime=i+i+3;
				ptr=flags+i+prime;		/* ptr<endptr since i<SQRSIZE */
				while (ptr<endptr)
				{
					*ptr=FALSE;
					ptr+=prime;			/* does not overflow since in flags */
				}
				count++;
			}
		}
		ptr=flags+SQRSIZE;
		while (ptr<endptr)
			if (*ptr++)
				count++;
	}
	printf( "%d primes\n", count );
}
