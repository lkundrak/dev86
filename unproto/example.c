 /*
  * @(#) example.c 1.2 91/09/22 21:21:45
  * 
  * Examples of things that can be done with the unproto package
  */

 /*
  * New-style argument list with structured argument, one field being pointer
  * to function returning pointer to function with function-pointer argument
  */

x(struct {
    struct {
	int (*(*foo) (int (*arg1) (double))) (float arg2);
    } foo;
} baz) {
    return (0);
}

 /*
  * Old-style argument list with new-style argument type, declaration
  * embedded within block. Plus a couple assignments with function calls that
  * look like casts.
  */

foo(bar)
int     (*(*bar) (float)) (int);
{
    int     (*baz) (int) = (int (*) (int)) 0,
	    y = (y * (*baz) (y)),
	    *(*z) (int) = (int *(*) (int)) 0;

    struct { int (*foo)(int); } *(*s)(int) = 
	(struct { int (*foo)(int); } *(*)(int)) 0;

    {
	y = (y * (*baz) (y));
    }
    {
	z = (int *(*) (int)) 0;
    }
    {
	s = (struct { int (*foo)(int); } *(*)(int)) 0;
    }

    return (0);
}

/* Multiple declarations in one statement */

test1()
{
	int foo2,*(*(*bar)(int))(float),*baz(double);
}

/* Discriminate declarations from executable statements */

test2(char *y)
{
	int foo = 5,atoi(char *);

	foo = 5,atoi(y);
}

/* Declarations without explicit type */

test3,test4(int);

test5(int y)
{
	{
		test3;
	}
	{
		test4(y);
	}
}

test6[1],test7(int);

test7(int x)
{
	{
		test6[1];
	}
	{
		test7(x);
	}
}

/* Checking a complicated cast */

struct {
    struct {
	int (*f)(int), o;
    } bar;
} (*baz2)(int) = (struct { struct { int (*f)(int), o; } bar; } (*)(int)) 0;

/* Distinguish things with the same shape but with different meaning */

test8(x)
{
    {
	struct {
	    int     foo;
	} bar(int);
    }
    {
	do {
	    int     foo;
	} while (x);
    }
}

/* Do not think foo(*bar) is a function pointer declaration */

test9(char *bar)
{
    foo(*bar);
}

/* another couple of special-cased words. */

test10(int x)
{
    {
	int test10(int);
	do  test10(x);
	while (x);
    }
    {
	return test10(x);
    }
}

test11(int *x)
{
	while (*x)
	    (putchar(*x++));
}
