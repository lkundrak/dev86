unsigned t[10000];
unsigned s[10000];
unsigned size,repet;
unsigned comp=0,swap=0;		/* s.b. long */

main(argc)
{
	int i,zz;

/*
	printf("size?");
	scanf("%d",&size);
	printf("repet?");
	scanf("%d",&repet);
*/
	if ( argc > 20 )
	{
		printf( "usage: sort [args], where 500*argc is the array size\n" );
		exit( 1 );
	}
	size = 500 * argc;
	repet = 1;
	for (i = 0; i < size; i++)
		s[i] = size-i;
	printf("\npress key to begin shell sorting\n");
	getchar();
	for (zz=0;zz<repet;zz++)
	{
		comp=swap=0;
		for (i=0;i<size;i++)
			t[i]=s[i];
		sort();
	}
	printf("\nsorted\n");
	printf("\ncompares = %d, swaps = %d\n",comp,swap);
	printf("\npress key to begin quick sorting\n");
	getchar();
	for (zz=0;zz<repet;zz++)
	{
		comp=swap=0;
		for (i=0;i<size;i++)
			t[i]=s[i];
		qqsort();
	}
	printf("\nsorted\n");
	printf("\ncompares = %d, swaps = %d\n",comp,swap);
}

sort()
{
	int i,j,h,temp;

	h=1;
	while (9*h+4<size)
		h=3*h+1;
	while (h>0)
	{
		for (j=h; j<size; j++)
			for (i=j-h; i>=0; i-=h)
			{
				++comp;
				if (t[i]<=t[i+h])
					break;
				++swap;
				temp=t[i];
				t[i]=t[i+h];
				t[i+h]=temp;
			}
		h=(h-1)/3;
	}
}

qqsort()
{
	qsort(0,size-1);	
}

qsort(l,r)
int l,r;
{
	int i,j;
	unsigned x,w;

	i=l;j=r;
	x=t[(l+r)>>1];
	do
	{
		while (t[i] < x)
		{
			++comp;
			i++;
		}
		while (x < t[j]) {--j;}
		if (i<=j)
		{
			++swap;
			w=t[i];t[i]=t[j];t[j]=w;
			i++;j--;
		}
	}
	while (i<=j);
	if (l<j) qsort(l,j);
	if (i<r) qsort(i,r);
}
