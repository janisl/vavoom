#
# glBSP Makefile for UNIX/Linux
#

CC=gcc

PROGNAME=glbsp

CFLAGS=-O3 -Wall -DUNIX -DINLINE_G=inline
LIBS=-lm

OBJS=blockmap.o level.o main.o node.o seg.o system.o wad.o

SRC_DIR=glbsp
SRC_FILES=$(SRC_DIR)/[Mm]ake* $(SRC_DIR)/*.[ch] $(SRC_DIR)/*.txt


# ----- TARGETS ------------------------------------------------------

all:    $(PROGNAME)

clean:
	rm -f $(PROGNAME) *.o core glbsp.txt glbsp.exe

$(PROGNAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGNAME) $(LIBS)

backup: clean
	tar czf /tmp/glbsp.tgz make* *.[ch]

bin: all
	strip --strip-unneeded $(PROGNAME)
	cat README.txt USAGE.txt > glbsp.txt

tar: clean
	cd ..; tar czf /tmp/glbspsrc.tgz $(SRC_FILES)

zip: clean
	cd ..; zip -l /tmp/glbspsrc.zip $(SRC_FILES)

.PHONY: all clean backup bin tar zip

