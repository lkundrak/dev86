/*
 * Since 1.3.99 vm86.h has moved from it's original position, this file
 * redirects sys/vm86.h to the correct location if it hasn't been updated
 * yet.
 *
 * Use "-idirafter . " on the command line.
 */
#include <asm/vm86.h>
