#
# Flag that we're using GNU make.
#

all: phony
	@$(MAKE) $(MAKEARG) -f Makefile IFDEFFLAGS=-DGNUMAKE IFDEFNAME=ifdefg $@

%: phony
	@$(MAKE) $(MAKEARG) -f Makefile IFDEFFLAGS=-DGNUMAKE IFDEFNAME=ifdefg $@

phony: ;

GNUmakefile: ;
