
/*
 * Under Linux 8086 the stack and heap areas are at the top and bottom
 * of the same area of memory, this version of malloc requires that the
 * malloc area is of a fixed size this means that there must also be a
 * specific amount of stack space reserved outside of this. The number
 * of bytes to be reserved is specified below.
 */

int __STACKSIZE = 2048;
