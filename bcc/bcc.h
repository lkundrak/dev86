/* Copyright (C) 1997 Robert de Bath <robert@mayday.cix.co.uk>
 * This file is part of the Linux-8086 Development environment and is
 * distributed under the GNU General Public License. */

#ifdef _AIX
#include <sys/types.h>	/* AIX 4.1 + GCC seems to need this */
#endif

/* Ansi C has certain guarentees ... except under MSdross :-( */

#ifdef __STDC__
#ifndef MSDOS
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#endif

#define P(x)	x

#else
#define P(x)	()
#endif

#include "const.h"
#include "types.h"
#include "proto.h"

#if !defined(__STDC__) || defined(MSDOS)
#include "sysproto.h"
#endif

