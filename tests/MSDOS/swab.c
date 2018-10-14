/*
 * bcc libc-test: swab
 *
 * This is a test-project for dev86/bcc to test the libc functions.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char src[32], dst[32];
    int n;

    n = 8;
    sprintf(src, "abcdefgh");
    swab(src, dst, n);
    printf("'%s' => '%s' (%u)\r\n", src, dst, n);


    n = 2;
    bzero(dst, strlen(dst));
    sprintf(src, "123456789");
    swab(src, dst, n);
    printf("'%s' => '%s' (%u)\r\n", src, dst, n);
    return 0;
}
