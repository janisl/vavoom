ifdef DJGPP
EXE=.exe
LIBS=-lstdcxx
INSTALLDIR=../bin
else
EXE=
LIBS=-lstdc++
INSTALLDIR=/usr/local/bin
endif

CFLAGS = -Wall -O3 -ffast-math -fomit-frame-pointer -mpentiumpro
LIB_OBJS = cmdlib.o wadlib.o level.o flow.o
OBJS = glvis.o
LIBS += -lm

all: glvis$(EXE)

glvis$(EXE): $(OBJS) libglvis.a
	gcc -s -o $@ $^ $(LIBS)

libglvis.a: $(LIB_OBJS)
	ar rc $@ $^
	ranlib $@

%.o: %.cpp *.h ../common/*.h
	gcc $(CFLAGS) -I../common -c -o $@ $<

%.o: ../common/%.cpp ../common/*.h
	gcc $(CFLAGS) -c -o $@ $<

install: glvis$(EXE)
	cp $< $(INSTALLDIR)

