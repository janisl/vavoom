ifdef DJGPP
EXE=.exe
LIBS=-lstdcxx
INSTALLDIR=../bin
else
EXE=
LIBS=-lm -lstdc++
INSTALLDIR=/usr/local/bin
endif

CFLAGS = -Wall -O3 -ffast-math -fomit-frame-pointer -fno-rtti -fno-exceptions -mpentiumpro
OBJS = cmdlib.o imglib.o scrlib.o wadlib.o vlumpy.o

all: vlumpy$(EXE)

vlumpy$(EXE): $(OBJS)
	gcc -s -o $@ $(OBJS) $(LIBS)

%.o: %.cpp ../common/*.h
	gcc $(CFLAGS) -I../common -c -o $@ $<

%.o: ../common/%.cpp ../common/*.h
	gcc $(CFLAGS) -c -o $@ $<

install: vlumpy$(EXE)
	cp $< $(INSTALLDIR)

