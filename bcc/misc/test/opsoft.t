/* opsoft.t - test software operations */

/* only doing chars, shorts, ints and unsigneds */
/* not doing longs, floats or doubles */

/* short = int, so only a few examples */
/* unsigned like int, so only a few examples */
/* most examples for DIVOP */
/* only (all) special cases of MULOP, MODOP, SLOP and SROP are tried */

op1()
{
	char c, *pc, **ppc, cfn();
	short s, *ps, **pps, sfn();
	int i, *pi, **ppi, ifn();
	unsigned u, *pu, **ppu, ufn();

	i = c / 0;
	i = 0 / c;
	i = i / 0;
	i = 0 / i;

	i = s / 0;
	i = 0 / u;

	i = c / 1;
	i = 1 / c;
	i = i / 1;
	i = 1 / i;

	i = s / 1;
	i = 1 / u;

	i = c / 2;
	i = 2 / c;
	i = i / 2;
	i = 2 / i;

	i = s / 2;
	i = 2 / u;

	i = c / 500;
	i = 500 / c;
	i = i / 500;
	i = 500 / i;

	i = s / 500;
	i = 500 / u;

	i = c / c;
	i = c / i;
	i = i / c;
	i = i / i;

	i = s / c;
	i = i / u;

	i = c / c++;
	i = c / i++;
	i = i / c++;
	i = i / i++;

	i = s / i++;
	i = c / u++;

	i = c++ / c;
	i = i++ / c;
	i = c++ / i;
	i = i++ / i;

	i = s++ / c;
	i = i++ / u;

	i = c / cfn();
	i = c / ifn();
	i = i / cfn();
	i = i / ifn();

	i = s / cfn();
	i = c / ufn();

	i = cfn() / c;
	i = ifn() / c;
	i = cfn() / i;
	i = ifn() / i;

	i = sfn() / c;
	i = ifn() / u;

	i = *pc / c++;
	i = *pc / i++;
	i = *pi / c++;
	i = *pi / i++;

	i = *ps / c++;
	i = *pi / u++;

	i = c++ / *pc;
	i = i++ / *pc;
	i = c++ / *pi;
	i = i++ / *pi;

	i = s++ / *pc;
	i = i++ / *pu;

	i = **ppc / c++;
	i = **ppc / i++;
	i = **ppi / c++;
	i = **ppi / i++;

	i = **pps / c++;
	i = **ppi / u++;

	i = c++ / **ppc;
	i = i++ / **ppc;
	i = c++ / **ppi;
	i = i++ / **ppi;

	i = s++ / **ppc;
	i = i++ / **ppu;

	i = c * 0;
	i = 0 * c;
	i = i * 0;
	i = 0 * i;

	i = s * 0;
	i = 0 * u;

	i = c * 1;
	i = 1 * c;
	i = i * 1;
	i = 1 * i;

	i = s * 1;
	i = 1 * u;

	i = c * 2;
	i = 2 * c;
	i = i * 2;
	i = 2 * i;

	i = s * 2;
	i = 2 * u;

	i = c * 500;
	i = 500 * c;
	i = i * 500;
	i = 500 * i;

	i = s * 500;
	i = 500 * u;

	i = c * c;
	i = c * c++;
	i = c++ * c;
	i = c * cfn();
	i = cfn() * c;
	i = *pc * c++;
	i = c++ * *pc;
	i = **ppc * c++;
	i = c++ * **ppc;

	i = c % 0;
	i = 0 % c;
	i = i % 0;
	i = 0 % i;

	i = s % 0;
	i = 0 % u;

	i = c % 1;
	i = 1 % c;
	i = i % 1;
	i = 1 % i;

	i = s % 1;
	i = 1 % u;

	i = c % 2;
	i = 2 % c;
	i = i % 2;
	i = 2 % i;

	i = s % 2;
	i = 2 % u;

	i = c % 500;
	i = 500 % c;
	i = i % 500;
	i = 500 % i;

	i = s % 500;
	i = 500 % u;

	i = c << 0;
	i = 0 << c;
	i = i << 0;
	i = 0 << i;

	i = s << 0;
	i = 0 << u;

	i = c << 1;
	i = 1 << c;
	i = i << 1;
	i = 1 << i;

	i = s << 1;
	i = 1 << u;

	i = c << 8;
	i = 8 << c;
	i = i << 8;
	i = 8 << i;

	i = s << 8;
	i = 8 << u;

	i = c << 9;
	i = 9 << c;
	i = i << 9;
	i = 9 << i;

	i = s << 9;
	i = 9 << u;

	i = c >> 0;
	i = 0 >> c;
	i = i >> 0;
	i = 0 >> i;

	i = s >> 0;
	i = 0 >> u;

	i = c >> 1;
	i = 1 >> c;
	i = i >> 1;
	i = 1 >> i;

	i = s >> 1;
	i = 1 >> u;

	i = c >> 8;
	i = 8 >> c;
	i = i >> 8;
	i = 8 >> i;

	i = s >> 8;
	i = 8 >> u;

	i = c >> 9;
	i = 9 >> c;
	i = i >> 9;
	i = 9 >> i;

	i = s >> 9;
	i = 9 >> u;
}
