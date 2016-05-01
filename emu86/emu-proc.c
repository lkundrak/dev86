// EMU86 - 80x86 emulation
// Processor emulation

#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "emu-proc.h"
#include "op-class.h"


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

byte_t reg8_get (byte_t reg)
	{
	assert (reg < REG8_MAX);
	assert (0);
	return 0;
	}

void reg8_set (byte_t reg, byte_t val)
	{
	assert (reg < REG8_MAX);
	assert (0);
	}

word_t reg16_get (byte_t reg)
	{
	assert (reg < REG16_MAX);
	return _proc_stat.regs [reg];
	}

void reg16_set (byte_t reg, word_t val)
	{
	assert (reg < REG16_MAX);
	_proc_stat.regs [reg] = val;
	}

word_t seg_get (byte_t seg)
	{
	assert (seg < SEG_MAX);
	return _proc_stat.segs [seg];
	}

void seg_set (byte_t seg, word_t val)
	{
	assert (seg < SEG_MAX);
	_proc_stat.segs [seg] = val;
	}


// Print registers

void regs_print ()
	{
	word_t ax = reg16_get (REG_AX);
	word_t bx = reg16_get (REG_BX);
	word_t cx = reg16_get (REG_CX);
	word_t dx = reg16_get (REG_DX);

	word_t si = reg16_get (REG_SI);
	word_t di = reg16_get (REG_DI);
	word_t bp = reg16_get (REG_BP);
	word_t sp = reg16_get (REG_SP);
	word_t ip = reg16_get (REG_IP);

	word_t cs = seg_get (SEG_CS);
	word_t ds = seg_get (SEG_DS);
	word_t es = seg_get (SEG_ES);
	word_t ss = seg_get (SEG_SS);

	printf ("AX %.4X  DS %.4X  SI %.4X\n",          ax, ds, si);
	printf ("BX %.4X  ES %.4X  DI %.4X\n",          bx, es, di);
	printf ("CX %.4X  SS %.4X  SP %.4X  BP %.4X\n", cx, ss, sp, bp);
	printf ("DX %.4X  CS %.4X  IP %.4X\n",          dx, cs, ip);
	}


// Fetch

byte_t fetch_cs_ip ()
	{
	word_t ip = reg16_get (REG_IP);
	byte_t b = mem_read_byte ((seg_get (SEG_CS) << 4) + ip);
	reg16_set (REG_IP, ++ip);
	return b;
	}


// Push & pop

void push_ss_sp (word_t val)
	{
	word_t sp = reg16_get (REG_SP) - 2;
	mem_write_word ((seg_get (SEG_SS) << 4) + sp, val);
	reg16_set (REG_SP, sp);
	}

word_t pop_ss_sp ()
	{
	word_t sp = reg16_get (REG_SP);
	word_t ss = seg_get (SEG_SS);
	word_t w = mem_read_word (ss << 4 + sp);
	reg16_set (REG_SP, sp + 2);
	return w;
	}


// Reset

void proc_reset ()
	{
	memset (&_proc_stat, 0, sizeof _proc_stat);
	seg_set (SEG_CS, 0xFFFF);
	}
