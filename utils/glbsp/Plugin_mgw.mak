#
# glBSP Makefile for MINGW32 Plugin
#

MAIN=.

OUTNAME=libglbsp.a

CC=gcc
CFLAGS=-mno-cygwin -O3 -Wall -DGLBSP_PLUGIN -DINLINE_G=inline
AR=ar rc
RANLIB=ranlib

OBJS=$(MAIN)/analyze.o  \
     $(MAIN)/blockmap.o \
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
	rm -f $(OUTNAME) $(MAIN)/*.o

$(OUTNAME): $(OBJS)
	$(AR) $(OUTNAME) $(OBJS)
	$(RANLIB) $(OUTNAME)

.PHONY: all clean

