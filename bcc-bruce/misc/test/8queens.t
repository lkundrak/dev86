/* 8queens - straightforward translation of pascal version */

#define TRUE 1
#define FALSE 0

int i;
char a[8],b[15],c[15],x[8];

main()
{
    for (i=0;i<8;i++)
		a[i]=TRUE;
    for (i=0;i<15;i++)
    	b[i]=TRUE;
    for (i=0;i<15;i++)
    	c[i]=TRUE;
    try(0);
}

print()
{
	int k;

    for	(k=0;k<8;k++)
		write(x[k]);
    writeln();
}

write(i)
int i;
{
	{
#asm
	LDA	#$20
	JSR	$E820
	LDD	.i,S
	JSR	$C682
#endasm
	}
}

writeln()
{
	{
#asm
	LDA	#13
	JSR	$E820
	LDA	#10
	JSR	$E820
#endasm
	}
}

try(i)
int i;
{
	int j;

    for(j=0;j<8;j++)
		if (a[j] && b[i+j] && c[7+i-j])
    {
		x[i]=j;
		a[j]=FALSE;
		b[i+j]=FALSE;
		c[7+i-j]=FALSE;
		if (i<7)
			try(i+1);
		else
			print();
        a[j]=TRUE;
		b[i+j]=TRUE;
		c[7+i-j]=TRUE;
    }
}
