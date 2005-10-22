AC_INIT([vavoom], [1.18])
AC_CONFIG_SRCDIR([source/gamedefs.h])
AC_CONFIG_AUX_DIR([utils])

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_RANLIB
AC_PROG_INSTALL

if test "x$AR" = "x"; then
	AR="ar"
fi
AC_SUBST([AR])

AC_ARG_WITH([allegro], AS_HELP_STRING([--with-allegro], [use Allegro library]))
AC_ARG_WITH([sdl], AS_HELP_STRING([--with-sdl], [use SDL library]))
AC_ARG_WITH([opengl], AS_HELP_STRING([--with-opengl], [use OpenGL library]))
AC_ARG_WITH([openal], AS_HELP_STRING([--with-openal], [use OpenAL library]))

OBJ_FILES='$(COMMON_OBJS)'

case $host in
dnl ---------------- DJGPP ----------------
*-*-msdos* | *-*-go32*)
	LIB_STDCXX='-lstdcxx'

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
	SV_LIBS='-lstdcxx'
	;;

dnl ---------------- Windows ----------------
*-*-mingw32* | *-*-cygwin* | *-*-windows*)
	CFLAGS="-mno-cygwin $CFLAGS"
	CXXFLAGS="-mno-cygwin $CXXFLAGS"
	LIB_STDCXX='-lstdc++'

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
	SV_LIBS='-lwsock32 -lm -lstdc++'
	;;

dnl ---------------- Other *NIX platforms ----------------
*)
	LIB_STDCXX=-lstdc++

	MAIN_EXE='Vavoom$(EXEEXT)'

	if test "x$with_allegro" = "xyes"; then
		AC_PATH_PROG([allegro_config], [allegro-config])
		if test "x$allegro_config" = "x"; then
			AC_MSG_ERROR(['cannot find Allegro instalation'])
		fi
	else
	if test "x$with_sdl" = "xyes"; then
		AC_PATH_PROG([sdl_config], [sdl-config])
		if test "x$sdl_config" = "x"; then
			AC_MSG_ERROR(['cannot find SDL instalation'])
		fi
	else
		if test "x$with_allegro" != "xno"; then
			AC_PATH_PROG([allegro_config], [allegro-config])
		fi
		if test "x$with_sdl" != "xno" -a "x$allegro_config" = "x"; then
			AC_PATH_PROG([sdl_config], [sdl-config])
		fi
	fi
	fi

	if test "x$allegro_config" != "x"; then
		MAIN_PLATFORM='UNIX_ALLEGRO'
		MAIN_CFLAGS="`${allegro_config} --cflags`"
		MAIN_LIBS="`${allegro_config} --libs` -lpng -lz -lm -lstdc++"
	else
	if test "x$sdl_config" != "x"; then
		MAIN_PLATFORM='UNIX_SDL'
		MAIN_CFLAGS="`${sdl_config} --cflags`"
		MAIN_LIBS="-lSDL_mixer `${sdl_config} --libs` -lpng -lz -lm -lstdc++"
	fi
	fi
	OBJ_FILES="$OBJ_FILES \$(SYS_OBJS_${MAIN_PLATFORM})"

	AC_CHECK_HEADER([linux/cdrom.h], [
		OBJ_FILES="$OBJ_FILES \$(CDAUDIO_OBJ_LINUX)"
	], [AC_CHECK_HEADER([sys/cdrom.h], [
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
		AC_PATH_PROG([openal_config], [openal-config])
		if test "x$openal_config" != "x"; then
			OBJ_FILES="$OBJ_FILES \$(AL_OBJS)"
			MAIN_CFLAGS="$MAIN_CFLAGS `$openal_config --cflags`"
			MAIN_LIBS="`$openal_config --libs` $MAIN_LIBS"
		fi
	fi

	SERVER_EXE='VavoomSV$(EXEEXT)'
	SV_OBJ_FILES='$(SV_COMMON_OBJ_FILES) $(SV_SYS_OBJS_UNIX)'
	SV_LIBS='-lm -lstdc++'
	;;
esac

AC_ARG_WITH([external-glbsp], AS_HELP_STRING([--with-external-glbsp], [use external glBSP library]))
if test "x$with_external_glbsp" = "xyes"; then
	MAIN_LIBS="-lglbsp $MAIN_LIBS"
	LIB_FILES="utils/glvis/libglvis.a"
else
	LIB_FILES="utils/glbsp/libglbsp.a utils/glvis/libglvis.a"
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

AC_SUBST([LIB_STDCXX])
AC_SUBST([MAIN_EXE])
AC_SUBST([SERVER_EXE])
AC_SUBST([MAIN_CFLAGS])
AC_SUBST([OBJ_FILES])
AC_SUBST([MAIN_LIBS])
AC_SUBST([LIB_FILES])
AC_SUBST([SV_OBJ_FILES])
AC_SUBST([SV_LIBS])

AC_OUTPUT([
	Makefile
	utils/acc/Makefile
	utils/fixmd2/Makefile
	utils/glbsp/Plugin.mak
	utils/glvis/Makefile
	utils/vcc/Makefile
	utils/vlumpy/Makefile])
