ifdef DJGPP
EXE=.exe
INSTALLDIR=../bin
else
EXE=
INSTALLDIR=/usr/local/bin
endif

CFLAGS = -Wall -O3 -ffast-math -fomit-frame-pointer -mpentiumpro

OBJS = \
	cmdlib.o \
	constexp.o \
	error.o \
	expressn.o \
	info.o \
	parse.o \
	pcode.o \
	token.o \
	types.o \
	vcc.o

all: vcc$(EXE)

vcc$(EXE): $(OBJS)
	gcc -s -o $@ $(OBJS)

%.o : %.cpp vcc.h ../../source/progdefs.h ../common/cmdlib.h
	gcc $(CFLAGS) -I../common -c -o $@ $<

%.o : ../common/%.cpp ../common/cmdlib.h
	gcc $(CFLAGS) -c -o $@ $<

install: vcc$(EXE)
	cp vcc$(EXE) $(INSTALLDIR)

