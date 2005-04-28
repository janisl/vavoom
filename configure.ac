AC_INIT([vavoom], [1.18])
AC_CONFIG_SRCDIR([source/gamedefs.h])
AC_CONFIG_AUX_DIR([utils])

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_INSTALL

AR="ar"
AC_SUBST([AR])

# Uncomment if for OpenGL you must link to libMesaGL.a and not to libGL.a
#MESAGL = 1

AC_ARG_WITH([allegro], AS_HELP_STRING([--with-allegro], [use Allegro library]))
AC_ARG_WITH([sdl], AS_HELP_STRING([--with-sdl], [use SDL library]))
AC_ARG_WITH([opengl], AS_HELP_STRING([--with-opengl], [use OpenGL library]))
AC_ARG_WITH([openal], AS_HELP_STRING([--with-openal], [use OpenAL library]))

OBJ_FILES='$(COMMON_OBJS)'

case $host in
	*-*-msdos* | *-*-go32*)
		LIB_STDCXX='-lstdcxx'
		MAIN_EXE='Vavoom$(EXEEXT)'
		SERVER_EXE='VavoomSV$(EXEEXT)'
		LIB_GL='-lGL'
		MAIN_PLATFORM='DJGPP'
		SV_PLATFORM='DJGPP'
		OBJ_FILES="$OBJ_FILES \$(SYS_OBJS_DJGPP)"
		MAIN_LIBS='$(LIBS_DJGPP)'
		;;

	*-*-mingw32* | *-*-cygwin* | *-*-windows*)
		CFLAGS="-mno-cygwin $CFLAGS"
		CXXFLAGS="-mno-cygwin $CXXFLAGS"
		LIB_STDCXX='-lstdc++'
		MAIN_EXE='Vavoom95$(EXEEXT)'
		SERVER_EXE='Vavm95SV$(EXEEXT)'
		LIB_GL='-lopengl32'
		MAIN_PLATFORM='WIN32'
		SV_PLATFORM='WIN32'
		OBJ_FILES="$OBJ_FILES \$(SYS_OBJS_WIN32)"
		MAIN_LIBS='$(LIBS_WIN32)'
		;;

	*)
		LIB_STDCXX=-lstdc++
		MAIN_EXE='Vavoom$(EXEEXT)'
		SERVER_EXE='VavoomSV$(EXEEXT)'
		SV_PLATFORM='UNIX'

dnl ifdef MESAGL
dnl LIB_GL = -lMesaGL
dnl else
		LIB_GL='-lGL'
dnl endif

		if test "x$with_sdl" = "xyes"; then
			MAIN_PLATFORM='UNIX_SDL'
			MAIN_CFLAGS=`sdl-config --cflags`
			MAIN_LIBS='$(LIBS_UNIX_SDL)'
		else
			MAIN_PLATFORM='UNIX_ALLEGRO'
			MAIN_CFLAGS=`allegro-config --cflags`
			MAIN_LIBS='$(LIBS_UNIX_ALLEGRO)'
		fi
		OBJ_FILES="$OBJ_FILES \$(SYS_OBJS_${MAIN_PLATFORM})"

		AC_CHECK_HEADER([linux/cdrom.h], [
			OBJ_FILES="$OBJ_FILES \$(CDAUDIO_OBJ_LINUX)"
		], [AC_CHECK_HEADER([sys/cdrom.h], [
			OBJ_FILES="$OBJ_FILES \$(CDAUDIO_OBJ_BSD)"
		], [
			OBJ_FILES="$OBJ_FILES \$(CDAUDIO_OBJ_NONE)"
		])])

		if test "x$with_opengl" != "xno"; then
			OBJ_FILES="$OBJ_FILES \$(GL_OBJS) \$(GL_SYS_OBJ)"
			MAIN_LIBS="\$(GL_SYS_LIB) \$(LIB_GL) $MAIN_LIBS"
		fi

		if test "x$with_openal" != "xno"; then
			AC_PATH_PROG([openal_config], [openal-config])
			if test "x$openal_config" != "x"; then
				OBJ_FILES="$OBJ_FILES \$(AL_OBJS)"
				MAIN_CFLAGS="$MAIN_CFLAGS `$openal_config --cflags`"
				MAIN_LIBS="`$openal_config --libs` $MAIN_LIBS"
			fi
		fi
		;;
esac

AC_ARG_WITH([external-glbsp], AS_HELP_STRING([--with-external-glbsp], [use external glBSP library]))
if test "x$with_external_glbsp" = "xyes"; then
	MAIN_LIBS="-lglbsp $MAIN_LIBS"
	LIB_FILES="utils/glvis/libglvis.a"
else
	LIB_FILES="utils/glbsp/libglbsp.a utils/glvis/libglvis.a"
fi

AC_SUBST([LIB_STDCXX])
AC_SUBST([MAIN_EXE])
AC_SUBST([SERVER_EXE])
AC_SUBST([LIB_GL])
AC_SUBST([MAIN_PLATFORM])
AC_SUBST([SV_PLATFORM])
AC_SUBST([MAIN_CFLAGS])
AC_SUBST([OBJ_FILES])
AC_SUBST([MAIN_LIBS])
AC_SUBST([LIB_FILES])

AC_OUTPUT([
	Makefile
	utils/acc/Makefile
	utils/glbsp/Plugin.mak
	utils/glvis/Makefile
	utils/vcc/Makefile
	utils/vlumpy/Makefile])
