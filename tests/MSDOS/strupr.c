/*
 * bcc libc-test: strupr
 *
 * This is a test-project for dev86/bcc to test the libc functions.
 *
 */

#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[]) {
    puts(strupr("abc"));

    return 0;
}


