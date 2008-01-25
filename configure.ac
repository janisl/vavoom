AC_INIT([vavoom], [1.26])
AC_CONFIG_SRCDIR([source/gamedefs.h])
AC_CONFIG_AUX_DIR([utils])

AM_INIT_AUTOMAKE([foreign no-define dist-zip dist-bzip2])
AM_MAINTAINER_MODE

dnl
dnl  **** Supported --with arguments ****
dnl

AC_ARG_WITH([iwaddir], AS_HELP_STRING([--with-iwaddir], [Specify IWAD directories]))

dnl
dnl  **** Supported --enable arguments ****
dnl

AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [Enable debuging]))
AC_ARG_ENABLE([asm], AS_HELP_STRING([--disable-asm], [Disable assembly-language code]))

dnl
dnl  **** Basic checks ****
dnl

AC_CANONICAL_HOST

AC_LANG_CPLUSPLUS

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AM_PROG_AS

if test "x$AR" = "x"; then
	AR="ar"
fi
AC_SUBST([AR])

if test "x$enable_debug" = "xyes"; then
	CFLAGS="$CFLAGS -W -Wall"
	CXXFLAGS="$CXXFLAGS -W -Wall"
	LDFLAGS="$LDFLAGS -W -Wall"
fi

case $host in
*-*-mingw32* | *-*-cygwin* | *-*-windows*)
	CFLAGS="-mno-cygwin $CFLAGS"
	CXXFLAGS="-mno-cygwin $CXXFLAGS"
	LDFLAGS="-mno-cygwin $LDFLAGS"
	;;
esac

dnl
dnl  **** Check wehter we need to use libm ****
dnl

AC_SEARCH_LIBS([sin], [m], [], [AC_ERROR([Library containing sin not found])])

dnl
dnl  **** Test for prefix prepended by compiler to global symbols ****
dnl

AC_MSG_CHECKING([for asm prefix before symbols])
AC_CACHE_VAL(vavoom_cv_asm_prefix,
[
	AC_TRY_LINK([#ifdef __cplusplus
		extern "C"
		#endif
		int test_for_underscore();
		asm (".globl _test_for_underscore\n"
		"_test_for_underscore:");], [test_for_underscore ();],
		vavoom_cv_asm_prefix=_,
		vavoom_cv_asm_prefix=no)
	if test "$vavoom_cv_asm_prefix" = no; then
		AC_TRY_LINK([#ifdef __cplusplus
			extern "C"
			#endif
			int test_for_underscore();
			asm (".globl test_for_undercore\n"
			"test_for_underscore:");], [test_for_underscore ();],
			vavoom_cv_asm_prefix=,
			vavoom_cv_asm_prefix=no)
	fi
	if test "$vavoom_cv_asm_prefix" = no; then
		vavoom_cv_asm_prefix=
	fi
])
AC_MSG_RESULT(\"$vavoom_cv_asm_prefix\")
if test "$vavoom_cv_asm_prefix" != ""; then
	ASMFLAGS="-DASM_PREFIX=$vavoom_cv_asm_prefix"
fi

dnl
dnl  **** Assembly-language files ****
dnl

if test "x$enable_asm" != "xno"; then
	case $host in
	i?86-*-*)
		MAIN_ASM_I386=TRUE
		CPPFLAGS="$CPPFLAGS -DUSE_ASM_I386=1"
		;;
	esac
fi

AM_CONDITIONAL(MAIN_ASM_I386, test x$MAIN_ASM_I386 = xTRUE)

dnl
dnl  **** IWAD directory option ****
dnl

if test "x$with_iwaddir" != "x"; then
	IWADDIR_PARM="-iwaddir $with_iwaddir"
fi

AC_SUBST([IWADDIR_PARM])

dnl
dnl  **** Output ****
dnl

AC_SUBST([ASMFLAGS])

dnl
dnl  Possibility to add additional subdirs to the build process that are not
dnl part of distribution.
dnl
AC_SUBST([EXTRA_SUBDIRS])

mkdir -p utils
mkdir -p utils/bin
cd utils && cmake ../${ac_confdir}/utils && cd ..
mkdir -p source
cd source && cmake ../${ac_confdir}/source && cd ..

AC_OUTPUT([
	Makefile
	basev/Makefile
	basev/common/Makefile
	basev/common/basepak.ls
	basev/doom/Makefile
	basev/doom/basepak.ls
	basev/doom1/Makefile
	basev/doom1/basepak.ls
	basev/doom2/Makefile
	basev/doom2/basepak.ls
	basev/heretic/Makefile
	basev/heretic/basepak.ls
	basev/hexen/Makefile
	basev/hexen/basepak.ls
	basev/plutonia/Makefile
	basev/plutonia/basepak.ls
	basev/strife/Makefile
	basev/strife/basepak.ls
	basev/tnt/Makefile
	basev/tnt/basepak.ls
	progs/Makefile
	progs/common/Makefile
	progs/doom/Makefile
	progs/doom2/Makefile
	progs/heretic/Makefile
	progs/hexen/Makefile
	progs/strife/Makefile])
