project(vavoom)

#
#  **** Check for zlib ****
#

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})

#
#  **** Check for inttypes.h ****
#

find_path(INTTYPES_INCLUDE_DIR inttypes.h)
if(INTTYPES_INCLUDE_DIR)
	add_definitions(-DHAVE_INTTYPES_H=1)
endif(INTTYPES_INCLUDE_DIR)

set(DATADIR share/vavoom)

add_subdirectory(utils)
add_subdirectory(source)
add_subdirectory(progs)
add_subdirectory(basev)
foreach(DIR ${EXTRA_SUBDIRS})
	add_subdirectory(${DIR})
endforeach(DIR)

# EXTRA_DIST = \
# 	autogen.sh \
# 	fixunix.sh \
# 	docs/changes.log \
# 	docs/changes1.log \
# 	docs/changes2.log \
# 	docs/changes3.log \
# 	docs/gnu.txt \
# 	docs/vavoom.txt \
# 	docs/vcc.log \
# 	msvc/Vavoom.sln \
# 	msvc/acc.vcproj \
# 	msvc/libglbsp.vcproj \
# 	msvc/libglvis.vcproj \
# 	msvc/progs_common.vcproj \
# 	msvc/progs_doom.vcproj \
# 	msvc/progs_doom2.vcproj \
# 	msvc/progs_heretic.vcproj \
# 	msvc/progs_hexen.vcproj \
# 	msvc/progs_strife.vcproj \
# 	msvc/timidity.vcproj \
# 	msvc/vavoom.vcproj \
# 	msvc/vcc.vcproj \
# 	msvc/vlumpy.vcproj

# [1.26])
# 
# dnl
# dnl  **** Supported --enable arguments ****
# dnl
# 
# AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [Enable debuging]))
# 
# dnl
# dnl  **** Basic checks ****
# dnl
# 
# if test "x$enable_debug" = "xyes"; then
# 	CFLAGS="$CFLAGS -W -Wall"
# 	CXXFLAGS="$CXXFLAGS -W -Wall"
# 	LDFLAGS="$LDFLAGS -W -Wall"
# fi
# 
# case $host in
# *-*-mingw32* | *-*-cygwin* | *-*-windows*)
# 	CFLAGS="-mno-cygwin $CFLAGS"
# 	CXXFLAGS="-mno-cygwin $CXXFLAGS"
# 	LDFLAGS="-mno-cygwin $LDFLAGS"
# 	;;
# esac
# 
# dnl
# dnl  **** Check wehter we need to use libm ****
# dnl
# 
# AC_SEARCH_LIBS([sin], [m], [], [AC_ERROR([Library containing sin not found])])
