/* codefrag.c - code fragments for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "bcc.h"
#include "byteord.h"
#include "condcode.h"
#include "gencode.h"
#include "label.h"
#include "output.h"
#include "reg.h"
#include "scan.h"
#include "sizes.h"

#define DEFSTR_BYTEMAX 10
#define DEFSTR_DELIMITER '"'
#define DEFSTR_STRINGMAX 40
#define EOS_TEXT '0'
#define MAXPRINTCHAR '~'
#define MINPRINTCHAR ' '

/* segment numbers */

#ifdef I8088
# define CSEG 0
# define outcseg() outop0str(".text\n")
# define DSEG 1
# define outdseg() outop0str(".data\n")
# define BSSSEG 2
# define outbssseg() outop0str(".bss\n")
#endif

#ifdef MC6809
# define CSEG 0
# define outcseg() outop0str("LOC\t0\n")
# define DPSEG 2
# define outdpseg() outop0str("LOC\t2\n")
# define DSEG 3
# define outdseg() outop0str("LOC\t3\n")
# define STRSEG 1
# define outstrseg() outop0str("LOC\t1\n")
#endif

#ifdef I8088
FORWARD void adjcarry P((void));
#endif
FORWARD void clr P((store_pt reg));
FORWARD bool_pt lowregisDreg P((void));
#ifdef I8088
FORWARD void outand P((void));
FORWARD void outequate P((void));
# ifdef XENIX_AS
FORWARD void outexport P((void));
# endif
FORWARD void outmovsx P((void));
FORWARD void outmovzx P((void));
FORWARD void tfrhilo P((void));
FORWARD void tfrlohi P((void));
#endif
#ifdef MC6809
FORWARD void negBsbcA P((void));
#endif
FORWARD void outaccum P((void));
FORWARD void outstackreg P((void));
FORWARD void opregadr P((void));

/* operator and miscellaneous strings */

#ifdef I8088

# define ACCHISTR "ah"
# define ANDSTRING "and\t"
# define DEFSTR_QUOTER '\\'
# define EORSTRING "xor\t"
# define MAX_INLINE_SHIFT 2	/* better 3 for 88, 1 for 186 and above */
# define ORSTRING "or\t"
# define TARGET_FIRST
# define addfactor(reg) (outadd(), outregname(reg), outncregname(DXREG))
# define defstorage() outop0str(".blkb\t")
# define extBnegD() (ctoi(), negDreg())
# define finishfactor()		/* save/add/subfactor() ended already */
# define outadc() outop3str("adc\t")
# define outandac() (outand(), outaccum(), bumplc())
# define outandlo() (outand(), outstr(acclostr))
# define outbimmed() outbyte('*')
# define outcommon() outop0str(".comm\t")
# define outcwd() outnop1str("cwd")
# define outdefstr() outop0str(".ascii\t\"")
# define outexchange() outop1str("xchg\t")
# define outglobl() outop0str(".globl\t")
# ifdef XENIX_AS
#  define outimport() outexport()
# else
#  define outexport() outop0str("export\t")
#  define outimport() outop0str("import\t")
# endif
# ifdef XENIX_AS
#  define outj1switch() outop3str("seg\tcs\nbr\t@");
# else
#  define outj1switch() outop3str("seg\tcs\nbr\t");
# endif
# define outj2switch() \
	(outindleft(), outstr(ireg0str), outindright(), bumplc2(), outnl())
# define outlcommon() outop0str("\tlcomm\t")
# define outlswitch() (outload(), outstr(ireg0str), outncregname(DREG))
# define outnc1() outnstr(",*1")
# define outsbc() outop3str("sbb\t")
# define outset() outstr ("\tset\t")
# define outsl() outop2str("shl\t")
# define outsr() outop2str("sar\t")
# define outtransfer() outload()
# define outusr() outop2str("shr\t")
# define outxor() outop2str(EORSTRING)
# define reclaimfactor()	/* factor in DXREG, DXREG now junk */
# define savefactor(reg) regtransfer((reg), DXREG)
# define smiDreg() (outcwd(), regexchange(DREG, DXREG))
# define sr1() (outsr(), outaccum(), outnc1())
# define subfactor(reg) (outsub(), outregname(reg), outncregname(DXREG))
# define usr1() (outusr(), outaccum(), outnc1())
PRIVATE void adjcarry()
{
    outop3str("rcl\t");
    outregname(DXREG);
    outncimmadr((offset_T) 9);
    outand();
    bumplc2();
    bumplc2();
    outregname(DXREG);
    outncimmadr((offset_T) 0x100);
}
PUBLIC void clrBreg()
{
    outxor();
    outstr(acclostr);
    outncregname(BREG);
}
PUBLIC void comment()
{
    outstr("! ");
}
PUBLIC void ctoi()
{
#ifdef I80386
    if (i386_32)
    {
	outmovzx();
	outaccum();
	outncregname(BREG);
    }
    else
#endif
    {
	outxor();
	outhiaccum();
	outcomma();
	outhiaccum();
	outnl();
    }
}
PUBLIC void defbyte()
{
    outop0str(".byte\t");
}
#ifdef XENIX_AS
PUBLIC void defword()
{
}				/* don't have to print ".word\t" */
#else
PUBLIC void defword()
{
    outop0str(".word\t");
}
#endif
PUBLIC void defdword()
{
    outop0str("dd\t");
}
PUBLIC void even()
{
    outop0str(".even\n");
}
PUBLIC void negDreg()
{
    outop2str("neg\t");
    outnregname(DREG);
}
PUBLIC void comDreg()
{
    outop2str("not\t");
    outnregname(DREG);
}
PUBLIC void outadd()
{
    outop2str("add\t");
}
PUBLIC void outaddsp()
{
    outadd();
    outstackreg();
    outcomma();
    outimmed();
    bumplc2();
}
PRIVATE void outand()
{
    outop2str(ANDSTRING);
}
#ifdef XENIX_AS
PUBLIC void outcalladr()
{
    outop2str("call\t@");
}
#else
PUBLIC void outcalladr()
{
    outop2str("call\t");
}
#endif
PUBLIC void outcmp()
{
    outop2str("cmp\t");
}
PUBLIC void outdec()
{
    outop1str("dec\t");
}
PUBLIC void outdword()
{
    outstr("dword ");
}
PRIVATE void outequate()
{
    outop0str("\t=\t");
}
#ifdef XENIX_AS
PRIVATE void outexport()
{
    outop0str(".globl\t");
}
#endif
PUBLIC void outfail()
{
    outop0str(".fail\t");
}
PUBLIC void outinc()
{
    outop1str("inc\t");
}
#ifdef XENIX_AS
PUBLIC void outindleft()
{
    outbyte('(');
}
PUBLIC void outindright()
{
    outbyte(')');
}
#else
PUBLIC void outindleft()
{
    outbyte('[');
}
PUBLIC void outindright()
{
    outbyte(']');
}
#endif
#ifndef FRAMEPOINTER
PUBLIC void outindstackreg()
{
    outindleft();
    outregname(STACKREG);
    outindright();
}
#endif
PUBLIC void outldaccum()
{
    outload();
    outaccum();
    outcomma();
}
PUBLIC void outldmulreg()
{
    outload();
    outregname(MULREG);
    outcomma();
}
PUBLIC void outlea()
{
    outop2str("lea\t");
}
PUBLIC void outleasp()
{
    outlea();
    outstackreg();
    outcomma();
}
PUBLIC void outload()
{
    outop2str("mov\t");
}
PRIVATE void outmovsx()
{
    outop3str("movsx\t");
}
PRIVATE void outmovzx()
{
    outop3str("movzx\t");
}
PUBLIC void outmulmulreg()
{
    outop2str("mul\t");
    outnregname(MULREG);
}
PUBLIC void outopsep()
{
    outcomma();
}
PUBLIC void outpshs()
{
    outop1str("push");
}
PUBLIC void outpuls()
{
    outop1str("pop");
}
PUBLIC void outreturn()
{
    outnop1str("ret");
}
PUBLIC void outstore()
{
    outload();
}
PUBLIC void outsub()
{
    outop2str("sub\t");
}
PUBLIC void outtest()
{
    outop2str("test\t");
}
PUBLIC void outword()
{
    outstr("word ");
}
PUBLIC void sctoi()
{
#ifdef I80386
    if (i386_32)
    {
	outmovsx();
	outaccum();
	outncregname(BREG);
    }
    else
#endif
	outnop1str("cbw");
}
PUBLIC void stoi()
{
    outnop1str("cwde");
}
PRIVATE void tfrhilo()
{
    outload();
    outstr(acclostr);
    outcomma();
    outhiaccum();
    outnl();
}
PRIVATE void tfrlohi()
{
    outload();
    outhiaccum();
    outncregname(BREG);
}
#ifdef I80386
PUBLIC void ustoi()
{
    outmovzx();
    outaccum();
    outcomma();
    outshortregname(DREG);
    outnl();
}
#endif /* I80386 */
#endif /* I8088 */

#ifdef MC6809

# define ACCHISTR "A"
# define ANDSTRING "AND"
# define DEFSTR_QUOTER '"'
# define EORSTRING "EOR"
# define MAX_INLINE_SHIFT 16
# define ORSTRING "OR"
# define addfactor(reg) outop2str("ADDD\t,S")
# define defstorage() outop0str("RMB\t")
# define extBnegD() (ctoi(), negBsbcA())
# define finishfactor() outnl()
# define outadc() outop2str("ADC")
# define outandhi() outop2str("ANDA")
# define outandlo() outop2str("ANDB")
# define outcommon() outop0str("\tCOMM\t")
# define outdefstr() outop0str("FCC\t\"")
# define outequate() outop0str("\tEQU\t")
# define outexchange() outop2str("EXG\t")
# define outexport() outop0str("EXPORT\t")
# define outglobl() outop0str("GLOBL\t")
# define outimport() outop0str("IMPORT\t")
# define outjswitch() outnop2str("JMP\t[D,X]")
# define outlcommon() outop0str("\tLCOMM\t")
# define outlswitch() outop3str("LDX\t#")
# define outpijswitch() outnop2str("JMP\tD,X")
# define outpil1switch() outop3str("LEAX\t<")
# define outpil2switch() outnop2str("LDD\tD,X")
# define outrolhi() outnop1str("ROLA");
# define outsbc() outop2str("SBC")
# define outset() outstr ("\tSET\t")
# define outsl() outop1str("LSL")
# define outtransfer() outop2str("TFR\t")
# define reclaimfactor() outnstr ("++")	/* discard factor from stack */
# define savefactor(reg) outop2str("PSHS\tD")
# define smiDreg() (clrBreg(), outnop1str("ROLA"), \
		    outnop2str("SBCB\t#0"), sctoi())
	/* this tricky sequence is twice as fast as TFR A,B; SEX; TFR A,B */
	/* it gets the sign bit of A in the carry */
	/* then subtracts it from 0 in D (effectively) */
# define sr1() (outnop1str("ASRA"), outnop1str("RORB"))
# define stackregstr "S"
# define subfactor(reg) outop2str("SUBD\t,S")
# define testhi() outnop1str("TSTA")
# define tfrhilo() outnop2str("TFR\tA,B")
# define tfrlohi() outnop2str("TFR\tB,A")
# define usr1() (outnop1str("LSRA"), outnop1str("RORB"))
PUBLIC void clrBreg()
{
    outnop1str("CLRB");
}
PUBLIC void comment()
{
    outstr("| ");
}
PUBLIC void defbyte()
{
    outop0str("FCB\t");
}
PUBLIC void defword()
{
    outop0str("FDB\t");
}
PUBLIC void negDreg()
{
    outnop1str("NEGA");
    negBsbcA();
}
PRIVATE void negBsbcA()
{
    outnop1str("NEGB");
    sbc0();
}
PUBLIC void comDreg()
{
    outnop1str("COMA");
    outnop1str("COMB");
}
PUBLIC void outABX()
{
    outnop1str("ABX");
}
PUBLIC void outadd()
{
    outop2str("ADD");
}
PUBLIC void outaddsp()
{
    outleasp();
    bumplc2();
}
PUBLIC void outcalladr()
{
    outop2str("JSR");
}
PUBLIC void outcmp()
{
    outop2str("CMP");
}
PUBLIC void outdec()
{
    outop1str("DEC");
}
PUBLIC void outdirectpage()
{
    outbyte('<');
}
PUBLIC void outextended()
{
    outbyte('>');
}
PUBLIC void outfail()
{
    outop0str("FAIL\t");
}
PUBLIC void outinc()
{
    outop1str("INC");
}
PUBLIC void outindleft()
{
    outbyte('[');
}
PUBLIC void outindright()
{
    outbyte(']');
}
PUBLIC void outldaccum()
{
    outload();
    outaccum();
}
PUBLIC void outldmulreg()
{
    outop2str("LDA");
}
PUBLIC void outlea()
{
    outop2str("LEA");
}
PUBLIC void outleasp()
{
    outop2str("LEAS\t");
}
PUBLIC void outload()
{
    outop2str("LD");
}
PUBLIC void outmulmulreg()
{
    outnop1str("MUL");
}
PUBLIC void outncspregname()
{
    outcomma();
    outstackreg();
    outnl();
}
PUBLIC void outopsep()
{
}				/* is tab, but already done by outadr() */
PUBLIC void outpshs()
{
    outop2str("PSHS");
}
PUBLIC void outpuls()
{
    outop2str("PULS");
}
PUBLIC void outreturn()
{
    outnop1str("RTS");
}
PUBLIC void outstore()
{
    outop2str("ST");
}
PUBLIC void outsub()
{
    outop2str("SUB");
}
PUBLIC void outtest()
{
    outop1str("TST");
}
PUBLIC void sctoi()
{
    outnop1str("SEX");
}
PUBLIC void ctoi()
{
    outnop1str("CLRA");
}
#endif /* MC6809 */
#ifdef FRAMEREG
PUBLIC void outindframereg()
{
    outindleft();
    outregname(FRAMEREG);
    outindright();
}
#endif

typedef fastin_t seg_t;		/* range 0..3 */

PRIVATE seg_t segment;		/* current seg, depends on init to CSEG = 0 */

/* add carry resulting from char addition */

PUBLIC void adc0()
{
#ifdef I80386
    if (i386_32)
    {
	adjcarry();
	outadd();
	outaccum();
	outncregname(DXREG);
    }
    else
#endif
    {
	outadc();
	outhiaccum();
	outncimmadr((offset_T) 0);
    }
}

/* add constant to register */

PUBLIC void addconst(offset, reg)
offset_T offset;
store_pt reg;
{
#ifdef I8088
#ifdef I80386
    if ((i386_32 && (uoffset_T) offset + 1 <= 2)  /* do -1 to 1 by dec/inc */
	|| (!i386_32 && (uoffset_T) offset + 2 <= 4))	/* do -2 to 2  */
#else
    if ((uoffset_T) offset + 2 <= 4)	/* do -2 to 2  */
#endif
    {
	if (reg == ALREG)
	    reg = AXREG;	/* shorter and faster */
	do
	{
	    if (offset < 0)
	    {
		outdec();
		++offset;
	    }
	    else		/* if offset == 0, do inc + dec */
	    {
		outinc();
		--offset;	/* shouldn't happen and harmless */
	    }
	    outnregname(reg);
	}
	while (offset);
    }
    else
#endif
#ifdef MC6809
    if (!(reg & ALLDATREGS))
	lea(offset, reg, reg);
    else if (reg == BREG && (offset == 1 || offset == -1))
    {
	if (offset < 0)
	    outdec();
	else
	    outinc();
	outnregname(reg);
    }
    else
#endif
    {
	outadd();
	outimadj(offset, reg);
    }
}

/* adjust lc for signed offset */

PUBLIC void adjlc(offset, reg)
offset_T offset;
store_pt reg;
{
    if (!(reg & CHARREGS))
    {
	bumplc();
	if (!isbyteoffset(offset))
	{
#ifdef I8088
	    if ((store_t) reg != AXREG)
#endif
		bumplc();
#ifdef I80386
	    if (i386_32)
		bumplc2();
#endif
	}
    }
}

/* adjust stack ptr by adding a labelled constant less current sp */

PUBLIC void adjsp(label)
label_no label;
{
    outaddsp();
    outbyte(LOCALSTARTCHAR);
    outlabel(label);
    if (switchnow != NULL)
    {
	outminus();
	outswstacklab();
    }
    else
    {
	outplus();
	outhex((uoffset_T) - sp);
    }
#ifdef MC6809
    outcregname(LOCAL);
#endif
#ifdef I80386
    if (i386_32)
	bumplc2();
#endif
    outnl();
}

/* and accumulator with constant */

PUBLIC void andconst(offset)
offset_T offset;
{
    char_t botbits;
    uoffset_T topbits;

    if ((topbits = offset & ~(uoffset_T) CHMASKTO & intmaskto) != 0 &&
	topbits != (~(uoffset_T) CHMASKTO & intmaskto))
	/* if topbits == 0, callers reduce the type */
    {
#ifdef OP1
	outandhi();
	outncimmadr((offset_T) (topbits >> (INT16BITSTO - CHBITSTO)));
#else
	outandac();
#ifdef I80386
	if (i386_32)
	    bumplc2();
#endif
	outncimmadr(offset);
	return;
#endif
    }
    if ((botbits = (char_t) offset & CHMASKTO) == 0)
	clrBreg();
    else if (botbits != CHMASKTO)
    {
	outandlo();
	outncimmadr((offset_T) botbits);
    }
}

#ifdef I8088

/* set bss segment */

PUBLIC void bssseg()
{
    if (segment != BSSSEG)
    {
	segment = BSSSEG;
	outbssseg();
    }
}

#endif

/* jump to case of switch */

PUBLIC label_no casejump()
{
    label_no jtablelab;

#ifdef I8088
    outlswitch();
    outj1switch();
    outlabel(jtablelab = getlabel());
    outj2switch();
#ifdef I80386
    if (i386_32)
	bumplc2();
#endif
#endif
#ifdef MC6809
    if (posindependent)
    {
	outpil1switch();
	outlabel(jtablelab = getlabel());
	outncregname(GLOBAL);
	outpil2switch();
	outpijswitch();
    }
    else
    {
	outlswitch();
	outnlabel(jtablelab = getlabel());
	outjswitch();
    }
#endif
    return jtablelab;
}

/* clear register to 0 */

PRIVATE void clr(reg)
store_pt reg;
{
    loadconst((offset_T) 0, reg);
}

/* define common storage */

PUBLIC void common(name)
char *name;
{
#ifdef I8088
    outcommon();
    outccname(name);
    outcomma();
#endif
#ifdef MC6809
    outccname(name);
    outcommon();
#endif
}

/* set code segment */

PUBLIC void cseg()
{
    if (segment != CSEG)
    {
	segment = CSEG;
	outcseg();
    }
}

/* define long */

PUBLIC void deflong(value)
uoffset_T value;
{
    uoffset_T longhigh;
    uoffset_T longlow;

    longlow = value & (uoffset_T) intmaskto;
#ifdef I80386
    if (i386_32)
	defdword();
    else
#endif
    {
	longhigh = (value >> INT16BITSTO) & (uoffset_T) intmaskto;
	defword();
#if DYNAMIC_LONG_ORDER
	if (long_big_endian)
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN
	    outnhex(longhigh);
#endif
#if DYNAMIC_LONG_ORDER
	else
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN == 0
	{
	    outnhex(longlow);
	    longlow = longhigh;
	}
#endif
	defword();
    }
    outnhex(longlow);
}

/* define null storage */

PUBLIC void defnulls(nullcount)
uoffset_T nullcount;
{
    if (nullcount != 0)
    {
	defstorage();
	outnhex(nullcount);
    }
}

/* define string */

PUBLIC label_no defstr(sptr, stop, dataflag)
char *sptr;
char *stop;
bool_pt dataflag;
{
    int byte;			/* promoted char for output */
    label_no strlab;
    seg_t oldsegment;
    fastin_t count;		/* range 0..max(DEFSTR_BYTEMAX,DEFSTR_STRMAX) */

#ifdef HOLDSTRINGS
    if (!(bool_t) dataflag)
	return holdstr(sptr, stop);
#endif
    oldsegment = segment;
#ifdef I8088
    dseg();
#endif
#ifdef MC6809
    if (dataflag)
	dseg();
    else
    {
	segment = STRSEG;	/* could use dseg() */
	outstrseg();		/* this brings strings together */
    }
#endif
    outnlabel(strlab = getlabel());
    byte = (unsigned char) *sptr++;
    while (sptr <= stop)
    {
	if ((unsigned char) byte >= MINPRINTCHAR
	    && (unsigned char) byte <= MAXPRINTCHAR)
	{
	    outdefstr();
	    count = DEFSTR_STRINGMAX;
	    while (count-- > 0 && (unsigned char) byte >= MINPRINTCHAR
		   && (unsigned char) byte <= MAXPRINTCHAR && sptr <= stop)
	    {
#if DEFSTR_DELIMITER - DEFSTR_QUOTER
		if ((unsigned char) byte == DEFSTR_DELIMITER
		    || (unsigned char) byte == DEFSTR_QUOTER)
#else
		if ((unsigned char) byte == DEFSTR_DELIMITER)
#endif
		    outbyte(DEFSTR_QUOTER);
		outbyte(byte);
		byte = (unsigned char) *sptr++;
	    }
	    outnbyte(DEFSTR_DELIMITER);
	}
	else
	{
	    defbyte();
	    count = DEFSTR_BYTEMAX;
	    while (count-- > 0 && ((unsigned char) byte < MINPRINTCHAR
		   || (unsigned char) byte > MAXPRINTCHAR) && sptr <= stop)
	    {
		if (count < DEFSTR_BYTEMAX - 1)
		    outcomma();	/* byte separator */
		outhex((uoffset_T) byte);
		byte = (unsigned char) *sptr++;
	    }
	    outnl();
	}
    }
    defbyte();
    outnbyte(EOS_TEXT);
    switch (oldsegment)
    {
    case CSEG:
	cseg();
	break;
    case DSEG:
	dseg();
	break;
#ifdef I8088
    case BSSSEG:
	bssseg();
	break;
#endif
#ifdef MC6809
    case DPSEG:
	dpseg();
	break;
#endif
    }
    return strlab;
}

/* divide D register by a constant if it is easy to do with shifts */

PUBLIC bool_pt diveasy(divisor, uflag)
value_t divisor;
bool_pt uflag;
{
    bool_t sign;

    sign = FALSE;
    if (divisor < 0 && !(bool_t) uflag)
    {
	sign = TRUE;
	divisor = -divisor;
    }
    if (bitcount((uvalue_t) divisor) > 1)
	return FALSE;
    if (divisor == 0)
	clr(DREG);
    else
    {
	if (sign)
	    negDreg();
	srconst((value_t) highbit((uvalue_t) divisor), uflag);
    }
    return TRUE;
}

#ifdef MC6809

/* set direct page segment */

PUBLIC void dpseg()
{
    if (segment != DPSEG)
    {
	segment = DPSEG;
	outdpseg();
    }
}

#endif

/* set data segment */

PUBLIC void dseg()
{
    if (segment != DSEG)
    {
	segment = DSEG;
	outdseg();
    }
}

/* equate a name to an EOL-terminated string */

PUBLIC void equ(name, string)
char *name;
char *string;
{
    outstr(name);
    outequate();
    outline(string);
}

/* equate a local label to a value */

PUBLIC void equlab(label, offset)
label_no label;
offset_T offset;
{
    outbyte(LOCALSTARTCHAR);
    outlabel(label);
    outequate();
    outshex(offset);
    outnl();
}

/* import or export a variable */

PUBLIC void globl(name)
char *name;
{
    outglobl();
    outnccname(name);
}

/* import a variable */

PUBLIC void import(name)
char *name;
{
    outimport();
    outnccname(name);
}

/* extend an int to a long */

PUBLIC void itol(reg)
store_pt reg;
{
#define TEMP_LABEL_FOR_REGRESSION_TESTS
#ifdef TEMP_LABEL_FOR_REGRESSION_TESTS
    getlabel();
#endif

    if (lowregisDreg())
    {
#ifdef I8088
	outcwd();
	regtransfer(DXREG, reg);
#else
	label_no exitlab;

	clr(reg);
	testhi();
	sbranch(GE, exitlab = getlabel());
	loadconst((offset_T) - 1, reg);
	outnlabel(exitlab);
#endif
    }
    else
    {
	regtransfer(DREG, reg);
	smiDreg();
    }
}

/* define local common storage */

PUBLIC void lcommlab(label)
label_no label;
{
    outlabel(label);
    outlcommon();
}

PUBLIC void lcommon(name)
char *name;
{
    outccname(name);
    outlcommon();
}

#ifdef MC6809

/* load effective address */

PUBLIC void lea(offset, sourcereg, targreg)
offset_T offset;
store_pt sourcereg;
store_pt targreg;
{
    outlea();
    outregname(targreg);
    outtab();
    outshex(offset);
    outncregname(sourcereg);
}

#endif

/* load constant into given register */

PUBLIC void loadconst(offset, reg)
offset_T offset;
store_pt reg;
{
#ifdef I8088
    if (offset == 0)
    {
	outxor();
	outregname(reg);
	outncregname(reg);
    }
    else
#endif
#ifdef MC6809
    if (offset == 0 && reg == BREG)
	clrBreg();
    else
#endif
    {
	outload();
	outregname(reg);
#ifdef MC6809
	if (reg == YREG)
	    bumplc2();
	else
#endif
	if (reg != BREG)
	{
	    bumplc();
#ifdef I80386
	    if (i386_32)
		bumplc2();
#endif
	}
	outncimmadr(offset);
    }
}

/* convert index half of long reg pair into low half of pair */

PRIVATE bool_pt lowregisDreg()
{
#if DYNAMIC_LONG_ORDER
    if (long_big_endian)
#endif
# if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN
	return FALSE;
#endif
#if DYNAMIC_LONG_ORDER
    else
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN == 0
	return TRUE;
#endif
}

/* partially long shift left register by a constant (negative = infinity) */

PUBLIC int lslconst(shift, reg)
value_t shift;
store_pt reg;
{
    if ((uvalue_t) shift >= INT16BITSTO)
    {
	slconst(shift - INT16BITSTO, lowregisDreg() ? DREG : reg);
	regexchange(reg, DREG);
	clr(lowregisDreg() ? DREG : reg);
	return 0;
    }
#ifdef I8088
    if (shift >= CHBITSTO)
    {
	if (long_big_endian)
	{
	    tfrlohi();
	    outnop2str("mov\tal,bh");
	    outnop2str("mov\tbh,bl");
	    outnop2str("sub\tbl,bl");
	}
	else
	{
	    outnop2str("mov\tbh,bl");
	    outnop2str("mov\tbl,ah");
	    tfrlohi();
	    clrBreg();
	}
	return (int) shift - CHBITSTO;
    }
#endif
    return (int) shift;
}

/* partially long shift right register by a constant (negative = infinity) */

PUBLIC int lsrconst(shift, reg, uflag)
value_t shift;
store_pt reg;
bool_pt uflag;
{
    if ((uvalue_t) shift >= INT16BITSTO)
    {
	if (lowregisDreg())
	    regexchange(reg, DREG);
	srconst(shift - INT16BITSTO, uflag);
	if ((bool_t) uflag)
	    uitol(reg);
	else
	    itol(reg);
	return 0;
    }
#ifdef I8088
    if (shift >= CHBITSTO)
    {
	if (long_big_endian)
	{
	    outnop2str("mov\tbl,bh");
	    outnop2str("mov\tbh,al");
	    tfrhilo();
	    if ((bool_t) uflag)
		ctoi();
	    else
		sctoi();
	}
	else
	{
	    tfrhilo();
	    outnop2str("mov\tah,bl");
	    outnop2str("mov\tbl,bh");
	    if ((bool_t) uflag)
		outnop2str("sub\tbh,bh");
	    else
	    {
		regexchange(reg, DREG);
		sctoi();
		regexchange(reg, DREG);
	    }
	}
	return (int) shift - CHBITSTO;
    }
#endif
    return (int) shift;
}

/* take D register modulo a constant if it is easy to do with a mask */

PUBLIC bool_pt modeasy(divisor, uflag)
value_t divisor;
bool_pt uflag;
{
    bool_t sign;

    sign = FALSE;
    if (divisor < 0 && !(bool_t) uflag)
    {
	sign = TRUE;
	divisor = -divisor;
    }
    if (bitcount((uvalue_t) divisor) > 1)
	return FALSE;
    if (--divisor == 0)
	clrBreg();		/* original divisor 1 or -1 yields 0 */
    else
    {
	if (sign)
	    negDreg();
	andconst((offset_T) divisor);	/* if original divisor 0, this is
					   null */
	if (sign)
	    negDreg();
    }
    return TRUE;
}

/* multiply register by a constant if it is easy to do with shifts */

PUBLIC bool_pt muleasy(factor, reg)
uvalue_t factor;
store_pt reg;
{
    int mulstack[MAXINTBITSTO / 2 + 1];	/* must be signed, not a fastin_t */
    fastin_pt count;
    fastin_t single1skip;
    fastin_t lastcount;
    fastin_t mulsp;
    int stackentry;		/* signed */

#ifdef I8088
    /* Now using imul directly so don't be so keen to shift */
    if( factor > 16 && factor != 32 && factor != 64 && factor != 0xFFFFFFFFL )
       return FALSE;
#endif

    if (factor == 0)
    {
	clr(reg);
	return TRUE;
    }
    single1skip = 0;
    mulsp = -1;			/* may be unsigned, but bumps to 0 */
    while (factor != 0)
    {
	for (lastcount = single1skip; (factor & 1) == 0; factor >>= 1)
	    ++lastcount;
	mulstack[(int)++mulsp] = lastcount;
	/* first time bumps mulsp to 0 even if an unsigned char */
	for (count = 0; (factor & 1) != 0; factor >>= 1)
	    ++count;
	single1skip = 1;
	if (count == 2 && factor == 0)
	    /* 3 = 2 + 1  better than  3 = 4 - 1 */
	    /* but rest of algorithm messed up unless factor now 0 */
	    mulstack[(int)++mulsp] = 1;
	else if (count > 1)
	{
	    single1skip = 0;
	    if (lastcount == 1 && mulsp != 0)
		mulstack[(int)mulsp] = -1 - count;
	    else
		mulstack[(int)++mulsp] = -count;
	}
    }
    if (mulsp > 3)
	return FALSE;
    if (mulsp != 0)
    {
	savefactor(reg);	/* on stack or in reg as nec */
	do
	{
	    finishfactor();	/* finish save/add/subfactor() if nec */
	    stackentry = mulstack[(int)mulsp--];
	    if (stackentry < 0)
	    {
#ifdef I8088
		if (stackentry == -INT32BITSTO)
		    clr(reg);	/* shifting would do nothing */
		else
#endif
		    slconst((value_t) - stackentry, reg);
		subfactor(reg);	/* from wherever put by savefactor() */
	    }
	    else
	    {
		slconst((value_t) stackentry, reg);
		addfactor(reg);	/* from wherever put by savefactor() */
	    }
	}
	while (mulsp != 0);
	reclaimfactor();	/* reclaim storage if nec */
    }
    slconst((value_t) mulstack[0], reg);
    return TRUE;
}

/* negate a register */

PUBLIC void negreg(reg)
store_pt reg;
{
    if ((store_t) reg == BREG)
	extBnegD();
    else
	negDreg();
}

/* return string of operator */

PUBLIC char *opstring(op)
op_pt op;
{
    switch (op)
    {
    case ANDOP:
	return ANDSTRING;
    case EOROP:
	return EORSTRING;
    case OROP:
	return ORSTRING;
    }
    return "badop";
}

/* print DREG (accumulator) */

PRIVATE void outaccum()
{
    outstr(accumstr);
}

/* print a c compiler name with leading CCNAMEPREXFIX */

PUBLIC void outccname(name)
char *name;
{
    outbyte(CCNAMEPREFIX);
    outstr(name);
}

/* print high byte of word accumulator */

PUBLIC void outhiaccum()
{
    outstr(ACCHISTR);
}

/* print immediate address */

PUBLIC void outimmadr(offset)
offset_T offset;
{
#ifdef I8088
    if (!isbyteoffset(offset))
	outimmed();
    else
	outbimmed();
#else
    outimmed();
#endif
    outshex(offset);
}

/* print register, comma, immediate address and adjust lc */

PUBLIC void outimadj(offset, targreg)
offset_T offset;
store_pt targreg;
{
    outregname(targreg);
    adjlc(offset, targreg);
    outncimmadr(offset);
}

/* print immediate address designator */

PUBLIC void outimmed()
{
    outbyte('#');
}

PUBLIC void outjumpstring()
{
    outop3str(jumpstring);
#ifdef I80386
    if (i386_32)
	bumplc2();
#endif
}

/* print cc name, then newline */

PUBLIC void outnccname(name)
char *name;
{
    outccname(name);
    outnl();
}

/* print separator, immediate address, newline */

PUBLIC void outncimmadr(offset)
offset_T offset;
{
#ifdef I8088
    outcomma();
#endif
#ifdef MC6809
    outtab();
#endif
    outimmadr(offset);
    outnl();
}

/* print signed offset and adjust lc */

PUBLIC void outoffset(offset)
offset_T offset;
{
#ifdef MC6809
    if (!is5bitoffset(offset))
#endif
	adjlc(offset, INDREG0);
    outshex(offset);
}

/* print stack register */

PRIVATE void outstackreg()
{
    outstr(stackregstr);
}

PUBLIC void public(name)
char *name;
{
#ifndef AS09
    outexport();
    outnccname(name);
#endif
    outccname(name);
    outnbyte(PUBLICENDCHAR);
}

/* print cc name as a private label */

PUBLIC void private(name)
char *name;
{
#ifdef LABELENDCHAR
    outccname(name);
    outnbyte(LABELENDCHAR);
#else
    outnccname(name);
#endif
}

/* exchange registers */

PUBLIC void regexchange(sourcereg, targreg)
store_pt sourcereg;
store_pt targreg;
{
    outexchange();
    outregname(sourcereg);
    outncregname(targreg);
#ifdef I8088
    if (!((sourcereg | targreg) & AXREG))
	bumplc();
#endif
}

/* transfer a register */

PUBLIC void regtransfer(sourcereg, targreg)
store_pt sourcereg;
store_pt targreg;
{
    outtransfer();
#ifdef TARGET_FIRST
    outregname(targreg);
    outncregname(sourcereg);
#else
    outregname(sourcereg);
    outncregname(targreg);
#endif
}

/* subtract carry resulting from char addition */

PUBLIC void sbc0()
{
#ifdef I80386
    if (i386_32)
    {
	adjcarry();
	outsub();
	outaccum();
	outncregname(DXREG);
    }
    else
#endif
    {
	outsbc();
	outhiaccum();
	outncimmadr((offset_T) 0);
    }
}

/* set a name to a value */

PUBLIC void set(name, value)
char *name;
offset_T value;
{
    outccname(funcname);
    outbyte(LOCALSTARTCHAR);
    outstr(name);
    outset();
    outshex(value);
#ifdef FRAMEPOINTER
#ifdef I8088
#ifndef NO_DEL_PUSH
    if (framep && optimise && !callersaves && value+sp-framep >= 0 
	  && !(regfuse & callee1mask)) {
	outbyte('-');
	outstr(funcname);
	outstr(".off");
    }
#endif
#endif
#endif
    outnl();
#ifdef FRAMEPOINTER
    if (framep) 
    {
       outbyte(LOCALSTARTCHAR);
       outstr(funcname);
       outbyte(LOCALSTARTCHAR);
       outstr(name);
       outset();
       outshex(value+sp-framep);
#ifdef I8088
#ifndef NO_DEL_PUSH
       if (optimise && !callersaves && value+sp-framep < 0 
	     && !(regfuse & callee1mask)) {
           outbyte('+');
           outstr(funcname);
           outstr(".off");
       }
#endif
#endif
       outnl();
    }
#endif
}

/* shift left register by 1 */

PUBLIC void sl1(reg)
store_pt reg;
{
    outsl();
#ifdef I8088
    outregname(reg);
    outnc1();
#endif
#ifdef MC6809
    outnregname(BREG);
    outrolhi();
#endif
}

/* shift left register by a constant (negative = infinity) */

PUBLIC void slconst(shift, reg)
value_t shift;
store_pt reg;
{
#ifdef I80386
    if (i386_32)
    {
	if ((shift = (uvalue_t) shift % INT32BITSTO) != 0)
	{
	    outsl();
	    if (shift != 1)
		bumplc();
	    outregname(reg);
	    outncimmadr((offset_T) shift);
	}
	return;
    }
#endif
    if ((uvalue_t) shift >= INT16BITSTO)
	clr(reg);
    else
    {
	if (shift >= CHBITSTO && reg == DREG)
	{
	    tfrlohi();
	    clrBreg();
	    shift -= CHBITSTO;
	}
#ifdef I8088
# if MAX_INLINE_SHIFT < INT16BITSTO
	if (shift > MAX_INLINE_SHIFT)
	{
	    outload();
	    outregname(SHIFTREG);
	    outcomma();
	    outimmadr((offset_T) shift);
	    outnl();
	    outsl();
	    outregname(reg);
	    outncregname(SHIFTREG);
	}
	else
# endif
#endif
	    while (shift--)
		sl1(reg);
    }
}

/* shift right D register by a constant (negative = infinity) */

PUBLIC void srconst(shift, uflag)
value_t shift;
bool_pt uflag;
{
#ifdef I80386
    if (i386_32)
    {
	if ((shift = (uvalue_t) shift % INT32BITSTO) != 0)
	{
	    if (uflag)
		outusr();
	    else
		outsr();
	    if (shift != 1)
		bumplc();
	    outaccum();
	    outncimmadr((offset_T) shift);
	}
	return;
    }
#endif
    if ((uvalue_t) shift >= INT16BITSTO)	/* covers negatives too */
    {
	if ((bool_t) uflag)
	    clr(DREG);
	else			/* make D == 0 if D >= 0, else D == -1 */
	    smiDreg();		/* special case of 68020 Scc instruction */
    }
    else
    {
	if (shift >= CHBITSTO)
	{
	    tfrhilo();
	    if ((bool_t) uflag)
		ctoi();
	    else
		sctoi();
	    shift -= CHBITSTO;
	}
#ifdef I8088
# if MAX_INLINE_SHIFT < INT16BITSTO
	if (shift > MAX_INLINE_SHIFT)
	{
	    outload();
	    outregname(SHIFTREG);
	    outcomma();
	    outimmadr((offset_T) shift);
	    outnl();
	    if ((bool_t) uflag)
		outusr();
	    else
		outsr();
	    outaccum();
	    outncregname(SHIFTREG);
	}
	else
# endif
#endif
	    while (shift--)
	    {
		if ((bool_t) uflag)
		    usr1();
		else
		    sr1();
	    }
    }
}

/* extend an unsigned in DREG to a long */

PUBLIC void uitol(reg)
store_pt reg;
{
    if (lowregisDreg())
	clr(reg);
    else
    {
	regexchange(DREG, reg);
	clr(DREG);
    }
}

PRIVATE char opregstr[] = "_opreg";

/*-----------------------------------------------------------------------------
	opregadr()
	outputs address of variable opreg where OPREG is saved
-----------------------------------------------------------------------------*/

PRIVATE void opregadr()
{
#ifdef I8088
    outindleft();
    outccname(opregstr);
    outindright();
    bumplc2();
#ifdef I80386
    if (i386_32)
	bumplc2();
#endif
#endif
#ifdef MC6809
    outregname(OPREG);
    outtab();
    if (posindependent)
    {
	outccname(opregstr);
	outncregname(GLOBAL);
	bumplc();
    }
    else
    {
	outextended();
	outnccname(opregstr);
    }
    bumplc();
#endif
}

/*-----------------------------------------------------------------------------
	restoreopreg()
	restores register OPREG from static location >opreg if it is was use
-----------------------------------------------------------------------------*/

PUBLIC void restoreopreg()
{
    if (reguse & OPREG)
    {
#ifdef I8088
	outload();
	outregname(OPREG);
	outopsep();
	opregadr();
	outnl();
#endif
#ifdef MC6809
	outload();
	opregadr();
#endif
    }
}

/*-----------------------------------------------------------------------------
	saveopreg()
	saves register OPREG to static location >opreg if it is in use
	this makes the flop routines non-reentrant. It is too messy to
	push it because the flop routines leave results on the stack
-----------------------------------------------------------------------------*/

PUBLIC void saveopreg()
{
    if (reguse & OPREG)
    {
#ifdef I8088
	bssseg();
	common(opregstr);
	outnhex(opregsize);
	cseg();
	outstore();
	opregadr();
	outncregname(OPREG);
#endif
#ifdef MC6809
	dseg();
	common(opregstr);
	outnhex(opregsize);
	cseg();
	outstore();
	opregadr();
#endif
    }
}
