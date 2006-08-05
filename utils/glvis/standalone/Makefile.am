lib_LIBRARIES = libglvis.a
libglvis_a_SOURCES = \
	cmdlib.cpp \
	cmdlib.h \
	fmapdefs.h \
	fwaddefs.h \
	wadlib.cpp \
	wadlib.h \
	flow.cpp \
	glvis.h \
	glvisint.h \
	level.cpp \
	vector.h

bin_PROGRAMS = glvis
glvis_SOURCES = glvis.cpp
glvis_LDADD = libglvis.a

docdir=$(datadir)/doc/glvis
dist_doc_DATA = glvis.txt

EXTRA_DIST = autogen.sh
