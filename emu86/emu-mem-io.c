
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "emu-mem-io.h"


static byte_t mem_stat [MEM_MAX];
static byte_t io_stat  [IO_MAX];


// Memory access

byte_t * mem_get_addr (addr_t a)
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


void mem_write_byte (addr_t a, byte_t b)
	{
	assert (a < MEM_MAX);
	mem_stat [a] = b;
	}

void mem_write_word (addr_t a, word_t w)
	{
	assert (a < MEM_MAX - 1);
	word_t * p = (word_t *) mem_get_addr (a);
	*p = w;
	}


// Memory dump

void print_mem (addr_t begin, addr_t end)
	{
	assert (begin <= end);

	addr_t b = begin & 0xFFFF0;  // align on 16 bytes
	addr_t o = begin & 0x0000F;
	addr_t a = 0;

	byte_t c;
	char s [17];

	puts ("       0  1  2  3  4  5  6  7   8  9  A  B  C  D  E  F");

	if (o)
		{
		printf ("%.5lX ", b);
		for (a = 0; a < o; a++) print_string ("   ");
		}

	for (a = begin; a < end; a++)
		{
		o = a & 0x0000F;
		if (!o)
			{
			b = a & 0xFFFF0;
			printf ("%.5lX ", b);

			memset (s, '.', 16);
			s [16] = 0;
			}

		if (o == 8) putchar (' ');

		c = mem_read_byte (a);
		printf (" %.2X", c);
		if (c >= 32 && c < 127) s [o] = (char) c;  // 127 = not printable DEL

		if (o == 15)
			{
			print_string ("  ");
			print_string (s);
			putchar ('\n');
			}
		}

	// TODO: complete line and print string

	if (o) putchar ('\n');
	}


// I/O access

byte_t io_read_byte (word_t p)
	{
	return 0xFF;
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
