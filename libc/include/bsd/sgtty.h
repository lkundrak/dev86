/* sgtty.h */

/* This file contains defintions to help make linux termios look like
   bsd sgtty. */

/* Copyright (c) 1992 Ross Biro

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef _BSD_SGTTY_H
#define _BSD_SGTTY_H

#include <termios.h>

#define TIOCGETP	0x5481
#define TIOCSETP	0x5482
#define TIOCGETC	0x5483
#define TIOCSETC	0x5484
#define TIOCGLTC	0x5485
#define TIOCSLTC	0x5486
#define TIOCLGET	0x5487
#define TIOCLSET	0x5488
#define TIOCFLUSH	0x5489
#define TIOCSETN	0x548a

struct sgttyb
{
    unsigned short sg_flags;
    char sg_ispeed;
    char sg_ospeed;
    char sg_erase;
    char sg_kill;
    struct termios t;
    int check;
};

struct tchars
{
    char t_intrc;
    char t_quitc;
    char t_eofc;
    char t_startc;
    char t_stopc;
    char t_brkc;
};

struct ltchars
{
    char t_werasc;
    char t_suspc;
    char t_dsuspc;
    char t_rprntc;
    char t_flushc;
    char t_lnextc;
};

#define O_RAW 1
#define O_LCASE 2
#define O_CRMOD 4
#define O_ECHO 8
#define O_ODDP 16
#define O_EVENP 32
#define O_CBREAK 64
/* these don't do anything yet. */
#define O_TBDELAY 0
#define O_LLITOUT 0
#define O_XTABS 0
#define O_ANYP 0

#ifndef _SGTTY_SOURCE

#undef ioctl
#define ioctl	bsd_ioctl

#undef RAW
#define RAW O_RAW
#undef LCASE
#define LCASE O_LCASE
#undef ECHO
#define ECHO O_ECHO
#undef CRMOD
#define CRMOD O_CRMOD
#undef ODDP
#define ODDP O_ODDP
#undef EVENP
#define EVENP O_EVENP
#undef CBREAK
#define CBREAK O_CBREAK
#undef TBDELAY
#define TBDELAY O_TBDELAY
#undef LLITOUT
#define LLITOUT O_LLITOUT
#undef XTABS
#define XTABS O_XTABS
#undef ANYP
#define ANYP O_ANYP
#endif

#endif /* _BSD_SGTTY_H */
