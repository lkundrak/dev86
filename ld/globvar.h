/* globvar.h - global variables for linker */

/* Copyright (C) 1994 Bruce Evans */

#ifdef EXTERN
EXTERN char hexdigit[];
#else
#define EXTERN
PUBLIC char hexdigit[] = "0123456789abcdef";
#endif
EXTERN unsigned errcount;		/* count of errors */
EXTERN struct entrylist *entryfirst;	/* first on list of entry symbols */
EXTERN struct modstruct *modfirst;	/* data for 1st module */
EXTERN struct redlist *redfirst;	/* first on list of redefined symbols */
