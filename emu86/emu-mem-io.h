
#pragma once

#include "op-common.h"


#define MEM_MAX 0x100000  // 1 MB
#define IO_MAX  0x10000   // 64 KB


// Memory & IO operations

byte_t *mem_get_addr (addr_t a);
byte_t mem_read_byte (addr_t a);
word_t mem_read_word (addr_t a);

void mem_write_byte (addr_t a, byte_t val);
void mem_write_word (addr_t a, word_t val);

void mem_io_reset ();
