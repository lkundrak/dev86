/* gencode.h - code generation for bcc */

/* Copyright (C) 1992 Bruce Evans */

#ifdef MC6809
# define CANABXCUTOFF 7		/* favour ABX over MUL before this */
#endif
#define EXPRLEVEL 126		/* level for symbols in exptree, > real levs */
#define OFFKLUDGELEVEL 127	/* level for expr sym with offset from name */
#define OPERANDSEPARATOR ','	/* char separating operands */
#define OPSEPARATOR '\t'	/* char separating op string and operand */

EXTERN uoffset_t arg1size;	/* size of 1st arg to function */
				/* zero after allocation of 1st arg */
EXTERN store_pt callee1mask;	/* calleemask with doubleregs masked if nec */
EXTERN uoffset_t dataoffset;	/* amount of initialized data so far */
#ifdef DEBUG
EXTERN bool_t debugon;		/* nonzero to print debugging messages */
				/* depends on zero init */
#endif
#ifdef FRAMEPOINTER
EXTERN store_pt framelist;	/* bit pattern for frame and saved regs */
EXTERN store_pt frame1list;	/* framelist with doubleregs masked if nec */
EXTERN offset_t framep;		/* hardware relative frame ptr */
#endif
EXTERN uoffset_t func1saveregsize;  /* choice of next two values */
EXTERN uoffset_t funcdsaveregsize;  /* funcsaveregsize adjusted for doubles */
EXTERN uoffset_t funcsaveregsize;  /* tot size of framelist/calleemask regs */
#ifdef I8088
EXTERN bool_t i386_32;		/* nonzero to generate 386 32 bit code */
				/* depends on zero init */
#endif
#ifdef DYNAMIC_LONG_ORDER
EXTERN bool_t long_big_endian;	/* nonzero if high long word is first */
				/* depends on zero init */
#endif
EXTERN offset_t lowsp;		/* low water sp (collects locals in switch) */
#ifdef POSINDEPENDENT
EXTERN bool_t posindependent;	/* nonzero to generate pos-independent code */
				/* depends on zero init */
#endif
EXTERN bool_t printf_fp;	/* nonzero if *printf called with FP arg  */
EXTERN bool_t regarg;		/* nonzero to show unloaded register arg */
				/* depends on zero init */
EXTERN store_t reguse;		/* registers in use */
EXTERN bool_t scanf_fp;		/* nonzero if *scanf called with ptr-to-FP */
EXTERN offset_t softsp;		/* software sp (leads sp during declares) */
EXTERN offset_t sp;		/* hardware relative stack ptr */
				/* depends on zero init */
#ifdef FRAMEPOINTER
EXTERN bool_t stackarg;		/* nonzero to show function has arg on stack */
#endif
EXTERN struct switchstruct *switchnow;	/* currently active switch */
				/* depends on NULL init */

/* variables to be initialised to nonzero */

extern store_pt allindregs;	/* mask (in) for index registers */
extern store_pt allregs;	/* mask (in) for registers */
extern bool_t arg1inreg;	/* nonzero to pass 1st arg in reg */
extern store_pt calleemask;	/* mask (in) for regs to be saved by callee */
extern bool_t callersaves;	/* nonzero to make caller save regs */
extern char *callstring;	/* opcode string for call */
extern store_pt doubleargregs;	/* mask (in) for regs for 1st arg if double */
extern store_pt doubleregs;	/* mask (in) for regs to temp contain double */
extern store_pt doublreturnregs;  /* mask (in) for regs for returning double */
extern offset_t jcclonger;	/* amount jcc long jumps are longer */
extern offset_t jmplonger;	/* amount long jumps is longer */
extern char *jumpstring;	/* opcode string for jump */
extern char *regpulllist;	/* reg names and sizes (0 store_t bit first) */
extern char *regpushlist;	/* reg names and sizes (0 store_t bit last) */
extern store_pt regregs;	/* mask (in) for regs which can be reg vars */

/* register names */

extern char *acclostr;
extern char *accumstr;
extern char *badregstr;
#ifdef I8088
extern char *dreg1str;
extern char *dreg1bstr;
extern char *dreg2str;
#endif
extern char *ireg0str;
extern char *ireg1str;
extern char *ireg2str;
extern char *localregstr;
#ifdef I8088
extern char *stackregstr;
#endif

/* register sizes */

extern uoffset_t accregsize;
#ifdef FRAMEPOINTER
extern uoffset_t frameregsize;
#endif
extern uoffset_t maxregsize;
extern uoffset_t opregsize;
extern uoffset_t pshregsize;
extern uoffset_t returnadrsize;
