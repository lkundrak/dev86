/* reg.h - registers for bcc */

/* Copyright (C) 1992 Bruce Evans */

/*
  The compiler generates "addresses" of the form
      indirect(indcount) (rx + offset)
  where
      rx is a machine register (possibly null)
      n is the indirection count (possibly 0)
      offset is a constant.
  It does not support more complicated formats like
      indirect(indcount) (rx + index * scale + offset).

      The register is coded as bit flag in the  storage  component of
  the symbol structure. This allows groups of registers to be tested
  using bitwise "&". Throughout the compiler, the group of these bit
  flags has the type  reg_t. If there are only a few registers, reg_t
  can be an unsigned char. It must be unsigned if the high bit is
  used, to avoid sign extension problems. For bootstrapping the compiler
  from a compiler with no unsigned char, the unsigned type should be
  used instead (with a signifigant waste of storage).

      The bit flags should really be defined as ((reg_t) whatever) but
  then they would not (necessarily) be constant expressions and couldn't
  be used in switch selectors or (worse) preprocessor expressions.

      The CONSTANT and GLOBAL (non-) register bits are almost
  equivalent. A constant with nonzero indirection is marked as a
  GLOBAL and not a CONSTANT. This makes it easier to test for a constant
  CONSTANT. Globals which are formed in this way are converted to
  constants if their indirection count is reset to 0 (by & operator).
*/

/* register bit flags */

#define NOSTORAGE 0x000		/* structure/union member offsets */
#define CONSTANT  0x001		/* offsets are values */
#define BREG      0x002
#define DREG      0x004
#define INDREG0   0x008
#define INDREG1   0x010
#define INDREG2   0x020
#define LOCAL     0x040
#define GLOBAL    0x080		/* offsets from storage name or 0 */
#define CCREG     CONSTANT	/* arg to PSHS/PULS functions only */
#ifdef I8088
# ifdef FRAMEPOINTER
#  define FRAMEREG LOCAL
# endif
# define STACKREG 0x100
# define DATREG1  0x200
# define DATREG2  0x400
# define DATREG1B 0x800
#endif
#ifdef MC6809
# define DPREG    LOCAL		/* arg to PSHS/PULS functions only */
# define PCREG    GLOBAL	/* arg to PSHS/PULS functions only */
#endif

/* data for pushing and pulling registers */

#define MINREGCHAR 'A'
#ifdef I8088
# define FLAGSREGCHAR 'f'
# define pushchar() pushlist(AXREG)
#endif
#ifdef MC6809
# define pushchar() pushlist(BREG)
#endif

/* special registers */

#ifdef I8088
# define ALREG    BREG
# define AXREG    DREG
# define DXREG    DATREG2
# define MULREG   DATREG1B
# define SHIFTREG DATREG1B
#endif
#ifdef MC6809
# define XREG INDREG0		/* XREG is special for ABX in index & switch */
# define YREG INDREG2		/* XREG and YREG allow LEA (Z test) in cmp() */
#endif

/* groups of registers */

#define ALLDATREGS (BREG|DREG)
#define CHARREGS BREG
#define MAXREGS 1		/* number of data registers */
#define WORKDATREGS (BREG|DREG)

/* function call and return registers */

#define ARGREG RETURNREG	/* for (1st) argument */
#define LONGARGREGS LONGRETURNREGS	/* for long or float arg */
#define LONGRETURNREGS (INDREG0|LONGREG2)
#define LONGREG2 DREG
#ifdef I8088
# define LONGRETSPECIAL	/* LONGRETURNREGS!=RETURNREG && RETURNREG==LONGREG2 */
# define RETURNREG DREG
#endif
#ifdef MC6809
# define RETURNREG INDREG0
#endif

/* registers which can be pulled as a group with the program counter */
/* to perform an efficient function return */

#ifdef MC6809
#define JUNK1REGS BREG		/* 1 bytes locals to discard */
#define JUNK2REGS INDREG2
#define JUNK3REGS (BREG|INDREG2)
#define JUNK4REGS (INDREG1|INDREG2)
#endif

/* registers which can be pushed as a group with the first argument */
/* to perform an efficient function startup */

#ifdef MC6809
# define LOC1REGS CCREG		/* 1 bytes local to allocate */
# define LOC2REGS DREG
# define LOC3REGS (CCREG|DREG)
# define LOC4REGS (CCREG|DREG|DPREG)
# endif

/* registers to be used by software operations */

#define OPREG INDREG0		/* 2nd reg for software ops (1st is DREG) */
#define OPWORKREG INDREG2	/* 3rd register for software ops */

/* maximum indirection count for 1 instruction */

#ifdef I8088
# define MAXINDIRECT 1
#endif
#ifdef MC6809
# define MAXINDIRECT 2
#endif
