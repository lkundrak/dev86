/* byteord.h - byte order dependencies for bcc */

/* Copyright (C) 1992 Bruce Evans */

#ifdef I8088
# define INT_BIG_ENDIAN 0
# define LONG_BIG_ENDIAN 1	/* longs are back to front for Xenix */
#endif

#ifdef MC6809
# define INT_BIG_ENDIAN 1	/* byte order in words is high-low */
# define LONG_BIG_ENDIAN 1	/* byte order in longs is high-low */
#endif
