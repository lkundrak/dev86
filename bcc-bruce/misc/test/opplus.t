/* op+-.t - test plus, minus operators */

/* only doing chars and ints, not shorts, longs, floats, doubles */
/* not doing pointer arithmetic */

op1()
{
	char cfn();
	int ifn();
	char c, *pc, **ppc;
	int i, *pi, **ppi;
	
	i = c + 1;
	i = 1 + c;
	i = i + 1;
	i = 1 + i;

	i = c + 500;
	i = 500 + c;
	i = i + 500;
	i = 500 + i;

	i = c - 1;
	i = 1 - c;
	i = i - 1;
	i = 1 - i;

	i = c - 500;
	i = 500 - c;
	i = i - 500;
	i = 500 - i;

	i = c + c;
	i = c + i;
	i = i + c;	/* -2 cycles, +1 byte different from c + i, lhs loaded 1st */
	i = i + i;

	i = c - c;
	i = c - i;
	i = i - c;
	i = i - i;

	i = c + c++;
	i = c + i++;
	i = i + c++;
	i = i + i++;

	i = c++ + c;
	i = i++ + c;
	i = c++ + i;
	i = i++ + i;

	i = c - c++;
	i = c - i++;
	i = i - c++;
	i = i - i++;

	i = c++ - c;
	i = i++ - c;
	i = c++ - i;
	i = i++ - i;

	i = c + cfn();
	i = c + ifn();
	i = i + cfn();
	i = i + ifn();

	i = cfn() + c;
	i = ifn() + c;
	i = cfn() + i;
	i = ifn() + i;

	i = c - cfn();
	i = c - ifn();
	i = i - cfn();
	i = i - ifn();

	i = cfn() - c;
	i = ifn() - c;
	i = cfn() - i;
	i = ifn() - i;

	i = *pc + c++;
	i = *pc + i++;
	i = *pi + c++;
	i = *pi + i++;

	i = c++ + *pc;
	i = i++ + *pc;
	i = c++ + *pi;
	i = i++ + *pi;

	i = *pc - c++;
	i = *pc - i++;
	i = *pi - c++;
	i = *pi - i++;

	i = c++ - *pc;
	i = i++ - *pc;
	i = c++ - *pi;
	i = i++ - *pi;

	i = **ppc + c++;
	i = **ppc + i++;
	i = **ppi + c++;
	i = **ppi + i++;

	i = c++ + **ppc;
	i = i++ + **ppc;
	i = c++ + **ppi;
	i = i++ + **ppi;

	i = **ppc - c++;
	i = **ppc - i++;
	i = **ppi - c++;
	i = **ppi - i++;

	i = c++ - **ppc;
	i = i++ - **ppc;
	i = c++ - **ppi;
	i = i++ - **ppi;
}
