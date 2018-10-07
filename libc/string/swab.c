/* Copyright (C) 2018 Nils Stec <stecdose@gmail.com>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <string.h>

void swab(src, dst, n)
const void *src;
void *dst;
unsigned int n;
{
    char *pdst, *psrc;

    if(n & 1)  // n is odd?
        n--;

    pdst = dst;
    psrc = src;

    while(n > 0) {
        pdst[0] = psrc[1];
        pdst[1] = psrc[0];
        pdst += 2;
        psrc += 2;
        n -= 2;
    }

    return;
}

