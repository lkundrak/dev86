/* table.h - table handler for bcc */

/* Copyright (C) 1992 Bruce Evans */

EXTERN char *charptr;		/* next free spot in catchall table */
EXTERN char *chartop;		/* spot after last in table */
EXTERN char *char1top;		/* last character spot in table */
EXTERN char *char3top;		/* third last character spot in table */
EXTERN struct symstruct *exprptr;
				/* next entry in expression symbol table */
EXTERN struct symstruct *locptr;
				/* next entry in local symbol table */
extern struct symstruct locsyms[];
				/* local symbol table */

#define TS1
#ifdef TS
uvalue_t ts_n_newtypelist;
uvalue_t ts_s_newtypelist;
uvalue_t ts_n_filename_term;
uvalue_t ts_s_filename_term;
uvalue_t ts_n_filename;
uvalue_t ts_s_filename;
uvalue_t ts_s_filename_tot;
uvalue_t ts_n_pathname;
uvalue_t ts_s_pathname;
uvalue_t ts_s_pathname_tot;
uvalue_t ts_n_inputbuf;
uvalue_t ts_s_inputbuf;
uvalue_t ts_s_inputbuf_tot;
uvalue_t ts_n_includelist;
uvalue_t ts_s_includelist;
uvalue_t ts_s_outputbuf;
uvalue_t ts_n_macstring_ident;
uvalue_t ts_n_macstring_ordinary;
uvalue_t ts_n_macstring_param;
uvalue_t ts_n_macstring_quoted;
uvalue_t ts_n_macstring_term;
uvalue_t ts_s_macstring;
uvalue_t ts_n_defines;
uvalue_t ts_s_defines;
uvalue_t ts_n_macparam;
uvalue_t ts_s_macparam;
uvalue_t ts_s_macparam_tot;
uvalue_t ts_n_macparam_string_ordinary;
uvalue_t ts_n_macparam_string_quoted;
uvalue_t ts_n_macparam_string_term;
uvalue_t ts_s_macparam_string;
uvalue_t ts_s_macparam_string_tot;
uvalue_t ts_s_macparam_string_alloced;
uvalue_t ts_s_macparam_string_alloced_tot;
uvalue_t ts_s_fakeline;
uvalue_t ts_s_fakeline_tot;
uvalue_t ts_n_string;
uvalue_t ts_n_case;
uvalue_t ts_n_case_realloc;
uvalue_t ts_s_case;
uvalue_t ts_s_case_tot;
uvalue_t ts_n_structname;
uvalue_t ts_s_structname;
uvalue_t ts_n_type;
uvalue_t ts_s_type;
uvalue_t ts_n_global;
uvalue_t ts_size_global;
uvalue_t ts_n_holdstr;
uvalue_t ts_size_holdstr;
uvalue_t ts_n_growobj;
uvalue_t ts_size_growobj_wasted;
uvalue_t ts_n_growheap;
uvalue_t ts_s_growheap;
#endif
