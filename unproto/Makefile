# @(#) Makefile 1.3 91/12/01 12:37:57

## BEGIN CONFIGURATION STUFF

# For maximal flexibility, the "/lib/cpp | unproto" pipeline can be
# packaged as an executable shell script (see the provided "cpp.sh" script)
# that should be installed as "/whatever/cpp". This script should then be
# specified to the C compiler as a non-default preprocessor.
#
PROG	= unproto
PIPE	= 

# For maximal performance, the overhead of shell script inpretation can
# be eliminated by having the unprototyper program itself open the pipe
# to the preprocessor.  In that case, define the PIPE_THROUGH_CPP macro
# as the path name of the default C preprocessor (usually "/lib/cpp"),
# install the unprototyper as "/whatever/cpp" and specify that to the C
# compiler as a non-default preprocessor.
# 
# PROG	= cpp
# PIPE	= -DPIPE_THROUGH_CPP=\"/lib/cpp\"

# Some compilers complain about some #directives. The following is only a 
# partial solution, because the directives are still seen by /lib/cpp.
# Be careful with filtering out #pragma, because some non-ANSI compilers
# (SunOS) rely on its use.
#
# SKIP	= -DIGNORE_DIRECTIVES=\"pragma\",\"foo\",\"bar\"
#
SKIP	=

# If you need support for functions that implement ANSI-style variable
# length argument lists, edit the stdarg.h file provided with this
# package so that it contains the proper definitions for your machine.

## END CONFIGURATION STUFF

SHELL	= /bin/sh

CFILES	= tok_io.c tok_class.c tok_pool.c unproto.c vstring.c symbol.c error.c
HFILES	= error.h token.h vstring.h symbol.h
SCRIPTS	= cpp.sh
SAMPLES	= stdarg.h varargs.c example.c example.out 
SOURCES	= README Makefile $(CFILES) $(HFILES) $(SCRIPTS) $(SAMPLES)
FILES	= $(SOURCES) unproto.1
OBJECTS	= tok_io.o tok_class.o tok_pool.o unproto.o vstring.o symbol.o error.o

ifneq ($(TOPDIR),)
include $(TOPDIR)/Make.defs
CFLAGS=$(CCFLAGS) -w $(PIPE) $(SKIP)
# I'm fed up, '-w' will shut GCC up about this nasty code!
else
CC=bcc
CFLAGS=-O $(PIPE) $(SKIP)
LDFLAGS=-s
endif

#CFLAGS	= -O -pg -Dstatic= $(PIPE) $(SKIP)
#CFLAGS	= -g $(PIPE) $(SKIP) -DDEBUG

$(PROG): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(MALLOC)

install: $(PROG)
	install -d $(LIBDIR)
	install -m 755 $(PROG) $(LIBDIR)/unproto

# For linting, enable all bells and whistles.

lint:
	lint -DPIPE_THROUGH_CPP=\"foo\" -DIGNORE_DIRECTIVES=\"foo\",\"bar\" \
	$(CFILES)

# Testing requires that the program is compiled with -DDEBUG

test:	$(PROG) example.c example.out
	./cpp example.c >example.tmp
	@echo the following diff command should produce no output
	diff -b example.out example.tmp
	rm -f example.tmp

shar:	$(FILES)
	@shar $(FILES)

archive:
	$(ARCHIVE) $(SOURCES)

clean:
	rm -f *.o core cpp unproto mon.out varargs.o varargs example.tmp

error.o : error.c token.h error.h Makefile
symbol.o : symbol.c token.h symbol.h Makefile
tok_class.o : tok_class.c error.h vstring.h token.h symbol.h Makefile
tok_io.o : tok_io.c token.h vstring.h error.h Makefile
tok_pool.o : tok_pool.c token.h vstring.h Makefile
unproto.o : unproto.c vstring.h stdarg.h token.h error.h symbol.h Makefile
varargs.o : varargs.c stdarg.h Makefile
vstring.o : vstring.c vstring.h Makefile
