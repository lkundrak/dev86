/* @(#) vstring.h 1.1 91/09/01 23:08:42 */

struct vstring {
    char   *str;			/* string value */
    char   *last;			/* last position */
};

extern struct vstring *vs_alloc();	/* initial allocation */
extern char *vs_realloc();		/* string extension */

/* macro to add one character to auto-resized string */

#define	VS_ADDCH(vs,wp,c) \
    ((wp < (vs)->last || (wp = vs_realloc(vs,wp))) ? (*wp++ = c) : 0)
