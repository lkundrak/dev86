CC=gcc 
CFLAGS=-O6 -fomit-frame-pointer
LDFLAGS=
# Where to get strtoul ()?
#LIBS=-liberty
DIRS=ld as

BINDIR=/usr/gnu/i486-linux/bin
AS86=i486-linux-ld86
LD86=i486-linux-ld86

BINDIR=/usr/bin
AS86=as86
LD86=ld86

STRIP=strip

MFLAGS= "LIBS=$(LIBS)" \
	"CFLAGS=$(CFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" \
	"CC=$(CC)"

all:
	for d in $(DIRS); do \
	  (cd $$d; $(MAKE) $(MFLAGS) $@;); \
	 done

install: all
	cp as/as86 $(BINDIR)/$(AS86)
	cp ld/ld86 $(BINDIR)/$(LD86)
	$(STRIP) $(BINDIR)/$(AS86) $(BINDIR)/$(LD86)
		   
depend clean clobber:
	for d in $(DIRS); do \
	  (cd $$d; $(MAKE) $(MFLAGS) $@;); \
	done
