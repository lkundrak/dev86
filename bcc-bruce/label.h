/* label.h - assembler-specific label characters for bcc */

/* Copyright (C) 1992 Bruce Evans */

/* defaults */

#define CCNAMEPREFIX '_'
# define LABELENDCHAR ':'	/* last char of ALL labels */
#define LABELSTARTCHAR '.'	/* first char of names of generated labels */
#define LOCALSTARTCHAR '.'	/* first char of local names */
#define PUBLICENDCHAR ':'

/* adjustments for other assemblers */

#ifdef AS09
# undef LABELENDCHAR
#endif

#ifdef XENIX_AS
# undef LABELSTARTCHAR
# define LABELSTARTCHAR 'L'
# undef LOCALSTARTCHAR
# define LOCALSTARTCHAR 'L'
#endif
