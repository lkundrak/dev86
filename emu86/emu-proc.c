
#include <string.h>
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
	word_t regs [REG_MAX];
	word_t segs [SEG_MAX];
	};

typedef struct proc_stat_s proc_stat_t;

static proc_stat_t proc_stat;


// Register access

word_t reg16_get (byte_t reg)
	{
	assert (reg < REG_MAX);
	return proc_stat.regs [reg];
	}

void reg16_set (byte_t reg, word_t val)
	{
	assert (reg < REG_MAX);
	proc_stat.regs [reg] = val;
	}

word_t seg_get (byte_t seg)
	{
	assert (seg < 0x4);
	return proc_stat.segs [seg];
	}

void seg_set (byte_t seg, word_t val)
	{
	assert (seg < 0x4);
	proc_stat.segs [seg] = val;
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
	word_t val = mem_read_word ((seg_get (SEG_SS) << 4) + sp);
	reg16_set (REG_SP, sp + 2);
	}


// Reset

void proc_reset ()
	{
	memset (&proc_stat, 0, sizeof proc_stat);
	seg_set (SEG_CS, 0xFFFF);
	}
