/* Copyright (C) 2018 Nils Stec <stecdose@gmail.com>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <string.h>

char *strrev(string)
char *string;
{
    static char *dst;
    char *p;
    unsigned int i, len;

    len = strlen(string);

    dst = (char *)malloc(len);

    for(i = 0; i < len; i++) {
        dst[i] = string[len-i-1];
    }
    dst[i] = '\0';

    return dst;
}
