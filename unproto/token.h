/* @(#) token.h 1.3 91/11/30 21:10:37 */

struct token {
    int     tokno;			/* token value, see below */
    int     len;			/* string or list length */
    struct vstring *vstr;		/* token contents */
    struct token *next;
    struct token *head;
    struct token *tail;
};

/* Special token values */

#define	TOK_LIST	256		/* () delimited list */
#define	TOK_WORD	257		/* keyword or identifier */
#define	TOK_NUMBER	258		/* number */
#define	TOK_WSPACE	259		/* white space except newline */
#define	TOK_OTHER	260		/* other multi-char token */
#define	TOK_CONTROL	261		/* flow control keyword */
#define	TOK_COMPOSITE	262		/* struct or union */

/* Input/output functions and macros */

extern struct token *tok_get();		/* read next single token */
extern void tok_show();			/* display (composite) token */
extern struct token *tok_class();	/* classify tokens */
extern void put_ch();			/* write character */
extern void put_str();			/* write string */
extern void tok_unget();		/* stuff token back into input */

#define	tok_flush(t)	(tok_show(t), tok_free(t))

/* tok_get() and tok_class() options */

#define	DO_WSPACE	0		/* retain space, tab */
#define	NO_WSPACE	1		/* skip space, tab */

/* Memory management */

struct token *tok_alloc();		/* allocate token storage */
extern void tok_free();			/* re-cycle storage */

/* Context */

extern char curr_path[];		/* current path name */
extern int curr_line;			/* current line number */
#define	show_line_control() printf("# %d %s\n", curr_line, curr_path);
