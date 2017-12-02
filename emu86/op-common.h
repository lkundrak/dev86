// LIB86 - 80x86 library
// Common types and helpers

#pragma once


// Basic types

typedef unsigned char  byte_t;
typedef unsigned short word_t;
typedef unsigned long  addr_t;
typedef unsigned long  dword_t;


// Output helpers

void print_string (char * s);  // faster than formatted print
void print_column (char * s, byte_t w);

void print_rel (byte_t prefix, short rel);
