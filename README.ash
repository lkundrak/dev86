Building /bin/ash

After doing make install for 'bcc' unpack ash-linux-0.2.tar.gz; within this
rename builtins to builtins.in then patch with this patchfile.

You'll also need to turn on 'JOBS' on line 58 of shell.h when libc can
provide the functions.

If you don't install the elksemu kernel patch you'll also have to make
sure that the programs that are run by the make file are interpreted
through elksemu. If it's installed you should be able just to type 'make'.

diff -ru ash-linux-0.2/Makefile ash/Makefile
--- ash-linux-0.2/Makefile	Sun May 15 20:16:13 1994
+++ ash/Makefile	Sat Mar 16 11:28:19 1996
@@ -1,5 +1,9 @@
 #	Makefile,v 1.7 1993/08/09 04:58:18 mycroft Exp
 
+CC=bcc
+CFLAGS=-DSHELL
+LDFLAGS=-s
+
 PROG=	sh
 SRCS=	builtins.c cd.c dirent.c echo.c error.c eval.c exec.c expand.c \
 	input.c jobs.c mail.c main.c memalloc.c miscbltin.c \
@@ -14,8 +18,8 @@
 
 OBJS =	$(OBJ1) $(OBJ2)
 
-CFLAGS = -O2 -fomit-frame-pointer -m486 -DSHELL -I/usr/include/bsd -I.
-LDFLAGS = -s -lbsd
+# CFLAGS = -O2 -fomit-frame-pointer -m486 -DSHELL -I/usr/include/bsd -I.
+# LDFLAGS = -s -lbsd
 
 CLEANFILES =\
 	builtins.c builtins.h init.c mkinit mknodes mksyntax \
@@ -32,7 +36,7 @@
 token.def: mktokens
 	sh ./mktokens
 
-builtins.h builtins.c: mkbuiltins builtins
+builtins.h builtins.c: mkbuiltins builtins.in
 	sh ./mkbuiltins
 
 init.c: mkinit $(SRCS)
Only in ash-linux-0.2: builtins
Only in ash: builtins.in
diff -ru ash-linux-0.2/main.c ash/main.c
--- ash-linux-0.2/main.c	Mon Oct  4 19:47:56 1993
+++ ash/main.c	Sun Feb 18 21:57:59 1996
@@ -321,9 +321,9 @@
 /*
  * Should never be called.
  */
-#endif
 
 void
 exit(exitstatus) {
 	_exit(exitstatus);
 }
+#endif
diff -ru ash-linux-0.2/miscbltin.c ash/miscbltin.c
--- ash-linux-0.2/miscbltin.c	Mon Oct  4 19:47:56 1993
+++ ash/miscbltin.c	Sun Feb 18 21:56:14 1996
@@ -45,6 +45,7 @@
 
 #include <sys/types.h>
 #include <sys/stat.h>
+#include <ctype.h>
 #include "shell.h"
 #include "options.h"
 #include "var.h"
@@ -208,14 +209,18 @@
 			umask(mask);
 		} else {
 #ifndef __linux__
+#ifndef __BCC__
 			void *set; 
 			if ((set = setmode (ap)) == 0)
 #endif
+#endif
 					error("Illegal number: %s", ap);
 
 #ifndef __linux__
+#ifndef __BCC__
 			mask = getmode (set, ~mask & 0777);
 			umask(~mask & 0777);
+#endif
 #endif
 		}
 	}
diff -ru ash-linux-0.2/mkbuiltins ash/mkbuiltins
--- ash-linux-0.2/mkbuiltins	Sun Apr 18 18:37:04 1993
+++ ash/mkbuiltins	Mon Feb 12 19:42:46 1996
@@ -54,7 +54,7 @@
 #include "builtins.h"
 
 !
-awk '/^[^#]/ {if('$havejobs' || $2 != "-j") print $0}' builtins |
+awk '/^[^#]/ {if('$havejobs' || $2 != "-j") print $0}' builtins.in |
 	sed 's/-j//' > $temp
 awk '{	printf "int %s();\n", $1}' $temp
 echo '
diff -ru ash-linux-0.2/shell.h ash/shell.h
--- ash-linux-0.2/shell.h	Mon Oct  4 19:47:56 1993
+++ ash/shell.h	Sat Mar 16 11:28:54 1996
@@ -60,7 +60,7 @@
 #define DIRENT 1
 #define UDIR 0
 #define ATTY 0
-#define BSD
+/* #define BSD */
 /* #define DEBUG */
 
 #ifdef __STDC__
