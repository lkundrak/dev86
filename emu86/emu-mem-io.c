
#include <string.h>
#include <assert.h>

#include "emu-mem-io.h"


static byte_t mem_stat [MEM_MAX];
static byte_t io_stat  [IO_MAX];


// Memory access

byte_t *mem_get_addr (addr_t a)
	{
	return (mem_stat + a);
	}


byte_t mem_read_byte (addr_t a)
	{
	assert (a < MEM_MAX);
	return mem_stat [a];
	}

word_t mem_read_word (addr_t a)
	{
	assert (a < MEM_MAX - 1);
	word_t * p = (word_t *) mem_get_addr (a);
	return *p;
	}

void mem_write_word (addr_t a, word_t val)
	{
	assert (a < MEM_MAX - 1);
	word_t * p = (word_t *) mem_get_addr (a);
	*p = val;
	}


void mem_io_reset ()
	{
	memset (mem_stat, 0xFF, sizeof mem_stat);
	memset (io_stat, 0xFF, sizeof io_stat);
	}
