// EMU86 - 80x86 emulator
// Operation handling

#pragma once

#include "op-class.h"


// Operation handling

int rep_stat ();
void seg_reset ();

int op_exec (op_desc_t * op_desc);
