/* stdio.h */

#asm

#define BUFSIZ   512
#define _NFILE   20

#define _IOREAD  1
#define _IOWRITE 2
#define _IODIRTY 4
#define _IOEOF   8
#define _IOERR   16
#define _IOMYBUF 32		/* says if stream routines allocated buffer */
#define _IOUNBUF 64

#define EOF      (-1)
#define NULL     0

#endasm

#define	stdin    (&_iob[0])
#define	stdout   (&_iob[1])
#define	stderr   (&_iob[2])

#define FILE     struct _iobuf

struct _iobuf
{
	char *_ptr;
	char *_base;
	char *_rtop;
	char *_wtop;
	char _flags;
	char _fd;
}
	_iob[_NFILE];

#define clearerr(fp) ((fp)->flags&=~_IOERR)
#define getchar()    getc(stdin)
#define feof(fp)     ((fp)->_flags&_IOEOF)
#define ferror(fp)   ((fp)->_flags&_IOERR)
#define fileno(fp)   ((fp)->_fd)
#define putchar(c)   putc((c),stdout)

#define void int

FILE *fdopen();
char *fgets();
FILE *fopen();
FILE *freopen();
long ftell();

long lseek();
unsigned read();
unsigned write();

char *malloc();
char *realloc();
char *sbrk();

char *index();
char *rindex();
char *strcat();
char *strcpy();
char *strncat();
char *strncpy();

#asm

BLANK		EQU	32
COEOL		EQU	10
EOL		EQU	13
MAXCONTROL	EQU	31

* struct _iobuf translated into offsets

	BLOCK 0
PTR		RMB	2
BASE		RMB	2
RTOP		RMB	2
WTOP		RMB	2
FLAGS		RMB	1
FD		RMB	1
IOB.SIZE
	ENDB

#endasm
