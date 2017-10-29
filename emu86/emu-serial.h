
#pragma once

#include "op-common.h"


void serial_send (byte_t c);
byte_t serial_recv ();

byte_t serial_poll ();

void serial_init ();
void serial_term ();
