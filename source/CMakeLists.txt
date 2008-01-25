add_subdirectory(timidity)

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})

link_directories(../utils/glbsp)
link_directories(../utils/glvis)

#---------------------------------------
#
#	Main executable sources
#
#---------------------------------------

#-- Common files --
set(MAIN_COMMON_SOURCES
	adivtab.h
	am_map.cpp
	anorm_dots.h
	anorms.h
	args.cpp
	args.h
	array.h
	asm_i386.h
	automap.h
	bitstream.cpp
	bitstream.h
	build.h
	chat.cpp
	chat.h
	cheats.cpp
	client.h
	cl_demo.cpp
	cl_input.cpp
	cl_local.h
	cl_main.cpp
	cl_parse.cpp
	cmd.cpp
	cmd.h
	common.h
	console.cpp
	console.h
	crc.cpp
	crc.h
	cvar.cpp
	cvar.h
	d_aclip.cpp
	d_alias.cpp
	d_data.cpp
	d_draw.cpp
	d_edge.cpp
	d_local.h
	d_main.cpp
	d_part.cpp
	d_polyse.cpp
	d_scache.cpp
	d_span.cpp
	d_sprite.cpp
	d_surf.cpp
	d_tex.cpp
	debug.cpp
	debug.h
	decorate.cpp
	decorate.h
	dehacked.cpp
	dehacked.h
	drawer.h
	files.cpp
	files.h
	finale.cpp
	finale.h
	fmd2defs.h
	fs_local.h
	fs_dir.cpp
	fs_wad.cpp
	fs_zip.cpp
	fwaddefs.h
	gamedefs.h
	host.cpp
	host.h
	iline.cpp
	iline.h
	imission.cpp
	imission.h
	in_input.cpp
	infostr.cpp
	infostr.h
	input.h
	l_glbsp.cpp
	l_glvis.cpp
	language.cpp
	language.h
	level.cpp
	level.h
	lockdefs.cpp
	lockdefs.h
	map.h
	mapinfo.cpp
	mapinfo.h
	maths.cpp
	maths.h
	menu.cpp
	menu.h
	misc.cpp
	misc.h
	name.cpp
	name.h
	names.h
	net_channel.cpp
	net_channel_control.cpp
	net_channel_level.cpp
	net_channel_player.cpp
	net_channel_thinker.cpp
	net_connection.cpp
	net_context.cpp
	net_dgrm.cpp
	net_loc.h
	net_loop.cpp
	net_main.cpp
	net_message.cpp
	net_message.h
	network.h
	p_acs.cpp
	p_acs.h
	p_clip.cpp
	p_clip.h
	p_entity.cpp
	p_entity.h
	p_entity_world.cpp
	p_gameinfo.h
	p_levelinfo.cpp
	p_levelinfo.h
	p_nodebuild.cpp
	p_player.cpp
	p_playerreplicationinfo.cpp
	p_playerreplicationinfo.h
	p_polyobj.cpp
	p_setup.cpp
	p_switch.cpp
	p_terrain.cpp
	p_thinker.cpp
	p_thinker.h
	p_trace.cpp
	p_world.cpp
	p_world.h
	p_worldinfo.cpp
	p_worldinfo.h
	player.h
	pr_cmds.cpp
	pr_exec.cpp
	progdefs.h
	progs.h
	r_bsp.cpp
	r_data.cpp
	r_light.cpp
	r_local.h
	r_main.cpp
	r_model.cpp
	r_public.h
	r_shared.h
	r_sky.cpp
	r_surf.cpp
	r_tex.cpp
	r_tex.h
	r_tex_automap.cpp
	r_tex_base.cpp
	r_tex_camera.cpp
	r_tex_flat.cpp
	r_tex_imgz.cpp
	r_tex_jpeg.cpp
	r_tex_multipatch.cpp
	r_tex_patch.cpp
	r_tex_pcx.cpp
	r_tex_png.cpp
	r_tex_raw.cpp
	r_tex_tga.cpp
	r_tex_warp.cpp
	r_things.cpp
	s_data.cpp
	s_local.h
	s_qmus2mid.cpp
	s_sound.cpp
	s_streamplayer.cpp
	s_tmidty.cpp
	s_wav.cpp
	save.h
	sbar.cpp
	sbar.h
	sc_man.cpp
	screen.cpp
	screen.h
	scripts.h
	server.h
	sound.h
	str.cpp
	str.h
	stream.cpp
	stream.h
	sv_local.h
	sv_main.cpp
	sv_save.cpp
	sv_tick.cpp
	sv_user.cpp
	sv_world.cpp
	system.h
	text.cpp
	text.h
	ui.h
	ui_actor.cpp
	ui_font.cpp
	ui_font.h
	ui_root.cpp
	ui_root.h
	ui_widget.cpp
	ui_widget.h
	vclass.cpp
	vclass.h
	vector.h
	video.h
	vobject.cpp
	vobject.h
	wad.cpp
	wad.h
	waddefs.h
	xml.cpp
	xml.h
	zipstream.cpp
	zipstream.h
	zone.cpp
	zone.h
)

#-- i386 assembler files --
set(MAIN_ASM_I386_SOURCES
	d_aclip_i386.s
	d_alias_i386.s
	d_edge_i386.s
	d_polyset_i386.s
	d_vars_i386.s
	d_zspan_i386.s
	d_part_8_i386.s
	d_polyset_8_i386.s
	d_span16_8_i386.s
	d_span_8_i386.s
	d_sprite_8_i386.s
	d_surf_8_i386.s
	d_part_16_i386.s
	d_polyset_16_i386.s
	d_span16_16_i386.s
	d_span_16_i386.s
	d_sprite_16_i386.s
	d_surf_16_i386.s
	d_part_32_i386.s
	d_polyset_32_i386.s
	d_span16_32_i386.s
	d_span_32_i386.s
	d_sprite_32_i386.s
	d_surf_32_i386.s
	sys_i386.s
)

#-- Common OpenGL files --
set(MAIN_OPENGL_SOURCES
	gl_draw.cpp
	gl_local.h
	gl_main.cpp
	gl_poly.cpp
	gl_tex.cpp
)

#-- OpenAL files --
set(MAIN_OPENAL_SOURCES
	s_al.cpp
)

#-- Vorbis files --
set(MAIN_VORBIS_SOURCES
	s_vorbis.cpp
)

#-- MP3 files --
set(MAIN_MP3_SOURCES
	s_mp3.cpp
)

#-- MikMod files --
set(MAIN_MIKMOD_SOURCES
	s_mikmod.cpp
)

#-- FLAC files --
set(MAIN_FLAC_SOURCES
	s_flac.cpp
)

#-- Linux with Allegro --
set(MAIN_UNIX_ALLEGRO_SOURCES
	d_alleg.cpp
	in_alleg.cpp
	net_udp.cpp
	s_alleg.cpp
	s_allegm.cpp
	sys_lin.cpp
)
set(MAIN_OPENGL_UNIX_ALLEGRO_SOURCES
	gl_agl.cpp
)

#-- Linux with SDL --
set(MAIN_UNIX_SDL_SOURCES
	d_sdl.cpp
	in_sdl.cpp
	net_udp.cpp
	s_sdl.cpp
	s_sdlm.cpp
	sys_sdl.cpp
)
set(MAIN_OPENGL_UNIX_SDL_SOURCES
	gl_sdl.cpp
)

#-- CD audio drivers for various UNIX platforms --
set(MAIN_CDAUDIO_LINUX_SOURCES
	cd_linux.cpp
)
set(MAIN_CDAUDIO_BSD_SOURCES
	cd_bsd.cpp
)

#-- Windows --
set(MAIN_WIN32_SOURCES
	cd_win32.cpp
	d_win32.cpp
	d3d_draw.cpp
	d3d_info.cpp
	d3d_local.h
	d3d_main.cpp
	d3d_poly.cpp
	d3d_tex.cpp
	eax.h
	in_win32.cpp
	net_wins.cpp
	s_win32.cpp
	s_win32m.cpp
	sys_win.cpp
	vavoom.ico
	vavoom_2.ico
	vavoom_3.ico
	vavoom.rc
	winlocal.h
)
set(MAIN_OPENGL_WIN32_SOURCES
	gl_win32.cpp
)

#---------------------------------------
#
#	Dedicated server sources
#
#---------------------------------------

set(SERVER_COMMON_SOURCES
	args.cpp
	bitstream.cpp
	cmd.cpp
	crc.cpp
	cvar.cpp
	debug.cpp
	decorate.cpp
	dehacked.cpp
	files.cpp
	fs_dir.cpp
	fs_wad.cpp
	fs_zip.cpp
	host.cpp
	infostr.cpp
	language.cpp
	level.cpp
	lockdefs.cpp
	mapinfo.cpp
	maths.cpp
	misc.cpp
	name.cpp
	net_channel.cpp
	net_channel_control.cpp
	net_channel_level.cpp
	net_channel_player.cpp
	net_channel_thinker.cpp
	net_connection.cpp
	net_context.cpp
	net_dgrm.cpp
	net_loop.cpp
	net_main.cpp
	net_message.cpp
	p_acs.cpp
	p_clip.cpp
	p_entity.cpp
	p_entity_world.cpp
	p_levelinfo.cpp
	p_nodebuild.cpp
	p_player.cpp
	p_playerreplicationinfo.cpp
	p_polyobj.cpp
	p_setup.cpp
	p_switch.cpp
	p_terrain.cpp
	p_thinker.cpp
	p_trace.cpp
	p_world.cpp
	p_worldinfo.cpp
	pr_cmds.cpp
	pr_exec.cpp
	r_data.cpp
	r_tex.cpp
	r_tex_automap.cpp
	r_tex_base.cpp
	r_tex_camera.cpp
	r_tex_flat.cpp
	r_tex_imgz.cpp
	r_tex_jpeg.cpp
	r_tex_multipatch.cpp
	r_tex_patch.cpp
	r_tex_pcx.cpp
	r_tex_png.cpp
	r_tex_raw.cpp
	r_tex_tga.cpp
	r_tex_warp.cpp
	s_data.cpp
	sc_man.cpp
	str.cpp
	stream.cpp
	sv_main.cpp
	sv_save.cpp
	sv_tick.cpp
	sv_user.cpp
	sv_world.cpp
	vclass.cpp
	vobject.cpp
	wad.cpp
	xml.cpp
	zipstream.cpp
	zone.cpp
)

set(SERVER_UNIX_SOURCES
	net_udp.cpp
	sys_bsd.cpp
)

set(SERVER_WIN32_SOURCES
	net_wins.cpp
	sys_wind.cpp
	vavoom.rc
)

#---------------------------------------
#
#	Common checks
#
#---------------------------------------

# CCASFLAGS = -x assembler-with-cpp $(ASMFLAGS)

#
#  **** Check for newtwork libraries ****
#

if(WIN32)
	set(NET_LIBRARIES wsock32)
else(WIN32)
	set(NET_LIBRARIES "")
#FIXME we need this on Solaris
#	AC_SEARCH_LIBS([recvfrom], [socket], [], [AC_ERROR([Library containing recvfrom not found])])
#	AC_SEARCH_LIBS([gethostbyname], [nsl], [], [AC_ERROR([Library containing gethostbyname not found])])
endif(WIN32)

#
#  **** Add debug / development flags ****
#

if(ENABLE_ZONE_DEBUG)
	add_definitions(-DZONE_DEBUG=1)
endif(ENABLE_ZONE_DEBUG)

#---------------------------------------
#
#	Main executable
#
#---------------------------------------

if(NOT DEFINED ENABLE_CLIENT)
	set(ENABLE_CLIENT TRUE)
endif(NOT DEFINED ENABLE_CLIENT)
if(ENABLE_CLIENT)

set(MAIN_SOURCES ${MAIN_COMMON_SOURCES})
set(MAIN_LIBS glbsp glvis timidity ${ZLIB_LIBRARY} ${NET_LIBRARIES})

if(NOT DEFINED WITH_OPENGL)
	set(WITH_OPENGL TRUE)
endif(NOT DEFINED WITH_OPENGL)
if(NOT DEFINED WITH_OPENAL)
	set(WITH_OPENAL TRUE)
endif(NOT DEFINED WITH_OPENAL)

if(WIN32)
	# ---------------- Windows ----------------
	set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_WIN32_SOURCES})
	set(MAIN_LIBS gdi32 ole32 winmm ${MAIN_LIBS})

	if(WITH_OPENGL)
		set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_OPENGL_SOURCES} ${MAIN_OPENGL_WIN32_SOURCES})
		set(MAIN_LIBS opengl32 ${MAIN_LIBS})
	endif(WITH_OPENGL)

	if(WITH_OPENAL)
		find_path(OPENALAL_INCLUDE_DIR AL/al.h)
		set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_OPENAL_SOURCES})
		set(MAIN_LIBS OpenAL32 ${MAIN_LIBS})
	endif(WITH_OPENAL)
else(WIN32)
	# ---------------- Other *NIX platforms ----------------
	if(NOT DEFINED ALLEGRO_CONFIG)
		set(ALLEGRO_CONFIG allegro-config)
	endif(NOT DEFINED ALLEGRO_CONFIG)

	if(WITH_ALLEGRO)
		find_program(ALLEGRO_CONFIG_EXECUTABLE ${ALLEGRO_CONFIG})
		if(NOT ALLEGRO_CONFIG_EXECUTABLE)
			message(FATAL_ERROR "Cannot find Allegro instalation")
		endif(NOT ALLEGRO_CONFIG_EXECUTABLE)
		set(ALLEGRO_FOUND TRUE)
	elseif(WITH_SDL)
		find_package(SDL REQUIRED)
	else(WITH_ALLEGRO)
		find_package(SDL)
		if(NOT SDL_FOUND)
			find_program(ALLEGRO_CONFIG_EXECUTABLE ${ALLEGRO_CONFIG})
			if(ALLEGRO_CONFIG_EXECUTABLE)
				set(ALLEGRO_FOUND TRUE)
			endif(ALLEGRO_CONFIG_EXECUTABLE)
		endif(NOT SDL_FOUND)
	endif(WITH_ALLEGRO)

	if(ALLEGRO_FOUND)
		message(STATUS "Using Allegro")
		# run the allegro-config program to get cxxflags
		exec_program(sh
			ARGS "${LIBALLEGRO_CONFIG_EXECUTABLE} --cflags"
			OUTPUT_VARIABLE ALLEGRO_CXX_FLAGS
			RETURN_VALUE RET)
		if(RET EQUAL 0)
			# parse definitions from cxxflags
			string(REGEX MATCHALL "-D.*[^ ;]+" ALLEGRO_DEFINITIONS ${ALLEGRO_CXX_FLAGS})
			# drop -D* from CXXFLAGS
			string(REGEX REPLACE "-D[^ ;]*" "" ALLEGRO_CXX_FLAGS ${ALLEGRO_CXX_FLAGS})
			# parse incdirs from cxxflags, drop -I prefix
			string(REGEX MATCHALL "-I.*[^ ;]+" ALLEGRO_INCLUDE_DIRS ${ALLEGRO_CXX_FLAGS})
			string(REGEX REPLACE "-I" "" ALLEGRO_INCLUDE_DIRS "${ALLEGRO_CXX_FLAGS}")
			# convert space to semicolons for list
			string(REGEX REPLACE " " ";" ALLEGRO_INCLUDE_DIRS "${ALLEGRO_INCLUDE_DIRS}")
		endif(RET EQUAL 0)

		# run the allegro-config program to get the libs
		exec_program(sh
			ARGS "${LIBALLEGRO_CONFIG_EXECUTABLE} --libs"
			OUTPUT_VARIABLE ALLEGRO_LIBRARIES
			RETURN_VALUE RET)
		if(RET EQUAL 0)
			string(REGEX REPLACE " " ";" ALLEGRO_LIBRARIES "${ALLEGRO_LIBRARIES}")
			# extract linkdirs (-L) for rpath (i.e., LINK_DIRECTORIES)
			string(REGEX MATCHALL "-L[^ ;]+" ALLEGRO_LIBRARY_DIRS "${ALLEGRO_LIBRARIES}")
			string(REGEX REPLACE "-L" "" ALLEGRO_LIBRARY_DIRS "${ALLEGRO_LIBRARY_DIRS}")
			# convert space to semicolons for list
			STRING(REGEX REPLACE " " ";" ALLEGRO_LIBRARY_DIRS "${ALLEGRO_LIBRARY_DIRS}")
		endif(RET EQUAL 0)

		include_directories(${ALLEGRO_INCLUDE_DIRS})
		set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_UNIX_ALLEGRO_SOURCES})
		set(MAIN_LIBS ${ALLEGRO_LIBRARIES} ${MAIN_LIBS})
	elseif(SDL_FOUND)
		message(STATUS "Using SDL")
		find_package(SDL_mixer REQUIRED)
		include_directories(${SDL_INCLUDE_DIR})
		set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_UNIX_SDL_SOURCES})
		set(MAIN_LIBS ${SDLMIXER_LIBRARY} ${SDL_LIBRARY} ${MAIN_LIBS})
	else(ALLEGRO_FOUND)
		mesage(FATAL_ERROR "Vavoom requires Allegro or SDL to compile")
	endif(ALLEGRO_FOUND)

	find_path(LINUX_CDROM_INCLUDE_DIR linux/cdrom.h)
	if(LINUX_CDROM_INCLUDE_DIR)
		message(STATUS "Using LINUX CD-ROM API")
		set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_CDAUDIO_LINUX_SOURCES})
	else(LINUX_CDROM_INCLUDE_DIR)
		find_path(BSD_CDIO_INCLUDE_DIR sys/cdio.h)
		if(BSD_CDIO_INCLUDE_DIR)
			message(STATUS "Using BSD CD-ROM API")
			set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_CDAUDIO_BSD_SOURCES})
		endif(BSD_CDIO_INCLUDE_DIR)
	endif(LINUX_CDROM_INCLUDE_DIR)

	if(WITH_OPENGL)
		find_package(OpenGL)
		if(OPENGL_FOUND)
			message(STATUS "Found OpenGL")
			include_directories(${OPENGL_INCLUDE_DIR})
			set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_OPENGL_SOURCES})
			set(MAIN_LIBS ${OPENGL_gl_LIBRARY} ${MAIN_LIBS})

			if(ALLEGRO_FOUND)
				find_path(ALLEGGL_INCLUDE_DIR alleggl.h)
				if(NOT ALLEGGL_INCLUDE_DIR)
					message(FATAL_ERROR "Vavoom requires AllegroGL to use OpenGL")
				endif(NOT ALLEGGL_INCLUDE_DIR)
				find_library(AGL_LIBRARY NAMES agl)
				if(NOT AGL_LIBRARY)
					message(FATAL_ERROR "Vavoom requires AllegroGL to use OpenGL")
				endif(NOT AGL_LIBRARY)
				set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_OPENGL_UNIX_ALLEGRO_SOURCES})
				set(MAIN_LIBS ${AGL_LIBRARY} ${OPENGL_glu_LIBRARY} ${MAIN_LIBS})
			else(ALLEGRO_FOUND)
				set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_OPENGL_UNIX_SDL_SOURCES})
			endif(ALLEGRO_FOUND)
		endif(OPENGL_FOUND)
	endif(WITH_OPENGL)

	if(WITH_OPENAL)
		find_package(OpenAL)
		if(OPENAL_FOUND)
			message(STATUS "Found OpenAL")
			include_directories(${OPENAL_INCLUDE_DIR})
			set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_OPENAL_SOURCES})
			set(MAIN_LIBS ${OPENAL_LIBRARY} ${MAIN_LIBS})
		endif(OPENAL_FOUND)
	endif(WITH_OPENAL)
endif(WIN32)

#
#  **** Check for libpng ****
#

find_package(PNG REQUIRED)
include_directories(${SDL_INCLUDE_DIR})
set(MAIN_LIBS ${PNG_LIBRARY} ${MAIN_LIBS})

#
#  **** Check for libjpeg ****
#

find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})
set(MAIN_LIBS ${JPEG_LIBRARY} ${MAIN_LIBS})

#
#  **** Check for Vorbis ****
#

if(NOT DEFINED WITH_VORBIS)
	set(WITH_VORBIS TRUE)
endif(NOT DEFINED WITH_VORBIS)
if(WITH_VORBIS)
	find_path(OGG_INCLUDE_DIR ogg/ogg.h)
	find_path(VORBIS_INCLUDE_DIR vorbis/codec.h)
	find_library(OGG_LIBRARY NAMES ogg)
	find_library(VORBIS_LIBRARY NAMES vorbis)
	if (OGG_INCLUDE_DIR AND VORBIS_INCLUDE_DIR AND OGG_LIBRARY AND VORBIS_LIBRARY)
		message(STATUS "Found OggVorbis: ${OGG_LIBRARY} ${VORBIS_LIBRARY}")
		include_directories(${OGG_INCLUDE_DIR})
		include_directories(${VORBIS_INCLUDE_DIR})
		set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_VORBIS_SOURCES})
		set(MAIN_LIBS ${VORBIS_LIBRARY} ${OGG_LIBRARY} ${MAIN_LIBS})
	else (OGG_INCLUDE_DIR AND VORBIS_INCLUDE_DIR AND OGG_LIBRARY AND VORBIS_LIBRARY)
		message(STATUS "Could NOT find OggVorbis libraries")
	endif (OGG_INCLUDE_DIR AND VORBIS_INCLUDE_DIR AND OGG_LIBRARY AND VORBIS_LIBRARY)
endif(WITH_VORBIS)

#
#  **** Check for libmad ****
#

if(NOT DEFINED WITH_LIBMAD)
	set(WITH_LIBMAD TRUE)
endif(NOT DEFINED WITH_LIBMAD)
if(WITH_LIBMAD)
	find_path(MAD_INCLUDE_DIR mad.h)
	find_library(MAD_LIBRARY NAMES mad)
	if(MAD_INCLUDE_DIR AND MAD_LIBRARY)
		message(STATUS "Found LibMAD: ${MAD_LIBRARY}")
		include_directories(${MAD_INCLUDE_DIR})
		set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_MP3_SOURCES})
		set(MAIN_LIBS ${MAD_LIBRARY} ${MAIN_LIBS})
	else(MAD_INCLUDE_DIR AND MAD_LIBRARY)
		message(STATUS "Could NOT find LibMAD libraries")
	endif(MAD_INCLUDE_DIR AND MAD_LIBRARY)
endif(WITH_LIBMAD)

#
#  **** Check for MikMod ****
#

if(NOT DEFINED WITH_MIKMOD)
	set(WITH_MIKMOD TRUE)
endif(NOT DEFINED WITH_MIKMOD)
if(WITH_MIKMOD)
	# Find libmikmod-config script
	if(NOT DEFINED LIBMIKMOD_CONFIG)
		set(LIBMIKMOD_CONFIG libmikmod-config)
	endif(NOT DEFINED LIBMIKMOD_CONFIG)
	find_program(LIBMIKMOD_CONFIG_EXECUTABLE ${LIBMIKMOD_CONFIG})
	if(LIBMIKMOD_CONFIG_EXECUTABLE)
		# run the libmikmod-config program to get cxxflags
		exec_program(sh
			ARGS "${LIBMIKMOD_CONFIG_EXECUTABLE} --cflags"
			OUTPUT_VARIABLE MIKMOD_CXX_FLAGS
			RETURN_VALUE RET)
		if(RET EQUAL 0)
			# parse definitions from cxxflags
			string(REGEX MATCHALL "-D.*[^ ;]+" MIKMOD_DEFINITIONS ${MIKMOD_CXX_FLAGS})
			# drop -D* from CXXFLAGS
			string(REGEX REPLACE "-D[^ ;]*" "" MIKMOD_CXX_FLAGS ${MIKMOD_CXX_FLAGS})
			# parse incdirs from cxxflags, drop -I prefix
			string(REGEX MATCHALL "-I.*[^ ;]+" MIKMOD_INCLUDE_DIRS ${MIKMOD_CXX_FLAGS})
			string(REGEX REPLACE "-I" "" MIKMOD_INCLUDE_DIRS "${MIKMOD_CXX_FLAGS}")
			# convert space to semicolons for list
			string(REGEX REPLACE " " ";" MIKMOD_INCLUDE_DIRS "${MIKMOD_INCLUDE_DIRS}")
		endif(RET EQUAL 0)

		# run the libmikmod-config program to get the libs
		exec_program(sh
			ARGS "${LIBMIKMOD_CONFIG_EXECUTABLE} --libs"
			OUTPUT_VARIABLE MIKMOD_LIBRARIES
			RETURN_VALUE RET)
		if(RET EQUAL 0)
			string(REGEX REPLACE " " ";" MIKMOD_LIBRARIES "${MIKMOD_LIBRARIES}")
			# extract linkdirs (-L) for rpath (i.e., LINK_DIRECTORIES)
			string(REGEX MATCHALL "-L[^ ;]+" MIKMOD_LIBRARY_DIRS "${MIKMOD_LIBRARIES}")
			string(REGEX REPLACE "-L" "" MIKMOD_LIBRARY_DIRS "${MIKMOD_LIBRARY_DIRS}")
			# convert space to semicolons for list
			STRING(REGEX REPLACE " " ";" MIKMOD_LIBRARY_DIRS "${MIKMOD_LIBRARY_DIRS}")
		endif(RET EQUAL 0)

		if(MIKMOD_INCLUDE_DIRS AND MIKMOD_LIBRARIES)
			message(STATUS "Found MIKMOD: ${MIKMOD_LIBRARIES}")
			include_directories(${MIKMOD_INCLUDE_DIRS})
			set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_MIKMOD_SOURCES})
			set(MAIN_LIBS ${MIKMOD_LIBRARIES} ${MAIN_LIBS})
		else(MIKMOD_INCLUDE_DIRS AND MIKMOD_LIBRARIES)
			message(STATUS "Could NOT find MIKMOD libraries")
		endif(MIKMOD_INCLUDE_DIRS AND MIKMOD_LIBRARIES)
	endif(LIBMIKMOD_CONFIG_EXECUTABLE)
endif(WITH_MIKMOD)

#
#  **** Check for FLAC ****
#

if(NOT DEFINED WITH_FLAC)
	set(WITH_FLAC TRUE)
endif(NOT DEFINED WITH_FLAC)
if(WITH_FLAC)
	find_path(FLAC_INCLUDE_DIR FLAC++/decoder.h)
	find_library(FLAC_LIBRARY NAMES FLAC)
	find_library(FLACPP_LIBRARY NAMES FLAC++)
	if(FLAC_INCLUDE_DIR AND FLAC_LIBRARY AND FLACPP_LIBRARY)
		message(STATUS "Found FLAC: ${FLAC_LIBRARY} ${FLACPP_LIBRARY}")
		include_directories(${FLAC_INCLUDE_DIR})
		set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_FLAC_SOURCES})
		set(MAIN_LIBS ${FLACPP_LIBRARY} ${FLAC_LIBRARY} ${MAIN_LIBS})
	else(FLAC_INCLUDE_DIR AND FLAC_LIBRARY AND FLACPP_LIBRARY)
		message(STATUS "Could NOT find FLAC libraries")
	endif(FLAC_INCLUDE_DIR AND FLAC_LIBRARY AND FLACPP_LIBRARY)
endif(WITH_FLAC)

# #-- i386 assembler files --
# if MAIN_ASM_I386
# _____MAIN_EXE__SOURCES += \
# 	d_aclip_i386.s \
# 	d_alias_i386.s \
# 	d_edge_i386.s \
# 	d_polyset_i386.s \
# 	d_vars_i386.s \
# 	d_zspan_i386.s \
# 	d_part_8_i386.s \
# 	d_polyset_8_i386.s \
# 	d_span16_8_i386.s \
# 	d_span_8_i386.s \
# 	d_sprite_8_i386.s \
# 	d_surf_8_i386.s \
# 	d_part_16_i386.s \
# 	d_polyset_16_i386.s \
# 	d_span16_16_i386.s \
# 	d_span_16_i386.s \
# 	d_sprite_16_i386.s \
# 	d_surf_16_i386.s \
# 	d_part_32_i386.s \
# 	d_polyset_32_i386.s \
# 	d_span16_32_i386.s \
# 	d_span_32_i386.s \
# 	d_sprite_32_i386.s \
# 	d_surf_32_i386.s \
# 	sys_i386.s
# endif

add_executable(vavoom ${MAIN_SOURCES})
if(UNIX)
set_target_properties(vavoom PROPERTIES OUTPUT_NAME vavoom.i686)
endif(UNIX)
target_link_libraries(vavoom ${MAIN_LIBS})

# if MAIN_PLATFORM_UNIX_ALLEGRO
# bin_SCRIPTS += vavoom
# endif
# if MAIN_PLATFORM_UNIX_SDL
# bin_SCRIPTS += vavoom
# endif

# vavoom: Makefile
# 	echo "#!/bin/sh" > $@
# 	echo "# Needed to make symlinks/shortcuts work." >> $@
# 	echo "# the binaries must run with correct working directory" >> $@
# 	echo "cd \"$(datadir)/vavoom\"" >> $@
# 	echo "\"$(bindir)/$(MAIN_EXE)\" \$$* $(IWADDIR_PARM)" >> $@
# 	echo "exit \$$?" >> $@
endif(ENABLE_CLIENT)

#---------------------------------------
#
#	Dedicated server
#
#---------------------------------------

if(NOT DEFINED ENABLE_SERVER)
	set(ENABLE_SERVER TRUE)
endif(NOT DEFINED ENABLE_SERVER)
if(ENABLE_SERVER)

if(WIN32)
set(SERVER_SOURCES ${SERVER_COMMON_SOURCES} ${SERVER_WIN32_SOURCES})
else(WIN32)
set(SERVER_SOURCES ${SERVER_COMMON_SOURCES} ${SERVER_UNIX_SOURCES})
endif(WIN32)

add_executable(vavoom-dedicated ${SERVER_SOURCES})
if(UNIX)
set_target_properties(vavoom-dedicated PROPERTIES OUTPUT_NAME vavoom-dedicated.i686)
endif(UNIX)
set_target_properties(vavoom-dedicated PROPERTIES COMPILE_FLAGS -DSERVER)
target_link_libraries(vavoom-dedicated glbsp ${ZLIB_LIBRARY} ${NET_LIBRARIES})

# if SERVER_PLATFORM_UNIX
# bin_SCRIPTS += vavoom-dedicated
# endif

# vavoom-dedicated: Makefile
# 	echo "#!/bin/sh" > $@
# 	echo "# Needed to make symlinks/shortcuts work." >> $@
# 	echo "# the binaries must run with correct working directory" >> $@
# 	echo "cd \"$(datadir)/vavoom\"" >> $@
# 	echo "\"$(bindir)/$(SERVER_EXE)\" \$$* $(IWADDIR_PARM)" >> $@
# 	echo "exit \$$?" >> $@

endif(ENABLE_SERVER)

# # ---------------------------------------
# #
# #	Compilation of Windows resources
# #
# # ---------------------------------------
# 
# .rc.o:
# 	windres --include-dir=$(srcdir) -o $@ -i $<
# 
# # ---------------------------------------
# #
# #	Icon
# #
# # ---------------------------------------
# 
# basedir = $(datadir)/vavoom
# dist_base_DATA = vavoom.png
# 
# # ---------------------------------------
# 
# EXTRA_DIST = \
# 	gas2tasm.c \
# 	gl_x.cpp \
# 	template.cpp \
# 	template.h
# 
# CLEANFILES = $(bin_SCRIPTS)
