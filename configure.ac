AC_INIT([vavoom], [1.24])
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
AC_ARG_WITH([vorbis], AS_HELP_STRING([--with-vorbis], [Enable Vorbis support]))
AC_ARG_WITH([libmad], AS_HELP_STRING([--with-libmad], [Enable MP3 support]))
AC_ARG_WITH([mikmod], AS_HELP_STRING([--with-mikmod], [Enable MikMod support]))
AC_ARG_WITH([mikmod-config], AS_HELP_STRING([--with-mikmod-config], [Set MikMod config script]))
AC_ARG_WITH([flac], AS_HELP_STRING([--with-flac], [Enable FLAC support]))
AC_ARG_WITH([iwaddir], AS_HELP_STRING([--with-iwaddir], [Specify IWAD directories]))

dnl
dnl  **** Supported --enable arguments ****
dnl

AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [Enable debuging]))
AC_ARG_ENABLE([zone-debug], AS_HELP_STRING([--enable-zone-debug], [Enable memory allocation debuging]))
AC_ARG_ENABLE([client], AS_HELP_STRING([--disable-client], [Don't build client executable]))
AC_ARG_ENABLE([server], AS_HELP_STRING([--disable-server], [Don't build dedicated server executable]))
AC_ARG_ENABLE([asm], AS_HELP_STRING([--disable-asm], [Disable assembly-language code]))

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
dnl  **** ACC variables ****
dnl

ACC_CFLAGS=${CFLAGS}
ACC_CPPFLAGS=${CPPFLAGS}
ACC_LDFLAGS=${LDFLAGS}
ACC_LIBS=${LIBS}

AC_SUBST([ACC_CFLAGS])
AC_SUBST([ACC_CPPFLAGS])
AC_SUBST([ACC_LDFLAGS])
AC_SUBST([ACC_LIBS])

dnl
dnl  **** FixMD2 variables ****
dnl

FIXMD2_CXXFLAGS=${CXXFLAGS}
FIXMD2_CPPFLAGS=${CPPFLAGS}
FIXMD2_LDFLAGS=${LDFLAGS}
FIXMD2_LIBS=${LIBS}

AC_SUBST([FIXMD2_CXXFLAGS])
AC_SUBST([FIXMD2_CPPFLAGS])
AC_SUBST([FIXMD2_LDFLAGS])
AC_SUBST([FIXMD2_LIBS])

dnl
dnl  **** VCC variables ****
dnl

VCC_CXXFLAGS=${CXXFLAGS}
VCC_CPPFLAGS=${CPPFLAGS}
VCC_LDFLAGS=${LDFLAGS}
VCC_LIBS=${LIBS}

AC_SUBST([VCC_CXXFLAGS])
AC_SUBST([VCC_CPPFLAGS])
AC_SUBST([VCC_LDFLAGS])
AC_SUBST([VCC_LIBS])

dnl
dnl  **** Timidity variables ****
dnl

TIMIDITY_CXXFLAGS=${CXXFLAGS}
TIMIDITY_CPPFLAGS=${CPPFLAGS}

AC_SUBST([TIMIDITY_CXXFLAGS])
AC_SUBST([TIMIDITY_CPPFLAGS])

dnl
dnl  **** Check wehter we need to use libm ****
dnl

AC_SEARCH_LIBS([sin], [m], [], [AC_ERROR([Library containing sin not found])])

dnl
dnl  **** glVIS variables ****
dnl

GLVIS_CXXFLAGS=${CXXFLAGS}
GLVIS_CPPFLAGS=${CPPFLAGS}
GLVIS_LDFLAGS=${LDFLAGS}
GLVIS_LIBS=${LIBS}

AC_SUBST([GLVIS_CXXFLAGS])
AC_SUBST([GLVIS_CPPFLAGS])
AC_SUBST([GLVIS_LDFLAGS])
AC_SUBST([GLVIS_LIBS])

dnl
dnl  **** Check for zlib ****
dnl

AC_CHECK_HEADER([zlib.h], [], AC_MSG_ERROR(['Vavoom requires zlib to compile']))
AC_CHECK_LIB([z], [deflate], [], AC_MSG_ERROR(['Vavoom requires zlib to compile']))

dnl
dnl  **** glBSP variables ****
dnl

GLBSP_CFLAGS=${CFLAGS}
GLBSP_CPPFLAGS=${CPPFLAGS}
GLBSP_LDFLAGS=${LDFLAGS}
GLBSP_LIBS=${LIBS}

AC_SUBST([GLBSP_CFLAGS])
AC_SUBST([GLBSP_CPPFLAGS])
AC_SUBST([GLBSP_LDFLAGS])
AC_SUBST([GLBSP_LIBS])

dnl
dnl  **** VLumpy variables ****
dnl

VLUMPY_CXXFLAGS=${CXXFLAGS}
VLUMPY_CPPFLAGS=${CPPFLAGS}
VLUMPY_LDFLAGS=${LDFLAGS}
VLUMPY_LIBS=${LIBS}

AC_SUBST([VLUMPY_CXXFLAGS])
AC_SUBST([VLUMPY_CPPFLAGS])
AC_SUBST([VLUMPY_LDFLAGS])
AC_SUBST([VLUMPY_LIBS])

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

dnl
dnl  **** Server variables ****
dnl

case $host in
*-*-msdos* | *-*-go32*)
	SERVER_EXE='VavoomSV'
	SERVER_PLATFORM=DJGPP
	;;

*-*-mingw32* | *-*-cygwin* | *-*-windows*)
	SERVER_EXE='Vavm95SV'
	SERVER_PLATFORM=WIN32
	;;

*)
	SERVER_EXE="vavoom-dedicated.$host_cpu"
	SERVER_PLATFORM=UNIX
	;;
esac
AM_CONDITIONAL(SERVER_PLATFORM_DJGPP, test x$SERVER_PLATFORM = xDJGPP)
AM_CONDITIONAL(SERVER_PLATFORM_WIN32, test x$SERVER_PLATFORM = xWIN32)
AM_CONDITIONAL(SERVER_PLATFORM_UNIX, test x$SERVER_PLATFORM = xUNIX)

SV_CXXFLAGS=${CXXFLAGS}
SV_CPPFLAGS=${CPPFLAGS}
SV_LDFLAGS=${LDFLAGS}
SV_LIBS=${LIBS}

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
dnl ---------------- DJGPP ----------------
*-*-msdos* | *-*-go32*)
	MAIN_EXE='Vavoom'
	MAIN_PLATFORM='DJGPP'

	AC_CHECK_HEADER([allegro.h], [], AC_MSG_ERROR(['Vavoom requires Allegro to compile']))
	AC_CHECK_LIB([alleg], [allegro_exit], [], AC_MSG_ERROR(['Vavoom requires Allegro to compile']))

	if test "x$with_opengl" != "xno"; then
		AC_CHECK_HEADER([GL/gl.h], [
			MAIN_OPENGL=TRUE
			MAIN_OPENGL_DJGPP=TRUE
			LIBS="-lGL $LIBS"
		])
	fi
	;;

dnl ---------------- Windows ----------------
*-*-mingw32* | *-*-cygwin* | *-*-windows*)
	MAIN_EXE='Vavoom95'
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
dnl  **** Check for libpng ****
dnl

AC_CHECK_HEADER([png.h], [], AC_MSG_ERROR(['Vavoom requires libpng to compile']))
AC_CHECK_LIB([png], [png_read_image], [], AC_MSG_ERROR(['Vavoom requires libpng to compile']))

dnl
dnl  **** Check for libjpeg ****
dnl

AC_CHECK_HEADER([jpeglib.h], [], AC_MSG_ERROR(['Vavoom requires libjpeg to compile']))
AC_CHECK_LIB([jpeg], [jpeg_read_header], [], AC_MSG_ERROR(['Vavoom requires libjpeg to compile']))

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
dnl  **** Check for Vorbis ****
dnl

if test "x$with_vorbis" != "xno"; then
	AC_CHECK_HEADER([ogg/ogg.h], [], [with_vorbis=no])
fi
if test "x$with_vorbis" != "xno"; then
	AC_CHECK_HEADER([vorbis/codec.h], [], [with_vorbis=no])
fi
if test "x$with_vorbis" != "xno"; then
	AC_CHECK_LIB([ogg], [ogg_sync_init], [], [with_vorbis=no])
fi
if test "x$with_vorbis" != "xno"; then
	AC_CHECK_LIB([vorbis], [vorbis_info_init], [], [with_vorbis=no])
fi
if test "x$with_vorbis" != "xno"; then
	MAIN_VORBIS=TRUE
fi

dnl
dnl  **** Check for libmad ****
dnl

if test "x$with_libmad" != "xno"; then
	AC_CHECK_HEADER([mad.h], [], [with_libmad=no])
fi
if test "x$with_libmad" != "xno"; then
	AC_CHECK_LIB([mad], [mad_stream_init], [], [with_libmad=no])
fi
if test "x$with_libmad" != "xno"; then
	MAIN_MP3=TRUE
fi

dnl
dnl  **** Check for MikMod ****
dnl

if test "x$with_mikmod" != "xno"; then
	if test "x$with_mikmod_config" = "x"; then
		AC_PATH_PROG([LIBMIKMOD_CONFIG], [libmikmod-config])
	else
		mikmod_config=$with_mikmod_config
	fi
	if test "x$LIBMIKMOD_CONFIG" != "x"; then
		MAIN_MIKMOD=TRUE
		CXXFLAGS="$CXXFLAGS `$LIBMIKMOD_CONFIG --cflags`"
		LDFLAGS="$LDFLAGS `$LIBMIKMOD_CONFIG --ldadd`"
		LIBS="`$LIBMIKMOD_CONFIG --libs` $LIBS"
	fi
fi

dnl
dnl  **** Check for FLAC ****
dnl

if test "x$with_flac" != "xno"; then
	AC_CHECK_HEADER([FLAC++/decoder.h], [], [with_flac=no])
fi
if test "x$with_flac" != "xno"; then
	MAIN_FLAC=TRUE
	LIBS="-lFLAC++ -lFLAC $LIBS"
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

AM_CONDITIONAL(MAIN_PLATFORM_DJGPP, test x$MAIN_PLATFORM = xDJGPP)
AM_CONDITIONAL(MAIN_PLATFORM_WIN32, test x$MAIN_PLATFORM = xWIN32)
AM_CONDITIONAL(MAIN_PLATFORM_UNIX_ALLEGRO, test x$MAIN_PLATFORM = xUNIX_ALLEGRO)
AM_CONDITIONAL(MAIN_PLATFORM_UNIX_SDL, test x$MAIN_PLATFORM = xUNIX_SDL)
AM_CONDITIONAL(MAIN_ASM_I386, test x$MAIN_ASM_I386 = xTRUE)
AM_CONDITIONAL(MAIN_OPENGL, test x$MAIN_OPENGL = xTRUE)
AM_CONDITIONAL(MAIN_OPENGL_DJGPP, test x$MAIN_OPENGL_DJGPP = xTRUE)
AM_CONDITIONAL(MAIN_OPENGL_WIN32, test x$MAIN_OPENGL_WIN32 = xTRUE)
AM_CONDITIONAL(MAIN_OPENGL_UNIX_ALLEGRO, test x$MAIN_OPENGL_UNIX_ALLEGRO = xTRUE)
AM_CONDITIONAL(MAIN_OPENGL_UNIX_SDL, test x$MAIN_OPENGL_UNIX_SDL = xTRUE)
AM_CONDITIONAL(MAIN_CDAUDIO_LINUX, test x$MAIN_CDAUDIO_LINUX = xTRUE)
AM_CONDITIONAL(MAIN_CDAUDIO_BSD, test x$MAIN_CDAUDIO_BSD = xTRUE)
AM_CONDITIONAL(MAIN_OPENAL, test x$MAIN_OPENAL = xTRUE)
AM_CONDITIONAL(MAIN_VORBIS, test x$MAIN_VORBIS = xTRUE)
AM_CONDITIONAL(MAIN_MP3, test x$MAIN_MP3 = xTRUE)
AM_CONDITIONAL(MAIN_MIKMOD, test x$MAIN_MIKMOD = xTRUE)
AM_CONDITIONAL(MAIN_FLAC, test x$MAIN_FLAC = xTRUE)

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

AC_OUTPUT([
	Makefile
	basev/Makefile
	basev/common/basepak.ls
	basev/doom/basepak.ls
	basev/doom1/basepak.ls
	basev/doom2/basepak.ls
	basev/heretic/basepak.ls
	basev/hexen/basepak.ls
	basev/plutonia/basepak.ls
	basev/strife/basepak.ls
	basev/tnt/basepak.ls
	progs/Makefile
	progs/common/Makefile
	progs/doom/Makefile
	progs/doom2/Makefile
	progs/heretic/Makefile
	progs/hexen/Makefile
	progs/strife/Makefile
	source/Makefile
	source/timidity/Makefile
	utils/Makefile
	utils/acc/Makefile
	utils/fixmd2/Makefile
	utils/glbsp/Makefile
	utils/glvis/Makefile
	utils/vcc/Makefile
	utils/vlumpy/Makefile])
