/*
 * getenv.c - a simple getenv() implementation for MS-DOS target of dev86/bcc
 *
 * 2018/10/05 - created - Nils Stec <stecdose@gmail.com>
 *
 */

#include <dos.h>
#include <string.h>

char *getenv(varname)
char *varname;
{
    int save_es;
    unsigned int pos, size;
    static char buffer[129];
    char c, *var, *content;

    save_es = __get_es();   /* we're paranoid, we save/restore what we use */
    __set_es(__envseg);

    memset(buffer, '\0', sizeof(buffer));

    pos = 0;
    size = 0;
    for(;;) {
        c = __peek_es(pos);
        buffer[size] = c;
        size++;
        if(c == '\0') {
            var = strtok(buffer, "=");
            content = strtok(NULL, "");
            if(strcmp(var, varname) == 0) {
                strcpy(buffer, content);
                __set_es(save_es);
                return buffer;
            }
            if(__peek_es(pos+1) == '\0') {    // end of env-string array
                __set_es(save_es);
                return NULL;
            }
            /* cleanup for next round */
            size = 0;
        }
        pos++;
    }
}
