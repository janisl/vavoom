AC_INIT([vavoom], [1.18])
AC_CONFIG_SRCDIR([source/gamedefs.h])
AC_CONFIG_AUX_DIR([utils])

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_RANLIB
AC_PROG_INSTALL
AC_PROG_MAKE_SET

if test "x$AR" = "x"; then
	AR="ar"
fi
AC_SUBST([AR])

AC_ARG_WITH([allegro], AS_HELP_STRING([--with-allegro], [use Allegro library]))
AC_ARG_WITH([allegro-config], AS_HELP_STRING([--with-allegro-config=<path>], [Set Allegro config script]))
AC_ARG_WITH([sdl], AS_HELP_STRING([--with-sdl], [use SDL library]))
AC_ARG_WITH([sdl-config], AS_HELP_STRING([--with-sdl-config=<path>], [Set SDL config script]))
AC_ARG_WITH([opengl], AS_HELP_STRING([--with-opengl], [use OpenGL library]))
AC_ARG_WITH([openal], AS_HELP_STRING([--with-openal], [use OpenAL library]))
AC_ARG_WITH([openal-config], AS_HELP_STRING([--with-openal-config], [Set OpenAL config script]))

AC_ARG_ENABLE([client], AS_HELP_STRING([--disable-client], [Don't build client executable]))
AC_ARG_ENABLE([server], AS_HELP_STRING([--disable-server], [Don't build dedicated server executable]))

OBJ_FILES='$(COMMON_OBJS)'

case $host in
dnl ---------------- DJGPP ----------------
*-*-msdos* | *-*-go32*)
	LIB_STDCXX='-lstdcxx'
	LIB_MATH='-lm'

	MAIN_EXE='Vavoom$(EXEEXT)'
	OBJ_FILES="$OBJ_FILES \$(SYS_OBJS_DJGPP)"
	MAIN_LIBS='-lalleg -lpng -lz -lstdcxx'

	if test "x$with_opengl" != "xno"; then
		AC_CHECK_HEADER([GL/gl.h], [
			OBJ_FILES="$OBJ_FILES \$(GL_OBJS) \$(GL_SYS_OBJ_DJGPP)"
			MAIN_LIBS="-lGL $MAIN_LIBS"
		])
	fi

	SERVER_EXE='VavoomSV$(EXEEXT)'
	SV_OBJ_FILES='$(SV_COMMON_OBJ_FILES) $(SV_SYS_OBJS_DJGPP)'
	SV_LIBS='-lz -lstdcxx'
	;;

dnl ---------------- Windows ----------------
*-*-mingw32* | *-*-cygwin* | *-*-windows*)
	CFLAGS="-mno-cygwin $CFLAGS"
	CXXFLAGS="-mno-cygwin $CXXFLAGS"
	LIB_STDCXX='-lstdc++'
	LIB_MATH='-lm'

	MAIN_EXE='Vavoom95$(EXEEXT)'
	OBJ_FILES="$OBJ_FILES \$(SYS_OBJS_WIN32)"
	MAIN_LIBS='-lgdi32 -lole32 -lwsock32 -lwinmm -lstdc++'

	if test "x$with_opengl" != "xno"; then
		OBJ_FILES="$OBJ_FILES \$(GL_OBJS) \$(GL_SYS_OBJ_WIN32)"
		MAIN_LIBS="-lopengl32 $MAIN_LIBS"
	fi

	if test "x$with_openal" != "xno"; then
		AC_CHECK_HEADER([AL/al.h], [
			OBJ_FILES="$OBJ_FILES \$(AL_OBJS)"
			MAIN_LIBS="-lOpenAL32 $MAIN_LIBS"
		])
	fi

	SERVER_EXE='Vavm95SV$(EXEEXT)'
	SV_OBJ_FILES='$(SV_COMMON_OBJ_FILES) $(SV_SYS_OBJS_WIN32)'
	SV_LIBS='-lwsock32 -lz -lm -lstdc++'
	;;

dnl ---------------- Other *NIX platforms ----------------
*)
	LIB_STDCXX=-lstdc++

	MAIN_EXE="vavoom.$host_cpu$EXEEXT"

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
		MAIN_CFLAGS="`${allegro_config} --cflags`"
		MAIN_LIBS="`${allegro_config} --libs` -lpng -lz -lstdc++"
	else
	if test "x$sdl_config" != "x"; then
		AC_MSG_NOTICE([Using SDL])
		MAIN_PLATFORM='UNIX_SDL'
		MAIN_CFLAGS="`${sdl_config} --cflags`"
		MAIN_LIBS="-lSDL_mixer `${sdl_config} --libs` -lpng -lz -lstdc++"
	fi
	fi
	OBJ_FILES="$OBJ_FILES \$(SYS_OBJS_${MAIN_PLATFORM})"

	dnl FIXME This sucks a bit.
	VAVOOM_SAVED_LIBS=$LIBS
	LIBS=""
	AC_SEARCH_LIBS([sin], [m], [ LIB_MATH=$LIBS ])
	LIBS=$VAVOOM_SAVED_LIBS

	dnl FIXME This sucks a bit.
	VAVOOM_SAVED_LIBS=$LIBS
	LIBS=$MAIN_LIBS
	AC_SEARCH_LIBS([sin], [m], [], [AC_ERROR([Library containing sin not found])])
	AC_SEARCH_LIBS([recvfrom], [socket], [], [AC_ERROR([Library containing recvfrom not found])])
	AC_SEARCH_LIBS([gethostbyname], [nsl], [], [AC_ERROR([Library containing gethostbyname not found])])
	MAIN_LIBS=$LIBS
	LIBS=$VAVOOM_SAVED_LIBS

	AC_CHECK_HEADER([linux/cdrom.h], [
		OBJ_FILES="$OBJ_FILES \$(CDAUDIO_OBJ_LINUX)"
	], [AC_CHECK_HEADER([sys/cdio.h], [
		OBJ_FILES="$OBJ_FILES \$(CDAUDIO_OBJ_BSD)"
	])])

	if test "x$with_opengl" != "xno"; then
		OBJ_FILES="$OBJ_FILES \$(GL_OBJS) \$(GL_SYS_OBJ_${MAIN_PLATFORM})"
		MAIN_LIBS="-lGL $MAIN_LIBS"
		if test "x$MAIN_PLATFORM" = "xUNIX_ALLEGRO"; then
			MAIN_LIBS="-lagl -lGLU $MAIN_LIBS"
		fi
	fi

	if test "x$with_openal" != "xno"; then
		if test "x$with_openal_config" = "x"; then
			AC_PATH_PROG([openal_config], [openal-config])
		else
			openal_config=$with_openal_config
		fi
		if test "x$openal_config" != "x"; then
			OBJ_FILES="$OBJ_FILES \$(AL_OBJS)"
			MAIN_CFLAGS="$MAIN_CFLAGS `$openal_config --cflags`"
			MAIN_LIBS="`$openal_config --libs` $MAIN_LIBS"
		fi
	fi

	SERVER_EXE="vavoom-dedicated.$host_cpu$EXEEXT"
	SV_OBJ_FILES='$(SV_COMMON_OBJ_FILES) $(SV_SYS_OBJS_UNIX)'
	SV_LIBS='-lz -lstdc++'

	dnl FIXME This sucks a bit.
	VAVOOM_SAVED_LIBS=$LIBS
	LIBS=$SV_LIBS
	AC_SEARCH_LIBS([sin], [m], [], [AC_ERROR([Library containing sin not found])])
	AC_SEARCH_LIBS([recvfrom], [socket], [], [AC_ERROR([Library containing recvfrom not found])])
	AC_SEARCH_LIBS([gethostbyname], [nsl], [], [AC_ERROR([Library containing gethostbyname not found])])
	SV_LIBS=$LIBS
	LIBS=$VAVOOM_SAVED_LIBS
	;;
esac

dnl
dnl Test for prefix prepended by compiler to global symbols.
dnl
AC_MSG_CHECKING([for asm prefix before symbols])
AC_CACHE_VAL(vavoom_cv_asm_prefix,
[
	AC_TRY_LINK([int test_for_underscore(void);
		asm (".globl _test_for_underscore\n"
		"_test_for_underscore:");], [test_for_underscore ();],
		vavoom_cv_asm_prefix=_,
		vavoom_cv_asm_prefix=no)
	if test "$vavoom_cv_asm_prefix" = no; then
		AC_TRY_LINK([int test_for_underscore(void);
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

LIB_FILES="source/timidity/libtimidity.a utils/glvis/libglvis.a"

AC_ARG_WITH([external-glbsp], AS_HELP_STRING([--with-external-glbsp], [use external glBSP library]))
if test "x$with_external_glbsp" = "xyes"; then
	MAIN_LIBS="-lglbsp $MAIN_LIBS"
else
	LIB_FILES="$LIB_FILES utils/glbsp/libglbsp.a"
fi

AC_ARG_WITH([vorbis], AS_HELP_STRING([--with-vorbis], [Enable Vorbis support]))
if test "x$with_vorbis" != "xno"; then
	AC_CHECK_HEADER([vorbis/codec.h], [
		OBJ_FILES="$OBJ_FILES \$(VORBIS_OBJS)"
		MAIN_LIBS="-lvorbis -logg $MAIN_LIBS"
	])
fi

AC_ARG_WITH([libmad], AS_HELP_STRING([--with-libmad], [Enable MP3 support]))
if test "x$with_libmad" != "xno"; then
	AC_CHECK_HEADER([mad.h], [
		OBJ_FILES="$OBJ_FILES \$(MP3_OBJS)"
		MAIN_LIBS="-lmad $MAIN_LIBS"
	])
fi

AC_ARG_WITH([mikmod], AS_HELP_STRING([--with-mikmod], [Enable MikMod support]))
AC_ARG_WITH([mikmod-config], AS_HELP_STRING([--with-mikmod-config], [Set MikMod config script]))
if test "x$with_mikmod" != "xno"; then
	if test "x$with_mikmod_config" = "x"; then
		AC_PATH_PROG([LIBMIKMOD_CONFIG], [libmikmod-config])
	else
		mikmod_config=$with_mikmod_config
	fi
	if test "x$LIBMIKMOD_CONFIG" != "x"; then
		OBJ_FILES="$OBJ_FILES \$(MIKMOD_OBJS)"
		MAIN_CFLAGS="$MAIN_CFLAGS `$LIBMIKMOD_CONFIG --cflags`"
		MAIN_LIBS="`$LIBMIKMOD_CONFIG --libs` $MAIN_LIBS"
	fi
fi

AC_ARG_WITH([flac], AS_HELP_STRING([--with-flac], [Enable FLAC support]))
if test "x$with_flac" != "xno"; then
	AC_LANG_CPLUSPLUS
	AC_CHECK_HEADER([FLAC++/decoder.h], [
		OBJ_FILES="$OBJ_FILES \$(FLAC_OBJS)"
		MAIN_LIBS="-lFLAC++ $MAIN_LIBS"
	])
fi

TARGETS="utils"
INSTALL_TARGETS=""
if test "x$enable_client" != "xno"; then
	TARGETS="$TARGETS exe"
	INSTALL_TARGETS="$INSTALL_TARGETS installexe"
fi
if test "x$enable_server" != "xno"; then
	TARGETS="$TARGETS svexe"
	INSTALL_TARGETS="$INSTALL_TARGETS installsvexe"
fi
TARGETS="$TARGETS data"
INSTALL_TARGETS="$INSTALL_TARGETS installdata"

AC_SUBST([ASMFLAGS])
AC_SUBST([LIB_MATH])
AC_SUBST([LIB_STDCXX])
AC_SUBST([MAIN_EXE])
AC_SUBST([SERVER_EXE])
AC_SUBST([MAIN_CFLAGS])
AC_SUBST([OBJ_FILES])
AC_SUBST([MAIN_LIBS])
AC_SUBST([LIB_FILES])
AC_SUBST([SV_OBJ_FILES])
AC_SUBST([SV_LIBS])
AC_SUBST([TARGETS])
AC_SUBST([INSTALL_TARGETS])

AC_OUTPUT([
	Makefile
	source/timidity/Makefile
	utils/acc/Makefile
	utils/fixmd2/Makefile
	utils/glbsp/Plugin.mak
	utils/glvis/Makefile
	utils/vcc/Makefile
	utils/vlumpy/Makefile])
