#define TRUE 1
#define FALSE 0
#define SIZE 8190

int flags[SIZE+1];

main()
{
	int i,prime,k,count,iter;

	for (iter=0;iter<10;iter++)
	{
		count=0;
		for (i=0;i<=SIZE;i++)
			flags[i]=TRUE;
		for (i=0;i<=SIZE;i++)
		{
			if (flags[i])
			{
				prime=i+i+3;
				for (k=i+prime;k<=SIZE;k=k+prime)
					flags[k]=FALSE;
				count++;
			}
		}
	}
	printf( "%d primes\n", count );
}
