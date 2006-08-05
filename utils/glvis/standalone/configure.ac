AC_INIT([glvis], [1.5])
AC_CONFIG_SRCDIR([glvis.h])
AM_INIT_AUTOMAKE([foreign no-define dist-zip dist-bzip2])

AC_CANONICAL_HOST
AC_LANG_CPLUSPLUS
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_RANLIB
AC_PROG_INSTALL

AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [Enable debuging]))
if test "x$enable_debug" == "xyes"; then
	CFLAGS="$CFLAGS -W -Wall"
	CXXFLAGS="$CXXFLAGS -W -Wall"
	LDFLAGS="$LDFLAGS -W -Wall"
else
	LDFLAGS="$LDFLAGS -s"
fi

AC_CHECK_HEADER([inttypes.h], [CPPFLAGS="$CPPFLAGS -DHAVE_INTTYPES_H=1"])
AC_SEARCH_LIBS([sin], [m], [], [AC_ERROR([Library containing sin not found])])

AC_OUTPUT([Makefile])
