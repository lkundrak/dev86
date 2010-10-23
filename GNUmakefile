#
# Flag that we're using GNU make.
#

all: phony
	@$(MAKE) -f Makefile IFDEFFLAGS=-DGNUMAKE IFDEFNAME=ifdefg $@

%: phony
	@$(MAKE) -f Makefile IFDEFFLAGS=-DGNUMAKE IFDEFNAME=ifdefg $@

phony: ;

GNUmakefile: ;
