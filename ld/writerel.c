/*
 * This uses a special version of writebin for bug compatibility with
 * the old bin86 package.
 *
 * This _should_ be replaced by a function that writes out a as86 object
 * but then it would completely **** up dosemu compiles.
 *
 * NOTE: A some time I intend to replace this with a routine that generates
 *       an as86 object file.
 */

#ifdef BUGCOMPAT
#define A_OUT_INCL		"rel_aout.h"
#define BSD_A_OUT		1
#define FILEHEADERLENGTH	32
#define ELF_SYMS		0

#define FUNCNAME		write_rel
#undef NO_AOUT

#include "writebin.c"
#endif
