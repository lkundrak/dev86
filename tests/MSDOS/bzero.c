/*
 * bcc libc-test: bzero
 *
 * This is a test-project for dev86/bcc to test the libc functions.
 *
 */

#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    char tmp[32];
    int i, j;

    memset(tmp, 0x55, 32);

    bzero(tmp, 32);

    j = 0;
    for(i = 0; i < 32; i++) {
         j += tmp[i];
    }

    if(j)
        printf("bzero failed!\r\n");
    else
        printf("bzero succeeded!\r\n");

    return 0;
}
