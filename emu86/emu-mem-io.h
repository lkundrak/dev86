
#pragma once

#include "op-common.h"


#define MEM_MAX 0x100000  // 1 MB
#define IO_MAX  0x10000   // 64 KB

#define ROM_BASE 0x80000


// Memory breakpoint

extern byte_t _break_data_flag;
extern addr_t _break_data_addr;


// Memory & IO operations

byte_t * mem_get_addr (addr_t a);

byte_t mem_read_byte (addr_t a);
word_t mem_read_word (addr_t a);

void mem_write_byte (addr_t a, byte_t b, byte_t init);
void mem_write_word (addr_t a, word_t w, byte_t init);


byte_t io_read_byte (word_t p);
word_t io_read_word (word_t p);

void io_write_byte (word_t p, byte_t b);
void io_write_word (word_t p, word_t w);


void mem_io_reset ();
