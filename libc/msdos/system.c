/*
 * system.c - a simple system() implementation for MS-DOS target of dev86/bcc
 *
 * 2018/10/05 - created - Nils Stec <stecdose@gmail.com>
 *
 */

#include <string.h>
#include <dos.h>

int system(string)
char *string;
{
    struct SREGS sregs;
    union REGS inregs, outregs;

    struct exec_params {
        unsigned int env_seg;
        unsigned int cmd_off;
        unsigned int cmd_seg;
        unsigned int fcb1_off;
        unsigned int fcb1_seg;
        unsigned int fcb2_off;
        unsigned int fcb2_seg;
    } param_block;

    char cmd[129];
    char args[129];
    char *p;

    memset(cmd, '\0', sizeof(cmd));
    memset(args, '\0', sizeof(args));

    /* look for env var %COMSPEC%, if not set, search COMMAND.COM in path */
    p = getenv("COMSPEC");
    if(p != NULL) {
        strcpy(cmd, p);
    } else {
        /* TODO implement search in %PATH% for command.com. Until then we
         * just return an error.
         */
        return -1;
    }

    args[0] = ' ';          /* WARNING dirty hack, first char of args is always
                             * lost. We place a dummy to get this lost. */
    strcat(args, "/C ");
    strcat(args, string);

    segread(&sregs);
    sregs.es = sregs.ss;

    param_block.env_seg = 0;
    param_block.cmd_off = (unsigned int)args;
    param_block.cmd_seg = __get_ds();
    param_block.fcb1_off = 0x5c;
    param_block.fcb2_off = 0x6c;
    param_block.fcb1_seg = __psp;
    param_block.fcb2_seg = __psp;

    inregs.h.ah = 0x4b;
    inregs.h.al = 0x00;
    inregs.x.dx = (unsigned int)cmd;
    inregs.x.bx = (unsigned int)&param_block;

//     printf("starting command interpreter '%s' with args '%s'\r\n", cmd, args);
    int86x(0x21, &inregs, &outregs, &sregs);

    return 0;
}
