
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#ifdef __STDC__
#include <stdlib.h>
#endif
#include "cc.h"

#define CPP_DEBUG 0	/* LOTS of junk to stdout. */

/*
 * This file comprises the 'guts' of a C preprocessor.
 *
 * Functions exported from this file:
 *   gettok() Returns the next token from the source
 *            curword contains the text of the token
 *
 * Variables
 *   curword  Contains the text of the last token parsed.
 *   curfile  Currently open primary file
 *   c_fname  Name of file being parsed
 *   c_lineno Current line number in file being parsed.
 *
 *   in_asm   Control flag for the kind of tokens you want (C or assembler)
 *   ansi_c   Control flag to change the preprocessor for Ansi C.
 *
 */

char curword[WORDSIZE];
int  in_asm = 0;
int  ansi_c = 0;

FILE * curfile;
char * c_fname;
int    c_lineno = 0;

typedef int int_type;		/* Used for preprocessor expressions */
static int  curtok = 0;		/* Used for preprocessor expressions */

static int    fi_count = 0;
static FILE * saved_files[MAX_INCLUDE];
static char * saved_fname[MAX_INCLUDE];
static int    saved_lines[MAX_INCLUDE];

static char * def_ptr = 0;
static char * def_start = 0;
static struct define_item * def_ref = 0;

static int    def_count =0;
static char * saved_def[MAX_DEFINE];
static char * saved_start[MAX_DEFINE];
static long   saved_unputc[MAX_DEFINE];
static struct define_item * saved_ref[MAX_DEFINE];
 
static long unputc = 0;

static int last_char = '\n';
static int in_preproc = 0;
static int dont_subst = 0;
static int quoted_str = 0;

static int if_count = 0;
static int if_false = 0;
static int if_has_else = 0;
static int if_hidden = 0;
static int if_stack = 0;

struct arg_store {
   char * name;
   char * value;
   int  in_define;
};

static int  chget _P((void));
static void unchget _P((int));
static int  gettok_nosub _P((void));
static int  get_onetok _P((void));
static int  pgetc _P((void));
static int  do_preproc _P((void));
static int  do_proc_copy_hashline _P((void));
static int  do_proc_if _P((int));
static void do_proc_include _P((void));
static void do_proc_define _P((void));
static void do_proc_undef _P((void));
static void do_proc_else _P((void));
static void do_proc_endif _P((void));
static void do_proc_tail _P((void));
static int  get_if_expression _P((void));
static int_type get_expression _P((int));
static int_type get_exp_value _P((void));
static int  gen_substrings _P((char *, char *, int));
static char * insert_substrings _P((char *, struct arg_store *, int));

int
gettok()
{
   int ch;

   for(;;) 
   {
      /* Normal C Preprocessor decoding */
      if (in_asm) {
	 *curword = 0;
	 if (quoted_str) ch = chget();
	 else            ch = pgetc();
	 if (ch > 0xFF || ch == EOF) return ch;

	 if(!quoted_str && 
	       ( (ch >= 'A' && ch <= 'Z')
	       || (ch >= 'a' && ch <= 'z')
	       || ch == '_' || ch == '$' ) ) {
	    unchget(ch);
	 }
	 else
	 {
	    curword[0] = ch;
	    curword[1] = 0;
	    if (quoted_str==2)
	       quoted_str=1;
	    else {
	       if (ch == '"') {
		  quoted_str = !quoted_str;
		  return '"';
	       }
	       if (quoted_str == 1 && ch == '\\')
		  quoted_str = 2;
	       else if (quoted_str && ch == '\n') {
		  quoted_str = 0;
		  unchget(ch);
		  return '\n';
	       }
	    }

	    if (ch == '\n') continue;
	    return TK_STR;
	 }
      }

      /* Tokenised C-Preprocessing */
      if (!quoted_str)
      {
	 ch = get_onetok();

	 /* Erase meaningless backslashes */
	 if( ch == '\\' ) {
	    int ch1 = chget();
	    if (ch1 == '\n') continue;
	    unchget(ch1);
	 }

	 if( ch == '"' )
	    quoted_str = 1;
	 return ch;
      }

      /* Special for quoted strings */
      *curword = 0;
      ch = chget();
      if( ch == EOF ) return ch;

      *curword = ch;
      curword[1] = '\0';

      if( quoted_str == 2 ) {
	 quoted_str = 1;
	 return TK_STR;
      }

      if( ch == '"' )
      {
	 /* Found a terminator '"' check for ansi continuation */
	 while( (ch = pgetc()) <= ' ' && ch != EOF) ;
	 if( ch == '"' ) continue;
	 quoted_str = 0;
	 unchget(ch);

	 *curword = '"';
	 curword[1] = '\0';
	 return '"';
      }
      if( ch == '\n' ) {
	 quoted_str = 0;
	 unchget(ch);
	 return ch;
      }
      if( ch != '\\' ) return TK_STR;

      /* Deal with backslash; NB this could interpret all the \X codes */
      ch = chget();

      if( ch != '\n' ) { quoted_str++; unchget(ch); return TK_STR; }
   }
}

static int
gettok_nosub()
{ int rv; dont_subst++; rv=get_onetok(); dont_subst--; return rv; }

static int
get_onetok()
{
   char * p;
   int state;
   int ch, cc;

Try_again:
   *(p=curword) = '\0';
   state=cc=ch=0;

   while( (ch = pgetc()) != EOF)
   {
      if( ch > ' ' ) break;
      if( in_preproc && ch == '\n') break;
      if( (!in_preproc) && in_asm && (ch == ' ' || ch == '\t') ) break;
   }
   if( ch > 0xFF ) return ch;
   for(;;)
   {
      switch(state)
      {
      case 0: if( (ch >= 'A' && ch <= 'Z')
               || (ch >= 'a' && ch <= 'z')
               || ch == '_' || ch == '$' )
                 state = 1;
              else if(ch == '0')
                 state = 2;
              else if(ch >= '1' && ch <= '9')
                 state = 5;
              else
                 goto break_break;
              break;
      case 1: if( (ch >= '0' && ch <= '9')
               || (ch >= 'A' && ch <= 'Z')
               || (ch >= 'a' && ch <= 'z')
               || ch == '_' || ch == '$' )
                 break;
              else
                 goto break_break;
      case 2: if( ch >= '0' && ch <= '7')
                 state = 3;
              else if( ch == 'x' || ch == 'X' )
                 state = 4;
              else
                 goto break_break;
              break;
      case 3: if( ch >= '0' && ch <= '7')
                 break;
              else
                 goto break_break;
      case 4: if( (ch >= '0' && ch <= '9')
               || (ch >= 'A' && ch <= 'F')
               || (ch >= 'a' && ch <= 'f') )
                 break;
              else
                 goto break_break;
      case 5:
      case 6: if( ch >= '0' && ch <= '9')
                 ;
              else if( ch == '.' && state != 6 )
                 state = 6;
              else if( ch == 'e' || ch == 'E' )
                 state = 7;
              else
                 goto break_break;
              break;
      case 7: if( ch == '+' || ch == '-' )
                 break;
              state = 8;
              /* FALLTHROUGH */
      case 8: if( ch >= '0' && ch <= '9')
                 break;
              else
                 goto break_break;
      }
      if( cc < WORDSIZE-1 ) *p++ = ch;	/* Clip to WORDSIZE */
      *p = '\0'; cc++;
      ch = pgetc();
   }
break_break:
   if( state == 1 )
   {
      struct define_item * ptr;
      unchget(ch);
      if( !dont_subst
         && (ptr = read_entry(0, curword)) != 0
	 && !ptr->in_use
	 )
      {
	 if ( def_count >= MAX_DEFINE ) {
	    cerror("Preprocessor recursion overflow");
	    goto oops_keep_going;
	 } else
         /* Add in the arguments if they're there */
	 if( ptr->arg_count >= 0 )
	 {
	    /* We have arguments to process so lets do so. */
	    if( !gen_substrings(ptr->name, ptr->value, ptr->arg_count) )
	       goto oops_keep_going;
	    def_ref = ptr;
	    ptr->in_use = 1;
	 }
	 else if (ptr->value[0])
	 {
            saved_ref[def_count] = def_ref;
            saved_def[def_count] = def_ptr;
            saved_start[def_count] = def_start;
            saved_unputc[def_count] = unputc;
            def_count++;
            unputc = 0;
	    def_ref = ptr;
            def_ptr = ptr->value;
	    def_start = 0;
	    ptr->in_use = 1;
	 }
         goto Try_again;
      }
oops_keep_going:
      if( !in_preproc )
      {
         struct token_trans *p;
	 if( p=is_ckey(curword, cc) )
	    return p->token;
      }
      return TK_WORD;
   }
   if( state >= 2 )
   {
      if( state < 6 )
      {
         if( ch == 'l' || ch == 'L' )
         {
            if( cc < WORDSIZE-1 ) *p++ = ch;	/* Clip to WORDSIZE */
            *p = '\0'; cc++;
         }
         else unchget(ch);
         return TK_NUM;
      }
      unchget(ch);
      return TK_FLT;
   }

   /* More tokeniser for C like stuff */
   if(!in_asm || in_preproc)
   {
      /* Quoted char (NOT strings!) */
      if (ch == '\'' )
      {
	 *p++ = ch; ch = chget();
	 for(;;)
	 {
	    if( cc < WORDSIZE-1 ) *p++ = ch;	/* Clip to WORDSIZE */
	    *p = '\0'; cc++;
	    if( ch == '\'' || ch == '\n' ) break;

	    if( ch == '\\' )
	    {
	       ch = chget();
	       if( cc < WORDSIZE-1 ) *p++ = ch;	/* Clip to WORDSIZE */
	       *p = '\0'; cc++;
	    }
	    ch = chget();
	 }
	 ch = TK_QUOT;
      }
      /* Composite tokens */
      if( ch > ' ' && ch <= '~' )
      {
	 struct token_trans *p;
	 *curword = cc = ch;

	 for(state=1; ; state++)
	 {
	    curword[state] = ch = chget();
	    if( !(p=is_ctok(curword, state+1)) )
	    {
	       unchget(ch);
	       curword[state] = '\0';
	       return cc;
	    }
	    cc=p->token;
	 }
      }
   } else {
      *curword = ch;
      curword[1] = 0;
   }
   return ch;
}

static int
pgetc()
{
   int ch, ch1;

   for(;;)
   {
      /* This loop is repeated if the current char is not suitable
	 either because of a comment or a false #if condition
       */

      ch = chget();

      if( ch == EOF ) return ch;
      if( in_preproc && ch == '\\' )
      {
	 ch1 = chget();
	 if( ch1 == '\n' ) ch = chget();
	 else              unchget(ch1);
      }

      /* Ansi trigraphs -- Ewww, anyway this doesn't work, it needs lots
       * of 'unchget' space too. */
      if (ansi_c && ch == '?') {
	 ch1 = chget();
	 if (ch1 != '?')
	    unchget(ch1);
	 else {
	    static char trig1[] = "()<>/!'-=";
	    static char trig2[] = "[]{}\\|^~#";
	    char * s;
	    ch1 = chget();
	    s = strchr(trig1, ch1);
	    if (s) ch = trig2[s-trig1];
	    else {
	       unchget(ch1);
	       unchget('?');
	    }
	 }
      }

      if( !in_preproc && last_char == '\n' && ch == '#' )
      {
	 in_preproc = 1;
	 ch = do_preproc();
	 in_preproc = 0;
	 last_char = '\n';
	 if(if_false) continue;
	 return ch;
      }
      if( last_char != '\n' || (ch != ' ' && ch != '\t') )
	 last_char = ch;

      /* Remove comments ... */
      if( ch != '/' )
         { if(if_false && !in_preproc) continue; return ch; }
      ch1 = chget();

      if( ch1 == '/' ) /* Double slash style comments */
      {
	 do
	 {
	    ch = chget();
	    if( ch == EOF ) return EOF;
	    if( ch == '\\' ) {
	       ch1 = chget();
	       if( ch1 == EOF ) return EOF;
	    }
	 }
	 while(ch != '\n');
	 continue;
      }

      if( ch1 != '*' )
      {
	 unchget(ch1);
	 if(if_false && !in_preproc) continue;
	 return ch;
      }

      for(;;)
      {
	 if( ch == '*' )
	 {
	    ch = chget();
	    if( ch == EOF ) return EOF;
	    if( ch == '/' ) break;
	 }
	 else ch = chget();
      }
      if (ansi_c)
	 return ' ';	/* If comments become " " */

      /* Comments become nulls */
   }
}

static void 
unchget(ch)
{
#if CPP_DEBUG
   fprintf(stderr, "\b", ch);
#endif
   if(ch == EOF) ch=26; /* EOF is pushed back as ^Z */
   ch &= 0xFF;

   if(unputc&0xFF000000) 
      cerror("Internal character pushback stack overflow");
   else       unputc = (unputc<<8) + (ch);
   if( ch == '\n' ) c_lineno--;
}

static int 
chget()
#if CPP_DEBUG
{
   int ch;
static int last_def = 0;
static int last_fi = 0;
   if (last_fi != fi_count)   fprintf(stderr, "<INC%d>", fi_count);
   if (last_def != def_count) fprintf(stderr, "<DEF%d>", def_count);
   last_def = def_count; last_fi = fi_count;

   ch = realchget();
   if (ch == EOF) fprintf(stderr, "<EOF>"); else fprintf(stderr, "%c", ch);

   if (last_def != def_count) fprintf(stderr, "<DEF%d>", def_count);
   if (last_fi != fi_count)   fprintf(stderr, "<INC%d>", fi_count);
   last_def = def_count; last_fi = fi_count;

   return ch;
}

static int 
realchget()
#endif
{
   int ch;
   for(;;)
   {
      if( unputc )
      {
	 if((unputc&0xFF)==26 && in_preproc) return '\n';
	 ch=(unputc&0xFF); unputc>>=8;
	 if( ch == 26 ) ch = EOF;
	 if( ch == '\n' ) c_lineno++;
	 return ch;
      }

      if( def_ptr )
      {
	 ch = *def_ptr++; if(ch) return (unsigned char)ch;
	 if( def_start ) free(def_start);
	 if( def_ref ) def_ref->in_use = 0;

	 def_count--;
	 def_ref   = saved_ref[def_count];
	 def_ptr   = saved_def[def_count];
	 def_start = saved_start[def_count];
	 unputc    = saved_unputc[def_count];
	 continue;
      }

      ch = getc(curfile);
      if( ch == EOF && fi_count != 0)
      {
	 fclose(curfile);
	 fi_count--;
	 curfile  = saved_files[fi_count];
	 if(c_fname) free(c_fname);
	 c_fname  = saved_fname[fi_count];
	 c_lineno = saved_lines[fi_count];
	 ch = '\n'; /* Ensure end of line on end of file */
      }
      else if( ch == '\n' ) c_lineno++;

      /* Treat all control characters, except the standard whitespace
       * characters of TAB and NL as completely invisible. 
       */
      if( ch >= 0 && ch < ' ' && ch!='\n' && ch!='\t' && ch!=EOF ) continue;

      if( ch == EOF ) { unchget(ch); return '\n'; } /* Ensure EOL before EOF */
      return (unsigned char)ch;
   }
}

static int
do_preproc()
{
   int val, no_match=0;

   if( (val=get_onetok()) == TK_WORD )
   {
      if( strcmp(curword, "ifdef") == 0 )
         do_proc_if(0);
      else if( strcmp(curword, "ifndef") == 0 )
         do_proc_if(1);
      else if( strcmp(curword, "if") == 0 )
         do_proc_if(2);
      else if( strcmp(curword, "elif") == 0 )
         do_proc_if(3);
      else if( strcmp(curword, "else") == 0 )
         do_proc_else();
      else if( strcmp(curword, "endif") == 0 )
         do_proc_endif();
      else if(if_false)
         no_match=1;
      else
      {
         if( strcmp(curword, "include") == 0 )
            do_proc_include();
         else if( strcmp(curword, "define") == 0 )
            do_proc_define();
         else if( strcmp(curword, "undef") == 0 )
            do_proc_undef();
         else if( strcmp(curword, "error") == 0 )
            return do_proc_copy_hashline();
         else if( strcmp(curword, "warning") == 0 )
            return do_proc_copy_hashline();
         else if( strcmp(curword, "asm") == 0 ) {
	    in_asm |= 1;
            return do_proc_copy_hashline();
	 } else if( strcmp(curword, "endasm") == 0 ) {
	    in_asm &= ~1;
            return do_proc_copy_hashline();
	 } else 
	    no_match=1;
      }
   }
   else if( val == '#' )  /* This is a comment, it's used as a marker */
                          /* for compiler specific header files */
   {
      while((val = pgetc()) != '\n');
   }
   else no_match=1;
   
   if( no_match )
   {
      if(!if_false) cerror("Unknown preprocessor directive");
      while( val != '\n' ) val = pgetc();
   }

   strcpy(curword, "\n");
   return '\n';
}

static int
do_proc_copy_hashline()
{
   int off, ch;

   off = strlen(curword);

   while( (ch=pgetc()) != '\n' )
   {
      if( off < WORDSIZE ) curword[off++] = ch;
   }
   if( off == WORDSIZE )
   {
      cerror("Preprocessor directive too long");
      curword[WORDSIZE-1] = '\0';
   }
   else
      curword[off] = '\0';

   unchget('\n');
   return TK_COPY;
}

static void do_proc_include()
{
   int ch, ch1;
   char * p;
   FILE * fd;

   ch = get_onetok();
   if( ch == '<' || ch == '"' )
   {
      if( ch == '"' ) ch1 = ch; else ch1 = '>';
      p = curword;
      while(p< curword+sizeof(curword)-1)
      {
         ch = pgetc();
         if( ch == '\n' ) break;
         if( ch == ch1 )
         {
            *p = '\0';

            fd = open_include(curword, "r", (ch=='"'));
            if( fd == 0 )
               cerror("Cannot open include file");
	    do { ch = pgetc(); } while(ch == ' ' || ch == '\t'); unchget(ch);
            do_proc_tail();

            if( fd )
            {
               saved_files[fi_count] = curfile;
               saved_fname[fi_count] = c_fname;
               saved_lines[fi_count] = c_lineno;
               fi_count++;

               curfile = fd;
               c_fname = malloc(strlen(curword)+1);
               if( c_fname == 0 ) cfatal("Preprocessor out of memory");
               strcpy(c_fname, curword);
               c_lineno = 1;
            }
            return;
         }
         *p++ = ch;
      }
   }
   cerror("Bad #include command");
   while(ch != '\n') ch = pgetc();
   return;
}

static void do_proc_define()
{
   int ch, ch1;
   struct define_item * ptr, * old_value = 0;
   int  cc, len;
   char name[sizeof(curword)];

   if( (ch=gettok_nosub()) == TK_WORD )
   {
      strcpy(name, curword);
      ptr = read_entry(0, name);
      if(ptr)
      {
         set_entry(0, name, (void*)0); /* Unset var */
         if (ptr->in_use) 
	    ; /* Eeeek! This shouldn't happen; so just let it leak. */
	 else
	    old_value = ptr;
      }
      for(ch=ch1=pgetc(); ch == ' ' || ch == '\t' ; ch=pgetc()) ;

      /* If #define with no substitute */
      if( ch == '\n' )
      {
#if CPP_DEBUG
	 fprintf(stderr, "\n### Define '%s' as null\n", name);
#endif
         ptr = malloc(sizeof(struct define_item));
         if(ptr==0) cfatal("Preprocessor out of memory");
         ptr->name = set_entry(0, name, ptr);
         ptr->value[0] = '\0';
	 ptr->arg_count = -1;
	 ptr->in_use = 0;
         ptr->next = 0;
	 if (old_value) {
	    if (strcmp(old_value->value, ptr->value) != 0)
	       cwarn("#define redefined macro");
	    free(old_value);
	 }
         return;
      }
      len = WORDSIZE; 
      ptr = malloc(sizeof(struct define_item) + WORDSIZE);
      if(ptr==0) cfatal("Preprocessor out of memory");
      ptr->value[cc=0] = 0;

      /* Add in arguments */
      if( ch1 == '(' )
      {
         ptr->arg_count=0;
	 for(;;)
	 {
	    ch=gettok_nosub();
	    if( ptr->arg_count==0 && ch == ')' ) break;
	    if( ch == TK_WORD ) 
	    {
	       if( cc+strlen(curword)+4 >= len)
	       {
		  len = cc + WORDSIZE;
		  ptr = (struct define_item *) realloc(ptr, sizeof(struct define_item) + len);
		  if(ptr==0) cfatal("Preprocessor out of memory");
	       }
	       if( cc+strlen(curword) < len)
	       {
		  strcpy(ptr->value+cc, curword);
		  cc+=strlen(curword);
		  strcpy(ptr->value+cc, ",");
		  cc++;
		  ptr->arg_count++;
		  ch=gettok_nosub();
		  if( ch == ')' ) break;
		  if( ch == ',' ) continue;
	       }
	    }
            cerror("Bad #define command");
	    free(ptr);
            while(ch != '\n') ch = pgetc();
	    set_entry(0, name, (void*)old_value); /* Return var to old. */
	    return;
	 }
	 while((ch=pgetc())==' ' || ch=='\t');
      }
      else ptr->arg_count = -1;

      /* And the substitution string */
      while(ch != '\n')
      {
         if( cc+4 > len )
         {
            len = cc + WORDSIZE;
            ptr = (struct define_item *) realloc(ptr, sizeof(struct define_item) + len);
            if(ptr==0) cfatal("Preprocessor out of memory");
         }
         ptr->value[cc++] = ch;
         ch = pgetc();
      }
      ptr->value[cc++] = ' ';	/* Byte of lookahead for recursive macros */
      ptr->value[cc++] = 0;

#if CPP_DEBUG
      if (ptr->arg_count<0)
	 fprintf(stderr, "\n### Define '%s' as '%s'\n", 
		 name, ptr->value);
      else
	 fprintf(stderr, "\n### Define '%s' as %d args '%s'\n", 
		 name, ptr->arg_count, ptr->value);
#endif

      /* Clip to correct size and save */
      ptr = (struct define_item *) realloc(ptr, sizeof(struct define_item) + cc);
      ptr->name = set_entry(0, name, ptr);
      ptr->in_use = 0;
      ptr->next = 0;

      if (old_value) {
	 if (strcmp(old_value->value, ptr->value) != 0)
	    cwarn("#define redefined macro");
	 free(old_value);
      }
   }
   else cerror("Bad #define command");
   while(ch != '\n') ch = pgetc();
}

static void do_proc_undef()
{
   int ch;
   struct define_item * ptr;
   if( (ch=gettok_nosub()) == TK_WORD )
   {
      ptr = read_entry(0, curword);
      if(ptr)
      {
         set_entry(0, curword, (void*)0); /* Unset var */
         if (ptr->in_use) 
	    ; /* Eeeek! This shouldn't happen; so just let it leak. */
	 else
	    free(ptr);
      }
      do_proc_tail();
   }
   else
   {
      cerror("Bad #undef command");
      while(ch != '\n') ch = pgetc();
   }
}

static int do_proc_if(type)
int type;
{
   int ch = 0;
   if( type == 3 )
   {
      if( if_count == 0 )
	 cerror("#elif without matching #if");
      else
      {
	 if( if_has_else )
	    cerror("#elif following #else for one #if");
	 if( if_has_else || if_false != 1 )
	 {
	    if_false=2;
	    while(ch != '\n') ch = pgetc();
	    return 0;
	 }
	 if_false=0;
      }
      if_has_else = 0;
   }
   if(if_false)
   {
      if( type != 3 ) if_hidden++;
      do_proc_tail();
   }
   else
   {
      if( type != 3 )
      {
         if_count++;
         if_stack <<= 1;
         if_stack |= if_has_else;
         if_has_else = 0;
      }
      if(type > 1)
      {
	 ch = get_if_expression();
	 if_false=!ch;
      }
      else
      {
         ch = gettok_nosub();
         if( ch == TK_WORD )
         {
            do_proc_tail();
            if_false = (read_entry(0, curword) == 0);
            if(type == 1) if_false = !if_false;
         }
         else
         {
            cerror("Bad #if command");
            if_false = 0;
            while(ch != '\n') ch = pgetc();
         }
      }
   }
   return 0;
}

static void do_proc_else()
{
   if( if_hidden == 0 )
   {
      if( if_count == 0 )
         cerror("#else without matching #if");
      else
         if_false = (if_false^1);
      if( if_has_else )
         cerror("Multiple #else's for one #if");
      if_has_else = 1;
   }
   do_proc_tail();
}

static void do_proc_endif()
{
   if( if_hidden )
      if_hidden--;
   else
   {
      if( if_count == 0 )
         cerror("Unmatched #endif");
      else
      {
         if_count--;
         if_false=0;
         if_has_else = (if_stack&1);
         if_stack >>=1;
      }
   }
   do_proc_tail();
}

static void
do_proc_tail()
{
   int ch, flg=1;
   while((ch = pgetc()) != '\n') if(ch > ' ')
   {
      if (!if_false && flg)
         cwarn("Unexpected text following preprocessor command");
      flg=0;
   }
}

static int
get_if_expression()
{
   int value = get_expression(0);

   if (curtok != '\n')
      do_proc_tail();

   return value;
}

static int_type
get_expression(prio)
int prio;
{
   int_type lvalue;
   int_type rvalue;
   int no_op = 0;

   curtok = get_onetok();
   lvalue = get_exp_value();

   do
   {
      switch(curtok)
      {
	 case '*': case '/': case '%':
	    if (prio >= 10) return lvalue;
	    break;
	 case '+': case '-':
	    if (prio >= 9) return lvalue;
	    break;
	 case TK_RIGHT_OP: case TK_LEFT_OP:
	    if (prio >= 8) return lvalue;
	    break;
	 case '<': case '>': case TK_LE_OP: case TK_GE_OP:
	    if (prio >= 7) return lvalue;
	    break;
	 case TK_EQ_OP: case TK_NE_OP:
	    if (prio >= 6) return lvalue;
	    break;
	 case '&':
	    if (prio >= 5) return lvalue;
	    break;
	 case '^':
	    if (prio >= 4) return lvalue;
	    break;
	 case '|':
	    if (prio >= 3) return lvalue;
	    break;
	 case TK_AND_OP:
	    if (prio >= 2) return lvalue;
	    break;
	 case TK_OR_OP:
	    if (prio >= 1) return lvalue;
	    break;
      }
      switch(curtok)
      {
	 case '*':
	    rvalue = get_expression(10);
	    lvalue *= rvalue;
	    break;
	 case '/':
	    rvalue = get_expression(10);
	    if (rvalue)
	       lvalue /= rvalue;
	    break;
	 case '%':
	    rvalue = get_expression(10);
	    if (rvalue)
	       lvalue %= rvalue;
	    break;
	 case '+':
	    rvalue = get_expression(9);
	    lvalue += rvalue;
	    break;
	 case '-':
	    rvalue = get_expression(9);
	    lvalue -= rvalue;
	    break;
	 case TK_RIGHT_OP:
	    rvalue = get_expression(8);
	    lvalue >>= rvalue;
	    break;
	 case TK_LEFT_OP:
	    rvalue = get_expression(8);
	    lvalue <<= rvalue;
	    break;
	 case '<':
	    rvalue = get_expression(7);
	    lvalue = (lvalue < rvalue);
	    break;
	 case '>':
	    rvalue = get_expression(7);
	    lvalue = (lvalue > rvalue);
	    break;
	 case TK_LE_OP:
	    rvalue = get_expression(7);
	    lvalue = (lvalue <= rvalue);
	    break;
	 case TK_GE_OP:
	    rvalue = get_expression(7);
	    lvalue = (lvalue >= rvalue);
	    break;
	 case TK_EQ_OP:
	    rvalue = get_expression(6);
	    lvalue = (lvalue == rvalue);
	    break;
	 case TK_NE_OP:
	    rvalue = get_expression(6);
	    lvalue = (lvalue != rvalue);
	    break;
	 case '&':
	    rvalue = get_expression(5);
	    lvalue = (lvalue & rvalue);
	    break;
	 case '^':
	    rvalue = get_expression(4);
	    lvalue = (lvalue ^ rvalue);
	    break;
	 case '|':
	    rvalue = get_expression(3);
	    lvalue = (lvalue | rvalue);
	    break;
	 case TK_AND_OP:
	    rvalue = get_expression(2);
	    lvalue = (lvalue && rvalue);
	    break;
	 case TK_OR_OP:
	    rvalue = get_expression(1);
	    lvalue = (lvalue || rvalue);
	    break;

	 case '?': /* XXX: To add */

	 default:
	    no_op = 1;
      }
   }
   while(prio == 0 && !no_op);

   return lvalue;
}

static int_type
get_exp_value()
{
   int_type value = 0;
   int sign = 1;

   if (curtok == '!') {
      curtok = get_onetok();
      return !get_exp_value();
   }
   if (curtok == '~') {
      curtok = get_onetok();
      return ~get_exp_value();
   }

   while (curtok == '+' || curtok == '-') {
      if (curtok == '-') sign = -sign;
      curtok = get_onetok();
   }

   if (curtok == TK_NUM) {
      value = strtoul(curword, (void*)0, 0);
      curtok = get_onetok();
   } else if (curtok == TK_QUOT) {
      value = curword[1];
      if (value == '\\') {
	 if (curword[2] >= '0' && curword[2] <= '7') {
	    value = curword[2] - '0';
	    if (curword[3] >= '0' && curword[3] <= '7') {
	       value = (value<<3) + curword[3] - '0';
	       if (curword[4] >= '0' && curword[4] <= '7') {
		  value = (value<<3) + curword[4] - '0';
	       }
	    }
	 } else switch(curword[2]) {
	    case 'n': value = '\n'; break;
	    case 'f': value = '\f'; break;
	    case 't': value = '\t'; break;
	    default:  value = curword[2]; break;
	 }
      }
#ifdef NATIVE_CPP
      value = (char) value; /* Fix range */
#elif SIGNED_CHAR
      value = (signed char) value;
#else
      value = (unsigned char) value;
#endif
      curtok = get_onetok();
   } else if (curtok == TK_WORD) {
      value = 0;
      if (strcmp("defined", curword) == 0) {
	 curtok = gettok_nosub();
	 if (curtok == '(' && gettok_nosub() != TK_WORD)
	    cerror("'defined' keyword requires argument");
	 else {
	    value = (read_entry(0, curword) != 0);
	    if (curtok == '(' && gettok_nosub() != ')')
	       cerror("'defined' keyword requires closing ')'");
	    else
	       curtok = get_onetok();
	 }
      }
      else
	 curtok = get_onetok();

   } else if (curtok == '(') {
      value = get_expression(0);
      if (curtok == ')')
	 curtok = get_onetok();
      else
	 curtok = '$';
   }

   return sign<0 ? -value: value;
}

static int 
gen_substrings(macname, data_str, arg_count)
char * macname;
char * data_str;
int arg_count;
{
   char * mac_text = 0;
   struct arg_store *arg_list;
   int ac, ch, cc, len;
   int args_found = 1;	/* An empty arg still counts as one. */

   int paren_count = 0;
   int in_quote = 0;

   arg_list = malloc(sizeof(struct arg_store) * arg_count);
   memset(arg_list, 0, sizeof(struct arg_store) * arg_count);

   for(ac=0; *data_str && ac < arg_count; data_str++) {
      if( *data_str == ',' ) { ac++; continue; }

      if (arg_list[ac].name == 0) cc = len = 0;

      if (cc+2 >= len) {
	 len += 20;
	 arg_list[ac].name = realloc(arg_list[ac].name, len);
      }
      arg_list[ac].name[cc++] = *data_str;
      arg_list[ac].name[cc] = 0;
   }

   while((ch = chget()) <= ' ' && ch != EOF ) ;

   if( ch != '(' )
   {
      /* Macro name used without arguments, ignore substitution */
      unchget(ch);
      /* unchget(' '); 	.* This is needed incase the next thing is a word */
      			/* to stop these two words being stuck together. */
      return 0;
   }

   for(;;) {
      if ((ch = chget()) == EOF) break;
      if(in_quote == 2) {
	 in_quote = 1;
      } else if (in_quote) {
	 if ( ch == '"' ) in_quote = 0;
	 if ( ch == '\\') in_quote = 2;
      } else {
	 if ( ch == '(' ) paren_count++;
	 if ( ch == '"' ) in_quote = 1;
	 if (paren_count == 0 && ch == ',' ) {
	    args_found++; continue;
	 }
	 if ( ch == ')' ) {
	    if (paren_count == 0) break;
	    paren_count--;
	 }
      }
      /* Too many args; ignore rest */
      if (args_found > arg_count ) continue;
      ac = args_found-1;
      if (arg_list[ac].value == 0) {
	 cc = len = 0;
	 arg_list[ac].in_define = def_count;
      }

      if (cc+2 >= len) {
	 len += 20;
	 arg_list[ac].value = realloc(arg_list[ac].value, len);
      }
      if (ch == '\n' && cc>0 && arg_list[ac].value[cc-1] == '\\' ) {
	 /* Humm, ok */
	 arg_list[ac].value[--cc] = 0;
	 ch = ' ';
	 if (!ansi_c) continue;
      }

      if (ch == '\n' && cc>0 && arg_list[ac].value[cc-1] == '\n' ) {
	 cerror("Unquoted newline in macro arguments");
	 break;
      }

      arg_list[ac].value[cc++] = ch;
      arg_list[ac].value[cc] = 0;
   }

   if( arg_count != args_found )
      cerror("Incorrect number of macro arguments");

   mac_text = insert_substrings(data_str, arg_list, arg_count);

   if (arg_list) {
      for (ac=0; ac<arg_count; ac++) {
	 if (arg_list[ac].name) free(arg_list[ac].name);
	 if (arg_list[ac].value) free(arg_list[ac].value);
      }
      free(arg_list);
   }

   saved_ref[def_count] = def_ref;
   saved_def[def_count] = def_ptr;
   saved_start[def_count] = def_start;
   saved_unputc[def_count] = unputc;
   def_count++;
   unputc = 0;
   def_ptr = mac_text;
   def_start = mac_text;
#if CPP_DEBUG
   fprintf(stderr, "\n### <DEF%d='%s'>\n", def_count, mac_text);
#endif
   return 1;
}

static char * 
insert_substrings(data_str, arg_list, arg_count)
char * data_str;
struct arg_store *arg_list;
int arg_count;
{
   int ac, ch;
   char * p, * s;
   char * rv = 0;
   int    len = 0;
   int    cc = 0;
   int    in_quote = 0;
   int    ansi_stringize = 0;

#if CPP_DEBUG
   fprintf(stderr, "\n### Macro substitution in '%s'\n", data_str);
   for (ac=0; ac<arg_count; ac++) {
      fprintf(stderr, "### Argument %d (%s) = '%s'\n", 
	 ac+1, arg_list[ac].name, arg_list[ac].value);
   }
#endif

   rv = malloc(4); *rv = 0; len = 4;

   while(*data_str) {
      p = curword;

      if (ansi_c) {
	 if (in_quote == 2)
	    in_quote = 1;
	 else if (in_quote) {
	    if (*data_str == '"') in_quote = 0;
	    if (*data_str == '\\') in_quote = 2;
	 } else {
	    if (*data_str == '"') in_quote = 1;
	 }
      }

      if (!in_quote) for(;;) {
	 ch = *data_str;
         if( (ch >= '0' && ch <= '9')
	  || (ch >= 'A' && ch <= 'Z')
	  || (ch >= 'a' && ch <= 'z')
	  || ch == '_' || ch == '$' )
	    *p++ = *data_str++;
	 else
	    break;
      }

      if (p == curword) {
	 /* Ansi Stringize and concat */
	 if (*data_str == '#' && ansi_c) {
	    if (data_str[1] == '#') {
	       while(cc>0 && (rv[cc-1] == ' ' || rv[cc-1] == '\t'))
		  cc--;
	       data_str+=2;
	       while(*data_str == ' ' || *data_str == '\t')
		  data_str++;
	       if (*data_str == 0) { /* Hummm */
		  data_str--;
		  cerror("'##' operator at end of macro");
	       }
	       continue;
	    }
	    data_str++;
	    ansi_stringize = 1;
	    continue;
	 }

	 if (ansi_stringize) {
	    ansi_stringize = 0;
	    cerror("'#' operator should be followed by a macro argument name");
	 }

	 /* Other characters ... */
	 if (cc+2 > len) { len += 20; rv = realloc(rv, len); }
	 rv[cc++] = *data_str++;
	 continue;
      }
      *p = 0; s = curword;
      for (ac=0; ac<arg_count; ac++) {
	 if (*curword == arg_list[ac].name[0] &&
	     strcmp(curword, arg_list[ac].name) == 0)
	 {
	    s = arg_list[ac].value;

	    /* Ansi stringize operation, this is very messy! */
	    if (ansi_stringize) {
	       if (arg_list[ac].in_define) {
		  struct define_item * ptr;
                  if ((ptr = read_entry(0, s)) &&  
		       ptr->arg_count == -1) {
		     s = ptr->value;
		  }
	       }

	       rv[cc++] = '"';
	       while(*s == ' ' || *s == '\t') s++;
	       while (*s) {
		  if (cc+4 > len) { len += 20; rv = realloc(rv, len); }
		  if (*s == '"') rv[cc++] = '\\';
		  rv[cc++] = *s++;
	       }
	       while(cc>0 && (rv[cc-1] == ' ' || rv[cc-1] == '\t'))
		  cc--;
	       rv[cc++] = '"';
	       rv[cc++] = 0;
	       ansi_stringize = 0;
	       s = "";
	       break;
	    }

	    break;
	 }
      }

      if (ansi_stringize) {
	 ansi_stringize = 0;
	 cerror("'#' operator should be followed by a macro argument name");
      }

      if (cc+2+strlen(s) > len) { len += strlen(s)+20; rv = realloc(rv, len); }
      strcpy(rv+cc, s);
      cc = strlen(rv);
   }

   rv[cc] = 0;
   return rv;
}
