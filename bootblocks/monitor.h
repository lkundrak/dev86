
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
#define NO_COLOUR

extern union REGS __argr;
#endif

#ifdef TARFLOPPY
#define SINGLEFS

#define open_file   tar_open_file
#define rewind_file tar_rewind_file
#define close_file  tar_close_file
#define file_length tar_file_length
#define read_block  tar_read_block
#endif

#ifdef MINFLOPPY
#define SINGLEFS

#define open_file   min_open_file
#define rewind_file min_rewind_file
#define close_file  min_close_file
#define file_length min_file_length
#define read_block  min_read_block
#endif

#ifdef DOSFLOPPY
#define SINGLEFS

#define open_file   dos_open_file
#define rewind_file dos_rewind_file
#define close_file  dos_close_file
#define file_length dos_file_length
#define read_block  dos_read_block
#endif

#ifdef SINGLEFS
/* #define NOCOMMAND */
#define NOMONITOR
#endif

#ifdef __STANDALONE__
#undef  putchar
#define putchar   putch
#define printf    cprintf
#define fflush(x)
#endif

