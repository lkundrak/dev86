/* proto.h - extern functions for bcc */

/* Copyright (C) 1992 Bruce Evans */

/* assign.c */
void assign P((struct symstruct *source, struct symstruct *target));
void cast P((struct typestruct *type, struct symstruct *target));
void extend P((struct symstruct *target));

/* codefrag.c */
void adjsp P((label_no label));
void clrBreg P((void));
void comment P((void));
void ctoi P((void));
void defbyte P((void));
void deflong P((uoffset_T value));
void defword P((void));
void defdword P((void));
void even P((void));
void negDreg P((void));
void comDreg P((void));
void outadd P((void));
void outaddsp P((void));
void outcalladr P((void));
void outcmp P((void));
void outdec P((void));
void outdword P((void));
void outfail P((void));
void outinc P((void));
void outindleft P((void));
void outindright P((void));
void outindstackreg P((void));
void outldaccum P((void));
void outldmulreg P((void));
void outlea P((void));
void outleasp P((void));
void outload P((void));
void outmulmulreg P((void));
void outopsep P((void));
void outpshs P((void));
void outpuls P((void));
void outreturn P((void));
void outstore P((void));
void outsub P((void));
void outtest P((void));
void outword P((void));
void sctoi P((void));
void stoi P((void));
void ustoi P((void));
void outABX P((void));
void outdirectpage P((void));
void outextended P((void));
void outncspregname P((void));
void outindframereg P((void));
void adc0 P((void));
void addconst P((offset_T offset, store_pt reg));
void adjlc P((offset_T offset, store_pt reg));
void andconst P((offset_T offset));
void bssseg P((void));
label_no casejump P((void));
void common P((char *name));
void cseg P((void));
void defnulls P((uoffset_T nullcount));
label_no defstr P((char *sptr, char *stop, bool_pt dataflag));
bool_pt diveasy P((value_t divisor, bool_pt uflag));
void dpseg P((void));
void dseg P((void));
void equ P((char *name, char *string));
void equlab P((label_no label, offset_T offset));
void globl P((char *name));
void import P((char *name));
void itol P((store_pt reg));
void lcommlab P((label_no label));
void lcommon P((char *name));
void lea P((offset_T offset, store_pt sourcereg, store_pt targreg));
void loadconst P((offset_T offset, store_pt reg));
int lslconst P((value_t shift, store_pt reg));
int lsrconst P((value_t shift, store_pt reg, bool_pt uflag));
bool_pt modeasy P((value_t divisor, bool_pt uflag));
bool_pt muleasy P((uvalue_t factor, store_pt reg));
void negreg P((store_pt reg));
char *opstring P((op_pt op));
void outccname P((char *name));
void outhiaccum P((void));
void outimmadr P((offset_T offset));
void outimadj P((offset_T offset, store_pt targreg));
void outimmed P((void));
void outjumpstring P((void));
void outnccname P((char *name));
void outncimmadr P((offset_T offset));
void outoffset P((offset_T offset));
void public P((char *name));
void private P((char *name));
void regexchange P((store_pt sourcereg, store_pt targreg));
void regtransfer P((store_pt sourcereg, store_pt targreg));
void sbc0 P((void));
void set P((char *name, offset_T value));
void sl1 P((store_pt reg));
void slconst P((value_t shift, store_pt reg));
void srconst P((value_t shift, bool_pt uflag));
void uitol P((store_pt reg));
void restoreopreg P((void));
void saveopreg P((void));

/* dbnode.c */
void dbitem P((struct symstruct *item));
void dbtype P((struct typestruct *type));
void dbnode P((struct nodestruct *exp));
void dbnodeswap P((void));

/* declare.c */
void colon P((void));
void decllist P((void));
void lparen P((void));
void needvarname P((void));
void program P((void));
void rbrace P((void));
void rbracket P((void));
void rparen P((void));
void semicolon P((void));
struct typestruct *typename P((void));

/* express.c */
struct nodestruct *assignment_exp P((void));
struct nodestruct *expression P((void));

/* exptree.c */
struct nodestruct *castnode P((struct typestruct *type,
			       struct nodestruct *nodeptr));
void etreeinit P((void));
struct nodestruct *leafnode P((struct symstruct *source));
struct nodestruct *node P((op_pt t, struct nodestruct *p1,
			   struct nodestruct *p2));

/* floatop.c */
bool_pt f_indirect P((struct symstruct *target));
void float1op P((op_pt op, struct symstruct *source));
void floatop P((op_pt op, struct symstruct *source, struct symstruct *target));
void fpush P((struct symstruct *source));
void justpushed P((struct symstruct *target));

/* function.c */
void call P((char *name));
void function P((struct symstruct *source));
void ldregargs P((void));
void loadretexpression P((void));
void listo P((struct symstruct *target, offset_T lastargsp));
void listroot P((struct symstruct *target));
void popframe P((void));
void reslocals P((void));
void ret P((void));

/* gencode.c */
void bileaf P((struct nodestruct *exp));
fastin_pt bitcount P((uvalue_t number));
void codeinit P((void));
fastin_pt highbit P((uvalue_t number));
void makeleaf P((struct nodestruct *exp));

/* genloads.c */
void addoffset P((struct symstruct *source));
void address P((struct symstruct *source));
void exchange P((struct symstruct *source, struct symstruct *target));
store_pt getindexreg P((void));
void indexadr P((struct symstruct *source, struct symstruct *target));
void indirec P((struct symstruct *source));
void load P((struct symstruct *source, store_pt targreg));
void loadany P((struct symstruct *source));
void loadreg P((struct symstruct *source, store_pt targreg));
void makelessindirect P((struct symstruct *source));
void movereg P((struct symstruct *source, store_pt targreg));
void onstack P((struct symstruct *target));
void outadr P((struct symstruct *adr));
void outcregname P((store_pt reg));
void outncregname P((store_pt reg));
void outnregname P((store_pt reg));
void outregname P((store_pt reg));
void outshortregname P((store_pt reg));
void pointat P((struct symstruct *target));
void poplist P((store_pt reglist));
void push P((struct symstruct *source));
void pushlist P((store_pt reglist));
void pushreg P((store_pt reg));
void storereg P((store_pt sourcereg, struct symstruct *target));
void struc P((struct symstruct *source, struct symstruct *target));
void transfer P((struct symstruct *source, store_pt targreg));

/* glogcode.c */
void cmp P((struct symstruct *source, struct symstruct *target,
	    ccode_t *pcondtrue));
void condop P((struct nodestruct *exp));
void jumpfalse P((struct nodestruct *exp, label_no label));
void jumptrue P((struct nodestruct *exp, label_no label));
void logop P((struct nodestruct *exp));

/* hardop.c */
void add P((struct symstruct *source, struct symstruct *target));
void incdec P((op_pt op, struct symstruct *source));
void neg P((struct symstruct *target));
void not P((struct symstruct *target));
void op1 P((op_pt op, struct symstruct *source, struct symstruct *target));
void ptrsub P((struct symstruct *source, struct symstruct *target));
void sub P((struct symstruct *source, struct symstruct *target));

/* input.c */
void closein P((void));
void errorloc P((void));
void gch1 P((void));
void include P((void));
void openio P((int argc, char **argv));
void skipeol P((void));
void specialchar P((void));
void linecontol P((void));

/* label.c */
void bumplc P((void));
void bumplc2 P((void));
void bumplc3 P((void));
void clearfunclabels P((void));
void clearlabels P((char *patchbuf, char *patchtop));
void clearswitchlabels P((void));
uoffset_T getlc P((void));
void deflabel P((label_no label));
label_no gethighlabel P((void));
label_no getlabel P((void));
void jump P((label_no label));
void lbranch P((ccode_pt cond, label_no label));
struct symstruct *namedlabel P((void));
void outcond P((ccode_pt cond));
void outlabel P((label_no label));
void outnlabel P((label_no label));
void sbranch P((ccode_pt cond, label_no label));
void unbumplc P((void));

/* loadexp.c */
value_t constexpression P((void));
void initexpression P((struct typestruct *type));
struct typestruct *loadexpression P((store_pt targreg,
				     struct typestruct *targtype));

/* longop.c */
void longop P((op_pt op, struct symstruct *source, struct symstruct *target));
void long1op P((op_pt op, struct symstruct *target));
void outlongendian P((void));

/* output.c */
void bugerror P((char *message));
void closeout P((void));
void error P((char *message));
void error2error P((char *message1, char *message2));
void fatalerror P((char *message));
void finishup P((void));
void flushout P((void));
void limiterror P((char *message));
void initout P((void));
void openout P((char *oname));
void outbyte P((int ch));
void outcomma P((void));
void outcpplinenumber P((unsigned nr, char *fname, char *str));
void outhex P((uoffset_T num));
void outhexdigs P((uoffset_T num));
void outline P((char *s));
void outminus P((void));
void outnl P((void));
void outnbyte P((int byte));
void outnhex P((uoffset_T num));
void outnop1str P((char *s));
void outnop2str P((char *s));
void outnstr P((char *s));
void outop0str P((char *s));
void outop1str P((char *s));
void outop2str P((char *s));
void outop3str P((char *s));
void outplus P((void));
void outshex P((offset_T num));
void outstr P((char *s));
void outtab P((void));
void outudec P((unsigned num));
#ifndef I8088
void outuvalue P((uvalue_t num));
void outvalue P((value_t num));
#endif
char *pushudec P((char *s, unsigned num));
void setoutbufs P((void));

/* preproc.c */
void blanks P((void));
bool_pt blanksident P((void));
void checknotinif P((void));
void define P((void));
void definestring P((char *str));
void docontrol P((void));
void entermac P((void));
void ifinit P((void));
int  ifcheck P((void));
void leavemac P((void));
void predefine P((void));
char *savedlineptr P((void));
void skipcomment P((void));
void skipline P((void));
void undefinestring P((char *str));

/* preserve.c */
void changesp P((offset_T newsp, bool_pt absflag));
void loadpres P((struct symstruct *source, struct symstruct *target));
void modstk P((offset_T newsp));
void pres2 P((struct symstruct *source, struct symstruct *target));
void preserve P((struct symstruct *source));
store_pt preslval P((struct symstruct *source, struct symstruct *target));
void recovlist P((store_pt reglist));
void savereturn P((store_pt savelist, offset_T saveoffset));

/* sc.c */
int main P((int argc, char **argv));

/* scan.c */
void cppscan P((int asmonly));
void eofin P((char *message));
bool_pt isident P((void));
void nextsym P((void));
void stringorcharconst P((void));

/* softop.c */
void softop P((op_pt op, struct symstruct *source, struct symstruct *target));

/* state.c */
void compound P((void));
void outswoffset P((offset_T offset));
void outswstacklab P((void));
void doasm P((void));

/* table.c */
struct symstruct *addglb P((char *name, struct typestruct *type));
struct symstruct *addloc P((char *name, struct typestruct *type));
struct symstruct *addlorg P((char *name, struct typestruct *type));
void addsym P((char *name, struct typestruct *type,
	       struct symstruct *symptr));
struct symstruct *constsym P((value_t intconst));
void delsym P((struct symstruct *symptr));
void dumpglbs P((void));
void dumplocs P((void));
void dumpstrings P((void));
struct symstruct *exprsym P((struct symstruct *symptr));
struct symstruct *findlorg P((char *name));
struct symstruct *findstruct P((char *name));
struct symstruct **gethashptr P((char *sname));
void growheap P((unsigned size));
void *growobject P((void *object, unsigned extra));
label_no holdstr P((char *sptr, char *stop));
void newlevel P((void));
void oldlevel P((void));
void ourfree P((void *ptr));
void *ourmalloc P((unsigned nbytes));
void outofmemoryerror P((char *message));
void *qmalloc P((unsigned size));
void swapsym P((struct symstruct *sym1, struct symstruct *sym2));
void syminit P((void));

/* type.c */
struct typestruct *addstruct P((char *structname));
struct typestruct *iscalartotype P((scalar_pt scalar));
struct typestruct *newtype P((void));
void outntypechar P((struct typestruct *type));
struct typestruct *pointype P((struct typestruct *type));
struct typestruct *prefix P((constr_pt constructor, uoffset_T size,
			     struct typestruct *type));
struct typestruct *promote P((struct typestruct *type));
struct typestruct *tosigned P((struct typestruct *type));
struct typestruct *tounsigned P((struct typestruct *type));
void typeinit P((void));
