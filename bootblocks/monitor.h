
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <bios.h>
#include <malloc.h>
#include "i86_funcs.h"
#include "readfs.h"

#define X_TARFLOPPY
#define X_DOSFLOPPY
#define X_CALC_CRC

#ifdef __STANDALONE__
#define COLOUR

extern union REGS __argr;
#endif

#ifdef TARFLOPPY
#define SINGLEFS
#define NOMONITOR
#define NOCOMMAND
#define MINI_BUF

#define open_file   tar_open_file
#define rewind_file tar_rewind_file
#define close_file  tar_close_file
#define file_length tar_file_length
#define read_block  tar_read_block
#endif

#ifdef DOSFLOPPY
#define SINGLEFS
#define NOMONITOR
#define NOCOMMAND
#define MINI_BUF
#define MAXTRK 24

#define open_file   dos_open_file
#define rewind_file dos_rewind_file
#define close_file  dos_close_file
#define file_length dos_file_length
#define read_block  dos_read_block
#endif

#if 1
#undef  putchar
#define putchar   cputchar
#define printf    cprintf
#define fflush(x)
#endif

