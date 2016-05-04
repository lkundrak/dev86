// DIS86 - 80x86 disassembler
// Operation common

#pragma once

// Basic types

typedef unsigned char  byte_t;
typedef unsigned short word_t;
typedef unsigned long  addr_t;


// Output helpers

void print_string (char * s);  // faster than formatted print
void print_column (char * s, byte_t w);
