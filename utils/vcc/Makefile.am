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
	lexer.cpp \
	lexer.h \
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
	types.cpp \
	vcc.cpp \
	vcc.h

EXTRA_DIST = vcc.txt
