ifdef DJGPP
EXE=.exe
LIBS=-lstdcxx
else
EXE=
LIBS=-lstdc++
endif

#CFLAGS = -Wall -O3 -ffast-math -fomit-frame-pointer -march=pentiumpro
#CFLAGS = -Wall -O2 -ffast-math
CFLAGS = -Wall -g

OBJS = \
	cmdlib.o \
	constexp.o \
	error.o \
	expressn.o \
	info.o \
	name.o \
	parse.o \
	parse1.o \
	pcode.o \
	token.o \
	types.o \
	vcc.o

CPP_OBJS = \
	vcpp/cpp.o \
	vcpp/eval.o \
	vcpp/hideset.o \
	vcpp/include.o \
	vcpp/lex.o \
	vcpp/macro.o \
	vcpp/nlist.o \
	vcpp/tokens.o \
	vcpp/unix.o

all: ../bin/vcc$(EXE)

../bin/vcc$(EXE): $(OBJS) vcpp/libcpp.a
	gcc -s -o $@ $^ $(LIBS)

vcpp/libcpp.a: $(CPP_OBJS)
	ar rc $@ $^
	ranlib $@

%.o : %.cpp vcc.h ../../source/progdefs.h ../common/cmdlib.h
	gcc $(CFLAGS) -I../common -c -o $@ $<

%.o : ../common/%.cpp ../common/cmdlib.h
	gcc $(CFLAGS) -c -o $@ $<

vcpp/%.o : vcpp/%.cpp vcpp/cpp.h
	gcc $(CFLAGS) -c -o $@ $<
