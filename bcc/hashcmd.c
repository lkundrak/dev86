
/* decode remaining preprocessor lines, minimal version. */

/* Copyright (C) GPL V2, derived from preproc.c by RDB. */

#include "bcc.h"
#include "input.h"
#include "os.h"
#include "output.h"
#include "parse.h"
#include "sc.h"
#include "scan.h"
#include "table.h"
#include "type.h"

#ifndef BUILTIN_CPP
/* docontrol() - process control statement, #line and #asm only. */

PUBLIC void docontrol()
{
   bugerror("docontrol for tiny machines not implemented yet");
}
#endif
