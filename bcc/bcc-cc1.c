/* bcc-cc1.c - "pass 1" for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "bcc.h"

PUBLIC int main(argc, argv)
int argc;
char **argv;
{
    growheap(0);		/* init order is important */
    syminit();
    etreeinit();
    ifinit();
    predefine();
    openio(argc, argv);
    codeinit();
    typeinit();
    program();
    finishup();

    /* NOTREACHED */
    return 0;
}
