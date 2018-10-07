/* Copyright (C) 2018 Nils Stec <stecdose@gmail.com>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <string.h>

char *strnset(str, c, n)
char *str;
char c;
unsigned int n;
{
    if(n > strlen(str))      /* some safety */
        n = strlen(str);

    while(*str && n) {
        *str = c;
        str++;
        n--;
    }
    return str;
}
