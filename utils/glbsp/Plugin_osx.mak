#
# glBSP Plugin Makefile for MacOSX
#

BIN=libglbsp.a

CC=gcc
CFLAGS=-O2 -Wall -DGLBSP_PLUGIN -DMACOSX -DINLINE_G=inline

# ----- OBJECTS ------------------------------------------------------

OBJS=analyze.o  \
     blockmap.o \
     glbsp.o    \
     level.o    \
     node.o     \
     reject.o   \
     seg.o      \
     system.o   \
     util.o     \
     wad.o

# ----- TARGETS ------------------------------------------------------

all:    $(BIN)

clean:
	rm -f $(BIN) *.o

$(BIN): $(OBJS)
	libtool -static -o $(BIN) - $(OBJS)
	ranlib $(BIN)

.PHONY: all clean

