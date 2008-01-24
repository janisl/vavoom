AC_INIT([vavoom], [1.26])
AC_CONFIG_SRCDIR([source/gamedefs.h])
AC_CONFIG_AUX_DIR([utils])

AM_INIT_AUTOMAKE([foreign no-define dist-zip dist-bzip2])
AM_MAINTAINER_MODE

dnl
dnl  **** Supported --with arguments ****
dnl

AC_ARG_WITH([allegro], AS_HELP_STRING([--with-allegro], [use Allegro library]))
AC_ARG_WITH([allegro-config], AS_HELP_STRING([--with-allegro-config=<path>], [Set Allegro config script]))
AC_ARG_WITH([sdl], AS_HELP_STRING([--with-sdl], [use SDL library]))
AC_ARG_WITH([sdl-config], AS_HELP_STRING([--with-sdl-config=<path>], [Set SDL config script]))
AC_ARG_WITH([opengl], AS_HELP_STRING([--with-opengl], [use OpenGL library]))
AC_ARG_WITH([openal], AS_HELP_STRING([--with-openal], [use OpenAL library]))
AC_ARG_WITH([openal-config], AS_HELP_STRING([--with-openal-config], [Set OpenAL config script]))
AC_ARG_WITH([external-glbsp], AS_HELP_STRING([--with-external-glbsp], [use external glBSP library]))
AC_ARG_WITH([iwaddir], AS_HELP_STRING([--with-iwaddir], [Specify IWAD directories]))

dnl
dnl  **** Supported --enable arguments ****
dnl

AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [Enable debuging]))
AC_ARG_ENABLE([zone-debug], AS_HELP_STRING([--enable-zone-debug], [Enable memory allocation debuging]))
AC_ARG_ENABLE([client], AS_HELP_STRING([--disable-client], [Don't build client executable]))
AC_ARG_ENABLE([server], AS_HELP_STRING([--disable-server], [Don't build dedicated server executable]))
AC_ARG_ENABLE([asm], AS_HELP_STRING([--disable-asm], [Disable assembly-language code]))
AC_ARG_ENABLE([launcher], AS_HELP_STRING([--disable-launcher], [Don't build launcher]))

dnl
dnl  **** Basic checks ****
dnl

AC_CANONICAL_HOST

AC_LANG_CPLUSPLUS

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_RANLIB
AC_PROG_INSTALL
AM_PROG_AS
AC_PROG_AWK

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
dnl  **** Check for inttypes.h ****
dnl

AC_CHECK_HEADER([inttypes.h], [CPPFLAGS="$CPPFLAGS -DHAVE_INTTYPES_H=1"])

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
dnl  **** Add debug / development flags ****
dnl

if test "x$enable_zone_debug" = "xyes"; then
	CPPFLAGS="$CPPFLAGS -W -Wall -DZONE_DEBUG=1"
fi

dnl
dnl  **** Check for newtwork libraries ****
dnl

case $host in
*-*-msdos* | *-*-go32*)
	;;

*-*-mingw32* | *-*-cygwin* | *-*-windows*)
	LIBS="-lwsock32 $LIBS"
	;;

*)
	AC_SEARCH_LIBS([recvfrom], [socket], [], [AC_ERROR([Library containing recvfrom not found])])
	AC_SEARCH_LIBS([gethostbyname], [nsl], [], [AC_ERROR([Library containing gethostbyname not found])])
	;;
esac

AC_SUBST([SERVER_EXE])
AC_SUBST([SV_CXXFLAGS])
AC_SUBST([SV_CPPFLAGS])
AC_SUBST([SV_LDFLAGS])
AC_SUBST([SV_LIBS])

dnl
dnl  **** Main client library checks ****
dnl

if test "x$enable_client" != "xno"; then

case $host in
dnl ---------------- Windows ----------------
*-*-mingw32* | *-*-cygwin* | *-*-windows*)
	MAIN_EXE='vavoom'
	MAIN_PLATFORM='WIN32'
	LIBS="-lgdi32 -lole32 -lwinmm $LIBS"

	if test "x$with_opengl" != "xno"; then
		MAIN_OPENGL=TRUE
		MAIN_OPENGL_WIN32=TRUE
		LIBS="-lopengl32 $LIBS"
	fi

	if test "x$with_openal" != "xno"; then
		AC_CHECK_HEADER([AL/al.h], [
			MAIN_OPENAL=TRUE
			LIBS="-lOpenAL32 $LIBS"
		])
	fi
	;;

dnl ---------------- Other *NIX platforms ----------------
*)
	MAIN_EXE="vavoom.$host_cpu"

	if test "x$with_allegro" = "xyes"; then
		if test "x$with_allegro_config" = "x"; then
			AC_PATH_PROG([allegro_config], [allegro-config])
			if test "x$allegro_config" = "x"; then
				AC_MSG_ERROR(['cannot find Allegro instalation'])
			fi
		else
			allegro_config=$with_allegro_config
		fi
	else
	if test "x$with_sdl" = "xyes"; then
		if test "x$with_sdl_config" = "x"; then
			AC_PATH_PROG([sdl_config], [sdl-config])
			if test "x$sdl_config" = "x"; then
				AC_MSG_ERROR(['cannot find SDL instalation'])
			fi
		else
			sdl_config=$with_sdl_config
		fi
	else
		if test "x$with_allegro" != "xno"; then
			if test "x$with_allegro_config" = "x"; then
				AC_PATH_PROG([allegro_config], [allegro-config])
			else
				allegro_config=$with_allegro_config
			fi
		fi
		if test "x$with_sdl" != "xno" -a "x$allegro_config" = "x"; then
			if test "x$with_sdl_config" = "x"; then
				AC_PATH_PROG([sdl_config], [sdl-config])
			else
				sdl_config=$with_sdl_config
			fi
		fi
	fi
	fi

	if test "x$allegro_config" != "x"; then
		AC_MSG_NOTICE([Using Allegro])
		MAIN_PLATFORM='UNIX_ALLEGRO'
		CPPFLAGS="$CPPFLAGS `${allegro_config} --cflags`"
		LIBS="`${allegro_config} --libs` $LIBS"
	else
	if test "x$sdl_config" != "x"; then
		AC_MSG_NOTICE([Using SDL])
		MAIN_PLATFORM='UNIX_SDL'
		CPPFLAGS="$CPPFLAGS `${sdl_config} --cflags`"
		LIBS="`${sdl_config} --libs` $LIBS"

		AC_CHECK_HEADER([SDL_mixer.h], [], AC_MSG_ERROR(['Vavoom requires SDL_mixer']))
		AC_CHECK_LIB([SDL_mixer], [Mix_OpenAudio], [], AC_MSG_ERROR(['Vavoom requires SDL_mixer']))
	else
		AC_MSG_ERROR(['Vavoom requires Allegro or SDL to compile'])
	fi
	fi

	AC_CHECK_HEADER([linux/cdrom.h], [
		MAIN_CDAUDIO_LINUX=TRUE
	], [AC_CHECK_HEADER([sys/cdio.h], [
		MAIN_CDAUDIO_BSD=TRUE
	])])

	if test "x$with_opengl" != "xno"; then
		AC_CHECK_HEADER([GL/gl.h], [], [with_opengl=no])
	fi
	if test "x$with_opengl" != "xno"; then
		AC_CHECK_LIB([GL], [glBegin], [], [with_opengl=no])
	fi
	if test "x$with_opengl" != "xno"; then
		MAIN_OPENGL=TRUE
		if test "x$MAIN_PLATFORM" = "xUNIX_ALLEGRO"; then
			AC_CHECK_HEADER([alleggl.h], [], AC_MSG_ERROR(['Vavoom requires AllegroGL to use OpenGL']))
			AC_CHECK_LIB([GLU], [gluProject], [], AC_MSG_ERROR(['Vavoom requires GLU to use OpenGL']))
			AC_CHECK_LIB([agl], [install_allegro_gl], [], AC_MSG_ERROR(['Vavoom requires AllegroGL to use OpenGL']))
			MAIN_OPENGL_UNIX_ALLEGRO=TRUE
		fi
		if test "x$MAIN_PLATFORM" = "xUNIX_SDL"; then
			MAIN_OPENGL_UNIX_SDL=TRUE
		fi
	fi

	if test "x$with_openal" != "xno"; then
		if test "x$with_openal_config" = "x"; then
			AC_PATH_PROG([openal_config], [openal-config])
		else
			openal_config=$with_openal_config
		fi
		if test "x$openal_config" != "x"; then
			MAIN_OPENAL=TRUE
			CPPFLAGS="$CPPFLAGS `$openal_config --cflags`"
			LIBS="`$openal_config --libs` $LIBS"
		fi
	fi
	;;
esac

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

dnl
dnl  **** Check wether using external glBSP ****
dnl

LIB_FILES="timidity/libtimidity.a ../utils/glvis/libglvis.a"

if test "x$with_external_glbsp" != "x" -a "x$with_external_glbsp" != "xno"; then
	if test "x$with_external_glbsp" != "xyes"; then
		CPPFLAGS="$CPPFLAGS -I$with_external_glbsp/include"
		LDFLAGS="$LDFLAGS -L$with_external_glbsp/lib"
	fi
	LIBS="-lglbsp $LIBS"
else
	LIB_FILES="$LIB_FILES ../utils/glbsp/libglbsp.a"
fi

dnl
dnl  **** Main client variables ****
dnl

MAIN_CXXFLAGS=${CXXFLAGS}
MAIN_CPPFLAGS=${CPPFLAGS}
MAIN_LDFLAGS=${LDFLAGS}
MAIN_LIBS=${LIBS}

AC_SUBST([MAIN_EXE])
AC_SUBST([LIB_FILES])
AC_SUBST([MAIN_CXXFLAGS])
AC_SUBST([MAIN_CPPFLAGS])
AC_SUBST([MAIN_LDFLAGS])
AC_SUBST([MAIN_LIBS])

fi  dnl if enabled client

AM_CONDITIONAL(MAIN_PLATFORM_WIN32, test x$MAIN_PLATFORM = xWIN32)
AM_CONDITIONAL(MAIN_PLATFORM_UNIX_ALLEGRO, test x$MAIN_PLATFORM = xUNIX_ALLEGRO)
AM_CONDITIONAL(MAIN_PLATFORM_UNIX_SDL, test x$MAIN_PLATFORM = xUNIX_SDL)
AM_CONDITIONAL(MAIN_ASM_I386, test x$MAIN_ASM_I386 = xTRUE)
AM_CONDITIONAL(MAIN_OPENGL, test x$MAIN_OPENGL = xTRUE)
AM_CONDITIONAL(MAIN_OPENGL_WIN32, test x$MAIN_OPENGL_WIN32 = xTRUE)
AM_CONDITIONAL(MAIN_OPENGL_UNIX_ALLEGRO, test x$MAIN_OPENGL_UNIX_ALLEGRO = xTRUE)
AM_CONDITIONAL(MAIN_OPENGL_UNIX_SDL, test x$MAIN_OPENGL_UNIX_SDL = xTRUE)
AM_CONDITIONAL(MAIN_CDAUDIO_LINUX, test x$MAIN_CDAUDIO_LINUX = xTRUE)
AM_CONDITIONAL(MAIN_CDAUDIO_BSD, test x$MAIN_CDAUDIO_BSD = xTRUE)
AM_CONDITIONAL(MAIN_OPENAL, test x$MAIN_OPENAL = xTRUE)

dnl
dnl  **** Build and instalation targets ****
dnl

AM_CONDITIONAL(ENABLE_CLIENT, test "x$enable_client" != "xno")
AM_CONDITIONAL(ENABLE_SERVER, test "x$enable_server" != "xno")

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
