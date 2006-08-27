CXXFLAGS=@VCC_CXXFLAGS@
CPPFLAGS=@VCC_CPPFLAGS@
DEFS=
LDFLAGS=@VCC_LDFLAGS@
LIBS=@VCC_LIBS@

INCLUDES = -I$(srcdir)/../common

noinst_PROGRAMS = ../bin/vcc

___bin_vcc_SOURCES = \
	../common/array.h \
	../common/cmdlib.cpp \
	../common/cmdlib.h \
	error.cpp \
	expression.cpp \
	expression.h \
	info.cpp \
	modifiers.cpp \
	name.cpp \
	name.h \
	names.h \
	parse.cpp \
	pcode.cpp \
	statement.cpp \
	statement.h \
	stream.cpp \
	stream.h \
	token.cpp \
	types.cpp \
	vcc.cpp \
	vcc.h \
	vcpp/cpp.cpp \
	vcpp/cpp.h \
	vcpp/eval.cpp \
	vcpp/hideset.cpp \
	vcpp/include.cpp \
	vcpp/lex.cpp \
	vcpp/macro.cpp \
	vcpp/nlist.cpp \
	vcpp/tokens.cpp \
	vcpp/unix.cpp

EXTRA_DIST = vcc.txt
