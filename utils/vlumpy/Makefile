ifdef DJGPP
EXE=.exe
LIBS=-lstdcxx
else
EXE=
LIBS=-lm -lstdc++
endif

CFLAGS = -Wall -O3 -ffast-math -fomit-frame-pointer -march=pentiumpro
OBJS = cmdlib.o imglib.o scrlib.o wadlib.o vlumpy.o

all: ../bin/vlumpy$(EXE)

../bin/vlumpy$(EXE): $(OBJS)
	gcc -s -o $@ $(OBJS) $(LIBS)

%.o: %.cpp ../common/*.h
	gcc $(CFLAGS) -I../common -c -o $@ $<

%.o: ../common/%.cpp ../common/*.h
	gcc $(CFLAGS) -c -o $@ $<

