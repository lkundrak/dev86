/* extern functions */

/* as.c */
int main P((int argc, char **argv));
void as_abort P((char *message));
void finishup P((void));
void initp1p2 P((void));
void line_zero P((void));

/* assemble.c */
void assemble P((void));

/* express.c */
void absexpres P((void));
void chkabs P((void));
void nonimpexpres P((void));
void showrelbad P((void));
void symabsexpres P((void));
void symexpres P((void));
void expres P((void));
void factor P((void));
void scompare P((void));

/* genbin.c */
void binheader P((void));
void bintrailer P((void));
void genbin P((void));
void initbin P((void));
void putbin P((int ch));

/* genlist.c */
char *build_2hex_number P((unsigned num, char *where));
char *build_number P((unsigned num, unsigned width, char *where));
void warning P((char * errorstr));
void error P((char * errorstr));
void listline P((void));
void writec P((int ch));
void writenl P((void));
void writeoff P((offset_t offset));
void writes P((char *s));
void writesn P((char *s));
void writew P((unsigned word));

/* genobj.c */
void accumulate_rmb P((offset_t offset));
void flushobj P((void));
void genobj P((void));
void initobj P((void));
void objheader P((void));
void objtrailer P((void));
void putabs P((opcode_pt ch));
void putobj P((opcode_pt ch));

/* gensym.c */
void gensym P((void));

/* macro.c */
void entermac P((struct sym_s *symptr));
void pmacro P((void));

/* mops.c */
#ifdef I80386
void mbcc P((void));
void mbswap P((void));
void mcall P((void));
void mcalli P((void));
void mdivmul P((void));
void menter P((void));
void mEwGw P((void));
void mExGx P((void));
void mf_inher P((void));
void mf_m P((void));
void mf_m2 P((void));
void mf_m2_ax P((void));
void mf_m2_m4 P((void));
void mf_m2_m4_m8 P((void));
void mf_m4_m8_optst P((void));
void mf_m4_m8_st P((void));
void mf_m4_m8_stst P((void));
void mf_m4_m8_m10_st P((void));
void mf_m10 P((void));
void mf_optst P((void));
void mf_st P((void));
void mf_stst P((void));
void mf_w_inher P((void));
void mf_w_m P((void));
void mf_w_m2 P((void));
void mf_w_m2_ax P((void));
void mgroup1 P((void));
void mgroup2 P((void));
void mgroup6 P((void));
void mgroup7 P((void));
void mgroup8 P((void));
void mGvEv P((void));
void mGvMa P((void));
void mGvMp P((void));
void mimul P((void));
void min P((void));
void mincdec P((void));
void minher P((void));
void minher16 P((void));
void minher32 P((void));
void minhera P((void));
void mint P((void));
void mjcc P((void));
void mjcxz P((void));
void mlea P((void));
void mmov P((void));
void mmovx P((void));
void mnegnot P((void));
void mout P((void));
void mpushpop P((void));
void mret P((void));
void mseg P((void));
void msetcc P((void));
void mshdouble P((void));
void mtest P((void));
void mxchg P((void));
#endif				/* I80386 */

#ifdef MC6809
void mall P((void));
void malter P((void));
void mimmed P((void));
void mindex P((void));
void minher P((void));
void mlong P((void));
void msstak P((void));
void mswap P((void));
void mustak P((void));
#endif				/* MC6809 */

void getcomma P((void));
void mshort P((void));

/* pops.c */
bool_pt checksegrel P((struct sym_s *symptr));
void checkdatabounds P((void));
void datatoobig P((void));
void fatalerror P((char * errorstr));
void labelerror P((char * errorstr));
void palign P((void));
void pasciz P((void));
void pblkw P((void));
void pblock P((void));
void pbss P((void));
void pcomm P((void));
void pcomm1 P((void));
void pdata P((void));
void pelse P((void));
void pelseif P((void));
void pelsifc P((void));
void pendb P((void));
void pendif P((void));
void penter P((void));
void pentry P((void));
void pequ P((void));
void peven P((void));
void pexport P((void));
void pfail P((void));
void pfcb P((void));
void pfcc P((void));
void pfdb P((void));
#if SIZEOF_OFFSET_T > 2
void pfqb P((void));
#endif
void pglobl P((void));
void pident P((void));
void pif P((void));
void pifc P((void));
void pimport P((void));
void plcomm P((void));
void plcomm1 P((void));
void plist P((void));
void pnolist P((void));
void ploc P((void));
void pmaclist P((void));
void pmap P((void));
void porg P((void));
void prmb P((void));
void psect P((void));
void pset P((void));
void psetdp P((void));
void ptext P((void));
void puse16 P((void));
void puse32 P((void));
void pwarn P((void));
void showlabel P((void));

/* readsrc.c */
void initsource P((void));
fd_t open_input P((char *name));
void pget P((void));
void pproceof P((void));
void readline P((void));
void skipline P((void));

/* scan.c */
void context_hexconst P((void));
void getsym P((void));
void getsym_nolookup P((void));
void initscan P((void));

/* table.c */
void inst_keywords P((void));
struct sym_s *lookup P((void));
void statistics P((void));

/* type.c */
u2_pt c2u2 P((char *buf));
u4_t c4u4 P((char *buf));
u2_pt cnu2 P((char *buf, unsigned count));
u4_t cnu4 P((char *buf, unsigned count));
void u2c2 P((char *buf, u16_pt offset));
void u4c4 P((char *buf, u32_T offset));
void u2cn P((char *buf, u16_pt offset, unsigned count));
void u4cn P((char *buf, u32_T offset, unsigned count));
bool_pt typeconv_init P((bool_pt big_endian, bool_pt long_big_endian));

/* alloc.c */
void * asalloc P((unsigned int size));
void * asrealloc P((void * oldptr, unsigned int size));
void * temp_buf P((void));
void init_heap P((void));

