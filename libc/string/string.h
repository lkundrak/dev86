
#ifndef __STRING_H
#define __STRING_H
#include <features.h>
#include <sys/types.h>
#include <stddef.h>

/* Basic string functions */
extern size_t strlen __P ((__const char* __str));

extern char * strcat __P ((char*, __const char*));
extern char * strcpy __P ((char*, __const char*));
extern int strcmp __P ((__const char*, __const char*));

extern char * strncat __P ((char*, char*, size_t));
extern char * strncpy __P ((char*, char*, size_t));
extern int strncmp __P ((__const char*, __const char*, size_t));

extern char * strchr __P ((char*, int));
extern char * strrchr __P ((char*, int));
extern char * strdup __P ((char*));

/* Basic mem functions */
extern void * memcpy __P ((void*, __const void*, size_t));
extern void * memccpy __P ((void*, void*, int, size_t));
extern void * memchr __P ((__const void*, __const int, size_t));
extern void * memset __P ((void*, int, size_t));
extern int memcmp __P ((__const void*, __const void*, size_t));

extern void * memmove __P ((void*, void*, size_t));

/* Minimal (very!) locale support */
#define strcoll strcmp
#define strxfrm strncpy

/* BSDisms */
#define index strchr
#define rindex strrchr

/* Other common BSD functions */
extern int strcasecmp __P ((char*, char*));
extern int strncasecmp __P ((char*, char*, size_t));
char *strpbrk __P ((char *, char *));
char *strsep __P ((char **, char *));
char *strstr __P ((char *, char *));
char *strtok __P ((char *, char *));
size_t strcspn __P ((char *, char *));
size_t strspn __P ((char *, char *));

/* Linux silly hour */
char *strfry __P ((char *));

#endif
