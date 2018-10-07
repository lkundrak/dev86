/* Copyright (C) 2018 Nils Stec <stecdose@gmail.com>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#include <string.h>

char *strlwr(str) 
char *str;
{
    char *ptr = str;

    while(*ptr) {
        *ptr = tolower(*ptr);
        ptr++;
    }

    return str;
}
