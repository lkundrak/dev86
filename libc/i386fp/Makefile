# Makefile for bcc 386 software floating point library

.SUFFIXES: .x		# .x files are .s files that need C-preprocessing
.x.o:
	$(CC) $(CFLAGS) -c $< -o $@

FPDIST		=Makefile $(FPSRC) test.c bccfp.tex
FPSRC		=fadd.x fcomp.x fdiv.x fmul.x fbsr.x \
		fperr.c fperror.x fptoi.x fpushd.x fpulld.x \
		fpushi.x fpushf.x fpullf.x frexp.x ftst.x \
		gcclib.x \
		fabs.x ldexp.x ecvt.c \
		fperr.h fplib.h
FPOBJ		=fadd.o fcomp.o fdiv.o fmul.o fpbsr.o \
		fperr.o fperror.o fptoi.o fpushd.o fpulld.o \
		fpushi.o fpushf.o fpullf.o frexp.o ftst.o \
		fabs.o ldexp.o ecvt.o
LIB		=.

CFLAGS=$(ARCH) $(CCFLAGS) $(DEFS)

ifeq ($(LIB_CPU),i386)
all: $(LIBC)($(FPOBJ))
	rm -f $(FPOBJ)
else
all:
	@:
endif

$(LIBC)($(FPOBJ)): fplib.h
$(LIBC)(fperr.o fperror.o): fperr.h

clean:
	rm -f $(FPOBJ) test
	rm -f $(LIB)/libfp.a bccfp.tar.Z bccfp.uue
