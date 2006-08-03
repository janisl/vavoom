CXXFLAGS=@GLVIS_CXXFLAGS@
CPPFLAGS=@GLVIS_CPPFLAGS@
DEFS=
LDFLAGS=@GLVIS_LDFLAGS@
LIBS=@GLVIS_LIBS@

INCLUDES = -I$(srcdir)/../common

noinst_LIBRARIES = libglvis.a
libglvis_a_SOURCES = \
	../common/cmdlib.cpp \
	../common/cmdlib.h \
	../common/fmapdefs.h \
	../common/fwaddefs.h \
	../common/wadlib.cpp \
	../common/wadlib.h \
	flow.cpp \
	glvis.h \
	glvisint.h \
	level.cpp \
	vector.h

noinst_PROGRAMS = glvis
glvis_SOURCES = glvis.cpp
glvis_LDADD = libglvis.a

EXTRA_DIST = glvis.txt
