/* DOS MOTD 
 * 
 *  (c) 2018 <stecdose@gmail.com>
 *  license: GPLv2
 */

#include <stdio.h>
#include <time.h>

#define uint16_t    unsigned int
#define uint8_t     unsigned char
#define int8_t      signed char
#define int16_t     signed int

int main(int argc, char *argv[]) {
    int r;

    srand(time(NULL));

    for(;;) {
        r = rand();
        printf("%04x\r\n", r);
    }
}
