/* sizes.h - target scalar type sizes for bcc */

/* Copyright (C) 1992 Bruce Evans */

/*
  the compiler is not very portable in this area
  it only directly supports I8088-I80386 and MC6809
  it assumes
      sizeof(source long) >= sizeof(target long)
      usual register size = int
      long = 2 int sizes
*/

#define CHBITSTO	8	/* bits in a character */
#define CHMASKTO	0xFF	/* mask to reduce SOURCE int to TARGET uchar */
#define INT16BITSTO	16	/* not accessed in non-16 bit case */
#define INT32BITSTO	32	/* not accessed in non-32 bit case */
#define MAXINTBITSTO	32	/* max bits in an integer (var processors) */
#define MAXSCHTO	127	/* maximum signed character */
#define MAXUCHTO	255	/* maximum unsigned character */
#define MINSCHTO	(-128)	/* minimum signed character */

#ifdef MC6809
# define is5bitoffset(n)   ((uoffset_T) (n) + 0x10 < 0x20)
#endif
#define isbyteoffset(n)	((uoffset_T) (n) - MINSCHTO <= MAXSCHTO - MINSCHTO)
#define ischarconst(n)	((uvalue_t) (n) <= MAXUCHTO)
#define isnegbyteoffset(n) ((uvalue_t) (n) + MAXSCHTO <= MAXSCHTO - MINSCHTO)
#define isshortbranch(n)   ((uoffset_T) (n) - MINSCHTO <= MAXSCHTO - MINSCHTO)

#ifdef MC6809
/* Hack to reduce number of direct page variables. */
#define intmaskto	((uvalue_t) 0xFFFFL)
#define maxintto	((uvalue_t) 0x7FFFL)
#define maxlongto	((uvalue_t) 0x7FFFFFFFL)
#define maxoffsetto	((uvalue_t) 0x7FFFL)
#define maxshortto	((uvalue_t) 0x7FFFL)
#define maxuintto	((uvalue_t) 0xFFFFL)
#define maxushortto	((uvalue_t) 0xFFFFL)
#define shortmaskto	((uvalue_t) 0xFFFFL)
#else
extern uvalue_t intmaskto;	/* mask for ints */
extern uvalue_t maxintto;	/* maximum int */
extern uvalue_t maxlongto;	/* maximum long */
extern uvalue_t maxoffsetto;	/* maximum offset */
extern uvalue_t maxshortto;	/* maximum short */
extern uvalue_t maxuintto;	/* maximum unsigned */
extern uvalue_t maxushortto;	/* maximum unsigned short */
extern uvalue_t shortmaskto;	/* mask for shorts */
#endif
