/* bt.t - bug tests */

test1()
{
	char *p, *q;

	q += *p++;
}

test2()
{
	int i,j,k,a[2],b[2],c[2];

	i = a[i] + b[j] + c[k];
}

test3()
{
	int *p, *q;
	int i;

	i = p - q;
}

test4()
{
	int a[2];
	int *p;

	p = a;
	test4(a);
}

test5()
{
	char c;

	test4(c);
}

test6(a,b)
int b,a;
{
	int i;

	i = b;
	i = a;
}

test7()
{
	int a[2];
	int *p;

	p = a + 1;
}

int *test8();

int *test8()
{
	int i;

	if ( test8() == i )
		;
	i = *test8();
}

test9()
{
	int i,j,k,n;

	if ((i < 0 || j > 100) && (k < 0 || n > 50))
		i = 1;
}

struct complex
{
	int re;
	int im;
}
	znum, *zptr, zarr[10], *zptrarr[10];

test10()
{
	int i;

	znum.re = i;
	znum.im = 1;
	zptr -> re = 1;
	zptr -> im = i;
	zarr[3].im = i;
	zptrarr[4] -> im = 1;
}

test11()
{
	char symtab[100];

	char *p;

	if (p > symtab)
		;
}

test12()
{
char symtab[100];

	char *p;

	p = symtab - 21;
}

test13()
{
	char a[10];
	int i;

	i = a - 1;
}

test14()
{
	int i, **pi;

	**pi = i;
}

test15()
{
	int i, j;

	if ( (i = j ) == 2 )
		test();
}

test16()
{
	struct cx3
	{
		int re3;
		char im3;
	}
		z[10], *pz, *pz1;
	int i;

	i = z[i].re3;
	i = pz1 - pz;
}

test17()
{
	int i;
	char c;

	c &= ~i;
}

test18()
{
	typedef int (*PFI)();
	PFI h();

	(*h())( 0 );
}

test19()
{
	register int *p, *q;
	int ***z,i,a[10];

	test(***z + a[i]);
}

char a[2][3][5];

char *p = a;
char *q = a[1];
char *r = a[1][2];

char test20();
char (*test21)();

test22()
{
	char i,k;
	char *p;

	p = a;
	p = a[1];
	p = a[1][2];
	p = a[k];
	p = a[k][k];
	i = sizeof a;
	i = sizeof a[k];
	i = sizeof a[k][k];
	i = sizeof test20;
	i = sizeof test21;
	i = sizeof test20();
	i = sizeof (*test21)();
}

test23()
{
	long *pl;
	int i;
	char *s;

	*pl += i;
	*s += *pl;
}

test24()
{
	float f;
	double *dp1(), *dp2();

	f = *dp1()++;			/* illegal */
	f = *++dp1();			/* illegal */
	f = *dp1() + *dp2();	/* bad code (pushes ptr to doubly indirect) */
}

test25()
{
	test25( "2"[1] );		/* added 1 to label number, not label address */
}

struct stat
{
	int st_dev;
};

test26( buf )
struct stat buf;			/* wrong declare, s.b. struct stat *buf */
{
	buf->st_dev = 1;		/* error caused null pointer to be dereferenced */
}

union
{
	long l;
}
	test27;

test28()
{
	test27.l = test27.l + 1;	/* produced poor code */
}

int test29 = (char) 1;			/* cast was clobbering the global symptr */

struct { int i; } test30;

test31()
{
	return test30;				/* a structure cannot be returned (yet) */
}

int *test32, test33() {}		/* this is illegal */

test35()
{
	char a[1];
	char b[1];
	int i;

	i = i == 1 ? a : b;			/* arrays were not converted to ptrs */
}

test36()
{
	struct fp
	{
		struct filp *fp_filp[1];
	};
	struct filp
	{
		int filp_int;
		long filp_long;
	};
	int i;
	register char *junk;
	register struct fp *cp;

	cp->fp_filp[i]->filp_int++;		/* used to push lvalue */
	cp->fp_filp[i]->filp_long++;	/* used to push lvalue */
	cp->fp_filp[i]->filp_int += 1;	/* was OK */
	cp->fp_filp[i]->filp_long += 1;	/* used to load long into DREG:DREG */
}

test37()
{
	unsigned u;

	u = -u;							/* used to give botched nodetype */
}

test38()
{
	char c;
	int i;

	i = -c;							/* did i = (char) -c for a time */
}

test39()
{
	int i;

	i = (char) 1000;				/* the constant wasn't being truncated */
}

#define test40(x) (x)

test41()
{
	int i;
	int j;
	int k;

	i = test40( j +				/* was confused by EOL in macro argument */
				k );
}

test42()
{
	register char *junk1;
	register char *junk2;
	long *longptr;
	long longfn();

	*longptr = longfn();		/* used to run out of index regs */
}

test43()
{
	register char *jnk1, *junk2;
	unsigned char *bp;
	struct FILE_BUF
	{
		char *bufp;
	} *op;

	*op->bufp++ = *bp;		/* used to push lvalue */
}

test44()
{
  register char *x, *y;
  int i;
  char a[2];
  char **p;

  a[**p] = **p;				/* used to push lvalue */
}

struct test45
{
  int i;
  int j;
} test45[10];

test46()
{
  register char *p, *q;
  int i;
  int j;

  test45[i] = test45[j];		/* used to push lvalue */
}

int (*test100( x )() {}			/* this is legal, keep last avoid swallowing */
