#
# glBSP Makefile for UNIX Plugin
#

MAIN=..
SYSDIR=.

OUTNAME=libglbsp.a

CC=gcc
CFLAGS=-O3 -g -Wall -DGLBSP_PLUGIN -DUNIX -DINLINE_G=inline
AR=ar rc
RANLIB=ranlib

OBJS=$(MAIN)/blockmap.o \
     $(MAIN)/glbsp.o    \
     $(MAIN)/level.o    \
     $(MAIN)/node.o     \
     $(MAIN)/reject.o   \
     $(MAIN)/seg.o      \
     $(MAIN)/system.o   \
     $(MAIN)/util.o     \
     $(MAIN)/wad.o


# ----- TARGETS ------------------------------------------------------

all:    $(OUTNAME)

clean:
	rm -f $(OUTNAME) $(MAIN)/*.o $(SYSDIR)/*.o

$(OUTNAME): $(OBJS)
	$(AR) $(OUTNAME) $(OBJS)
	$(RANLIB) $(OUTNAME)

.PHONY: all clean

