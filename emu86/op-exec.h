// EMU86 - 80x86 emulator
// Operation handling

#pragma once

#include "op-class.h"

// Operation handling

int exec_int (byte_t i);

int seg_none ();
void seg_reset ();

int rep_none ();
int rep_active ();
void rep_reset ();

int check_exec ();

int op_exec (op_desc_t * op_desc);
