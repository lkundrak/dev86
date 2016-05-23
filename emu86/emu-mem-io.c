
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "emu-mem-io.h"


static byte_t mem_stat [MEM_MAX];
static byte_t io_stat  [IO_MAX];


// Memory access

byte_t * mem_get_addr (addr_t a)
	{
	assert (a < MEM_MAX);
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


void mem_write_byte (addr_t a, byte_t b)
	{
	assert (a < MEM_MAX);

	// Protect ROM

	if (a < ROM_BASE)
		{
		mem_stat [a] = b;
		}
	else
		{
		printf ("warning: writing byte into ROM @ %lxh\n", a);
		}
	}

void mem_write_word (addr_t a, word_t w)
	{
	assert (a < MEM_MAX - 1);

	// Protect ROM

	if (a < ROM_BASE - 1)
		{
		word_t * p = (word_t *) mem_get_addr (a);
		*p = w;
		}
	else
		{
		printf ("warning: writing word into ROM @ %lxh\n", a);
		}
	}


// I/O access

byte_t io_read_byte (word_t p)
	{
	byte_t b = 0xFF;

	// ADVTECH stubbing

	switch (p)
		{
		case 0x0065:
			b = 0x00;   // needed to exit loop @ F000:11B7h
			break;

		}

	return b;
	}


word_t io_read_word (word_t p)
	{
	return 0xFFFF;
	}


void io_write_byte (word_t p, byte_t b)
	{
	}


 void io_write_word (word_t p, word_t w)
	{
	}


// Reset

void mem_io_reset ()
	{
	memset (mem_stat, 0xFF, sizeof mem_stat);
	memset (io_stat, 0xFF, sizeof io_stat);
	}
