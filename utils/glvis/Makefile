ifdef DJGPP
EXE=.exe
LIBS=-lstdcxx
INSTALLDIR=../bin
else
EXE=
LIBS=-lstdc++
INSTALLDIR=/usr/local/bin
endif

CFLAGS = -Wall -O3 -ffast-math -fomit-frame-pointer -fno-rtti -fno-exceptions -mpentiumpro
OBJS = cmdlib.o wadlib.o glvis.o level.o flow.o
LIBS += -lm

all: glvis$(EXE)

glvis$(EXE): $(OBJS)
	gcc -s -o $@ $(OBJS) $(LIBS)

%.o: %.cpp *.h ../common/*.h
	gcc $(CFLAGS) -I../common -c -o $@ $<

%.o: ../common/%.cpp ../common/*.h
	gcc $(CFLAGS) -c -o $@ $<

install: glvis$(EXE)
	cp $< $(INSTALLDIR)

