/* opcmp.t - test logical compare operations */

/* only doing chars, shorts, ints and unsigneds */
/* not doing longs, floats or doubles */

/* short = int, so only a few examples */
/* unsigned like int, so only a few examples */
/* all examples < */

op1()
{
	char c, *pc, **ppc, cfn();
	short s, *ps, **pps, sfn();
	int i, *pi, **ppi, ifn();
	unsigned u, *pu, **ppu, ufn();

	c < 0;
	0 < c;
	i < 0;
	0 < i;

	s < 0;
	0 < s;

	0 < u;
	u < 0;
	1 < u;
	u < 1;

	c < 1;
	1 < c;
	c < 500;
	500 < c;

	c < c;
	c < i;
	i < c;
	i < i;

	s < c;
	i < u;

	c < c++;
	c < i++;
	i < c++;
	i < i++;

	s < i++;
	c < u++;

	c++ < c;
	i++ < c;
	c++ < i;
	i++ < i;

	s++ < c;
	i++ < u;

	c < cfn();
	c < ifn();
	i < cfn();
	i < ifn();

	s < cfn();
	c < ufn();

	cfn() < c;
	ifn() < c;
	cfn() < i;
	ifn() < i;

	sfn() < c;
	ifn() < u;

	*pc < c++;
	*pc < i++;
	*pi < c++;
	*pi < i++;

	*ps < c++;
	*pi < u++;

	c++ < *pc;
	i++ < *pc;
	c++ < *pi;
	i++ < *pi;

	s++ < *pc;
	i++ < *pu;

	**ppc < c++;
	**ppc < i++;
	**ppi < c++;
	**ppi < i++;

	**pps < c++;
	**ppi < u++;

	c++ < **ppc;
	i++ < **ppc;
	c++ < **ppi;
	i++ < **ppi;

	s++ < **ppc;
	i++ < **ppu;
}
