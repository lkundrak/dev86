/*
 * bcc libc-test: strnset
 *
 * This is a test-project for dev86/bcc to test the libc functions.
 *
 */

#include <stdio.h>
#include <string.h>



int main(int argc, char *argv[]) {
    char str[81];
    int i;

    for(i = 0; i < 80; i++)
        str[i] = 'A';
    str[80] = '\0';

    strnset(str, '.', 3);

    puts(str);

    return 0;
}

