/* os.h - source/target operating system dependencies for bcc */

/* Copyright (C) 1992 Bruce Evans */

/*
  must have unix-near-compatible creat, open, read, write and close

  source O/S's supported:
    default:
      *IX
    special:
      EDOS		(#define SOS_EDOS if required)
      MSDOS		(#define SOS_MSDOS)
  target O/S's supported:
    default:
      *IX
      MSDOS
    special:
      EDOS		(#define TOS_EDOS)
*/

/* defaults */

#define CREATPERMS 0666		/* permissions for creat */
#define EOL 10			/* source newline */
#define EOLTO 10		/* target newline */
#define DIRCHAR '/'
#define DIRSTRING "/"
#define DEFAULT_INCLUDE_DIR "/usr/include"
#define isabspath(fnameptr, tempcptr) \
	((*(tempcptr) = *(fnameptr)) == DIRCHAR)

/* special */

#ifdef SOS_EDOS
# undef DEFAULT_INCLUDE_DIR
# define DEFAULT_INCLUDE_DIR "3"
# undef DIRCHAR
# define DIRCHAR ':'
# undef DIRSTRING
# define DIRSTRING ":"
# define AS09
# undef EOL
# define EOL 13
# undef isabspath
# define isabspath(fnameptr, tempcptr) \
	 ((*(tempcptr) = *(fnameptr)) >= '0' && *(tempcptr) <= '9' && \
	  (fnameptr)[1] == DIRCHAR)
#endif

#ifdef SOS_MSDOS
# define COEOL 10
# undef DEFAULT_INCLUDE_DIR
# define DEFAULT_INCLUDE_DIR "\\usr\\include"
# undef DIRCHAR
# define DIRCHAR '\\'
# undef DIRSTRING
# define DIRSTRING "\\"
# undef EOL
# define EOL 13
/*
  Problems with newline on MSDOS (as usual).
  Defining EOL as 10 (LF) handles LF-terminated lines as well as
  CRLF-terminated lines, but escaped CRLF's do not work. The compiler
  should handle the escapes better.  (Perhaps it does now.)
  Defining EOL as 13 (CR) handles CRLF-terminated lines but fails on
  LF-terminated lines.
*/
#endif

#ifdef TOS_EDOS
# undef EOLTO
# define EOLTO 13
#endif

/* don't let names dealt with here affect anything outside this file */

#undef SOS_EDOS
#undef SOS_MSDOS
