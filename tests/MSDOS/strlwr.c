/*
 * bcc libc-test: strlwr
 *
 * This is a test-project for dev86/bcc to test the libc functions.
 *
 */
#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[]) {
    puts(strlwr("ABC"));

    return 0;
}


