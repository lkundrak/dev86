// EMU86 - 80x86 emulation
// Processor emulation

#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "op-class.h"

#include "emu-mem-io.h"
#include "emu-proc.h"


struct proc_regs_s
	{
	word_t ax;
	word_t cx;
	word_t dx;
	word_t bx;
	word_t sp;
	word_t bp;
	word_t si;
	word_t di;
	word_t ip;
	word_t fl;
	};

typedef struct proc_regs_s proc_regs_t;

struct proc_segs_s
	{
	word_t es;
	word_t cs;
	word_t ss;
	word_t ds;
	};

typedef struct proc_segs_s proc_segs_t;

struct proc_stat_s
	{
	word_t regs [REG16_MAX];
	word_t segs [SEG_MAX];
	};

typedef struct proc_stat_s proc_stat_t;

static proc_stat_t _proc_stat;


// Register access

byte_t * reg8_addr (byte_t r8)
	{
	assert (r8 < REG8_MAX);
	byte_t r16 = (r8 & 3);
	byte_t * p = (byte_t *) &_proc_stat.regs [r16] +  (r8 >> 2);
	return p;
	}

byte_t reg8_get (byte_t r8)
	{
	return *(reg8_addr (r8));
	}

void reg8_set (byte_t r8, byte_t b)
	{
	*(reg8_addr (r8)) = b;
	}


word_t reg16_get (byte_t r16)
	{
	assert (r16 < REG16_MAX);
	return _proc_stat.regs [r16];
	}

void reg16_set (byte_t r16, word_t w)
	{
	assert (r16 < REG16_MAX);
	_proc_stat.regs [r16] = w;
	}


word_t seg_get (byte_t s)
	{
	assert (s < SEG_MAX);
	return _proc_stat.segs [s];
	}

void seg_set (byte_t s, word_t w)
	{
	assert (s < SEG_MAX);
	_proc_stat.segs [s] = w;
	}


// Flags access

byte_t flag_get (byte_t flag)
	{
	assert (flag < FLAG_MAX);
	word_t r = reg16_get (REG_FL);
	return ((r >> flag) & 1);
	}

void flag_set (byte_t flag, byte_t val)
	{
	assert (flag < FLAG_MAX);
	assert (val < 2);

	word_t m = ~(1 << flag);
	word_t r = reg16_get (REG_FL);
	r = (r & m) | (val << flag);
	reg16_set (REG_FL, r);
	}


// Print registers
// TODO: move to DBG86 part

void regs_print ()
	{
	// Get registers in CPU order

	word_t ax = reg16_get (REG_AX);
	word_t cx = reg16_get (REG_CX);
	word_t dx = reg16_get (REG_DX);
	word_t bx = reg16_get (REG_BX);

	word_t sp = reg16_get (REG_SP);
	word_t bp = reg16_get (REG_BP);
	word_t si = reg16_get (REG_SI);
	word_t di = reg16_get (REG_DI);

	word_t es = seg_get (SEG_ES);
	word_t cs = seg_get (SEG_CS);
	word_t ss = seg_get (SEG_SS);
	word_t ds = seg_get (SEG_DS);

	word_t ip = reg16_get (REG_IP);
	word_t fl = reg16_get (REG_FL);

	printf ("AX %.4hX  BX %.4hX  CX %.4hX  DX %.4hX  FL %.4hX\n", ax, bx, cx, dx, fl);
	printf ("SI %.4hX  DI %.4hX  IP %.4hX  SP %.4hX  BP %.4hX\n", si, di, ip, sp, bp);
	printf ("DS %.4hX  ES %.4hX  CS %.4hX  SS %.4hX\n", ds, es, cs, ss);

	// TODO: invert flag order

	printf ("\nCF %hhu  PF %hhu  AF %hhu  ZF %hhu  SF %hhu  TF %hhu  IF %hhu  DF %hhu  OF %hhu\n",
		flag_get (FLAG_CF), flag_get (FLAG_PF), flag_get (FLAG_AF),
		flag_get (FLAG_ZF), flag_get (FLAG_SF), flag_get (FLAG_TF),
		flag_get (FLAG_IF), flag_get (FLAG_DF), flag_get (FLAG_OF));

	}


// Get address from segment:offset

addr_t addr_seg_off (word_t seg, word_t off)
	{
	addr_t a = ((addr_t) seg) << 4;
	a += (addr_t) off;
	return a;
	}


// Print memory

void mem_print (word_t seg, word_t begin, word_t end)
	{
	assert (begin <= end);

	word_t b = begin & 0xFFFF0;  // align on 16 bytes
	word_t o = 0;
	word_t a = 0;

	byte_t c;
	char s [17];

	printf ("%.4X:  0  1  2  3  4  5  6  7   8  9  A  B  C  D  E  F\n", seg);

	while (1)
		{
		// Row header

		if (o == 0x0)
			{
			printf (" %.4X ", b);
			}

		// Row body

		if (o == 0x8) putchar (' ');

		a = b + o;
		if (a < begin || a > end)
			{
			print_string ("   ");
			s [o] = ' ';
			}
		else
			{
			c = mem_read_byte (addr_seg_off (seg, a));
			printf (" %.2X", c);

			s [o] = (c >= 32 && c < 127) ? c : '.';  // 127 = not printable DEL
			}

		// Row trailer

		o++;

		if (o == 0x10)
			{
			o = 0;

			print_string ("  ");
			s [16] = 0;
			print_string (s);
			putchar ('\n');

			if (a >= end) break;

			b += 0x10;
			}
		}
	}


// Stack operations

void stack_push (word_t val)
	{
	word_t ss = seg_get (SEG_SS);
	word_t sp = reg16_get (REG_SP) - 2;
	mem_write_word (addr_seg_off (ss, sp), val, 0);
	reg16_set (REG_SP, sp);
	}

word_t stack_pop ()
	{
	word_t ss = seg_get (SEG_SS);
	word_t sp = reg16_get (REG_SP);
	word_t w = mem_read_word (addr_seg_off (ss, sp));
	reg16_set (REG_SP, sp + 2);
	return w;
	}


void stack_print ()
	{
	word_t cs = seg_get (SEG_CS);
	word_t ss = seg_get (SEG_SS);

	word_t ip = reg16_get (REG_IP);
	word_t sp = reg16_get (REG_SP);
	word_t bp = reg16_get (REG_BP);

	word_t d = 0;  // depth
	addr_t a;

	while (d < 10)
		{
		// Local frame

		if (sp > bp) break;  // no frame

		if (d > 0) putchar ('\n');
		printf ("[%u] %.4X:%.4X\n", d, cs, ip);
		mem_print (ss, sp, bp + 1);

		// Next frame
		// TODO: not only for NEAR call

		sp = bp;

		a = addr_seg_off (ss, bp);
		bp = mem_read_word (a + 0);
		ip = mem_read_word (a + 2);

		if (sp > 0xFFFC) break;
		sp = sp + 4;

		d++;
		}
	}

// Reset processor context

void proc_reset ()
	{
	memset (&_proc_stat, 0, sizeof _proc_stat);
	seg_set (SEG_CS, 0xFFFF);
	}
