#
# Makefile for termcap functions
#

override DEBUG=false
override PROFILE=false
#override CHECKER=false
JUMP_LIB=libtermcap

TOPDIR=..

include $(TOPDIR)/Makeconfig
include $(TOPDIR)/Makerules

override STATIC_LIB=$(STATIC_DIR)/libtermcap.a
override SHARED_LIB=$(SHARED_DIR)/libtermcap.a
override CHECKER_LIB=$(CHECKER_DIR)/libtermcap.a

DIRS:=
SRCS = termcap.c tparam.c
ASMS= $(SRCS:.c=.s)
OBJS= $(SRCS:.c=.o)
ALIASES=

include $(TOPDIR)/Maketargets  
