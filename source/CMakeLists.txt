add_subdirectory(timidity)

option(WITH_ALLEGRO "Use Allegro library")
option(WITH_SDL "Use SDL library")
option(WITH_OPENGL "Use OpenGL library" ON)
option(WITH_OPENAL "Use OpenAL library" ON)
option(WITH_VORBIS "Enable Vorbis support" ON)
option(WITH_LIBMAD "Enable MP3 support" ON)
option(WITH_MIKMOD "Enable MikMod support" ON)
option(WITH_FLAC "Enable FLAC support" ON)

option(ENABLE_ZONE_DEBUG "Enable memory allocation debuging")
option(ENABLE_CLIENT "Build client executable" ON)
option(ENABLE_SERVER "Build dedicated server executable" ON)
option(ENABLE_ASM "Enable assembly-language code" ON)
if(UNIX)
option(ENABLE_WRAPPERS "Enable creation of wrapper scripts" ON)
endif(UNIX)

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
	cl_local.h
	cl_main.cpp
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
	net_channel_object_map.cpp
	net_channel_player.cpp
	net_channel_thinker.cpp
	net_connection.cpp
	net_context.cpp
	net_datagram.cpp
	net_local.h
	net_loopback.cpp
	net_main.cpp
	net_message.cpp
	net_message.h
	net_object_map.cpp
	network.h
	p_acs.cpp
	p_acs.h
	p_clip.cpp
	p_clip.h
	p_entity.cpp
	p_entity.h
	p_entity_world.cpp
	p_gameinfo.cpp
	p_gameinfo.h
	p_level_think.cpp
	p_levelinfo.cpp
	p_levelinfo.h
	p_nodebuild.cpp
	p_player.cpp
	p_player.h
	p_player_input.cpp
	p_playerreplicationinfo.cpp
	p_playerreplicationinfo.h
	p_polyobj.cpp
	p_setup.cpp
	p_setup_udmf.cpp
	p_switch.cpp
	p_terrain.cpp
	p_thinker.cpp
	p_thinker.h
	p_trace.cpp
	p_world.cpp
	p_world.h
	p_worldinfo.cpp
	p_worldinfo.h
	pr_cmds.cpp
	pr_exec.cpp
	progdefs.h
	progs.h
	r_bsp.cpp
	r_data.cpp
	r_hardware.cpp
	r_hardware.h
	r_light.cpp
	r_local.h
	r_main.cpp
	r_model.cpp
	r_particle.cpp
	r_portal.cpp
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
	s_reverbs.cpp
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
	vc_class.cpp
	vc_class.h
	vc_constant.cpp
	vc_constant.h
	vc_decorate.cpp
	vc_decorate.h
	vc_dehacked.cpp
	vc_dehacked.h
	vc_emit_context.cpp
	vc_emit_context.h
	vc_error.cpp
	vc_error.h
	vc_expr_array.cpp
	vc_expr_array.h
	vc_expr_assign.cpp
	vc_expr_assign.h
	vc_expr_base.cpp
	vc_expr_base.h
	vc_expr_cast.cpp
	vc_expr_cast.h
	vc_expr_field.cpp
	vc_expr_field.h
	vc_expr_invoke.cpp
	vc_expr_invoke.h
	vc_expr_literal.cpp
	vc_expr_literal.h
	vc_expr_local.cpp
	vc_expr_local.h
	vc_expr_misc.cpp
	vc_expr_misc.h
	vc_expr_type.cpp
	vc_expr_type.h
	vc_expr_unary_binary.cpp
	vc_expr_unary_binary.h
	vc_field.cpp
	vc_field.h
	vc_lexer.cpp
	vc_lexer.h
	vc_location.cpp
	vc_location.h
	vc_member.cpp
	vc_member.h
	vc_method.cpp
	vc_method.h
	vc_modifiers.cpp
	vc_modifiers.h
	vc_object.cpp
	vc_object.h
	vc_package.cpp
	vc_package.h
	vc_parser.cpp
	vc_parser.h
	vc_property.cpp
	vc_property.h
	vc_state.cpp
	vc_state.h
	vc_statement.cpp
	vc_statement.h
	vc_struct.cpp
	vc_struct.h
	vc_type.cpp
	vc_type.h
	vector.h
	video.h
	wad.cpp
	wad.h
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
	d3d_local.h
	d3d_main.cpp
	d3d_poly.cpp
	d3d_tex.cpp
	eax.h
	in_win32.cpp
	net_winsock.cpp
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
	net_channel_object_map.cpp
	net_channel_player.cpp
	net_channel_thinker.cpp
	net_connection.cpp
	net_context.cpp
	net_datagram.cpp
	net_loopback.cpp
	net_main.cpp
	net_message.cpp
	net_object_map.cpp
	p_acs.cpp
	p_clip.cpp
	p_entity.cpp
	p_entity_world.cpp
	p_gameinfo.cpp
	p_level_think.cpp
	p_levelinfo.cpp
	p_nodebuild.cpp
	p_player.cpp
	p_player_input.cpp
	p_playerreplicationinfo.cpp
	p_polyobj.cpp
	p_setup.cpp
	p_setup_udmf.cpp
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
	s_reverbs.cpp
	sc_man.cpp
	str.cpp
	stream.cpp
	sv_main.cpp
	sv_save.cpp
	sv_world.cpp
	vc_class.cpp
	vc_constant.cpp
	vc_decorate.cpp
	vc_dehacked.cpp
	vc_emit_context.cpp
	vc_error.cpp
	vc_expr_array.cpp
	vc_expr_assign.cpp
	vc_expr_base.cpp
	vc_expr_cast.cpp
	vc_expr_field.cpp
	vc_expr_invoke.cpp
	vc_expr_literal.cpp
	vc_expr_local.cpp
	vc_expr_misc.cpp
	vc_expr_type.cpp
	vc_expr_unary_binary.cpp
	vc_field.cpp
	vc_lexer.cpp
	vc_location.cpp
	vc_member.cpp
	vc_method.cpp
	vc_modifiers.cpp
	vc_object.cpp
	vc_package.cpp
	vc_parser.cpp
	vc_property.cpp
	vc_state.cpp
	vc_statement.cpp
	vc_struct.cpp
	vc_type.cpp
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
	net_winsock.cpp
	sys_wind.cpp
	vavoom.rc
)

#---------------------------------------
#
#	Common checks
#
#---------------------------------------

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

if(ENABLE_CLIENT)

set(MAIN_SOURCES ${MAIN_COMMON_SOURCES})
set(MAIN_LIBS glbsp libglvis timidity ${ZLIB_LIBRARY} ${NET_LIBRARIES})
set(MAIN_COMPILE_FLAGS "")

#
#  **** Test for prefix prepended by compiler to global symbols ****
#

file(WRITE ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/temp.c
	"#ifdef __cplusplus
	extern \"C\"
	#endif
	int test_for_underscore();
	asm (\".globl _test_for_underscore\\n\"
	\"_test_for_underscore:\");
	
	int main()
	{
		test_for_underscore();
		return 0;
	}
	")
try_compile(RES ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/temp.c)
if(RES)
	set(ASM_PREFIX "_")
else(RES)
	file(WRITE ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/temp.c
		"#ifdef __cplusplus
		extern \"C\"
		#endif
		int test_for_underscore();
		asm (\".globl test_for_underscore\\n\"
		\"test_for_underscore:\");
		
		int main()
		{
			test_for_underscore();
			return 0;
		}
		")
	try_compile(RES ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/temp.c)
	if(RES)
		set(ASM_PREFIX "")
	else(RES)
		message(STATUS "Unknown ASM prefix")
		set(ASM_PREFIX "")
	endif(RES)
endif(RES)
message(STATUS "ASM prefix: ${ASM_PREFIX}")

if(NOT ${ASM_PREFIX} EQUAL "")
	set(ASMFLAGS "-DASM_PREFIX=${ASM_PREFIX}")
endif(NOT ${ASM_PREFIX} EQUAL "")

#
#  **** Assembly-language files ****
#

if(ENABLE_ASM)
	file(WRITE ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/temp.c
		"#if !defined(__i386__)
		#error Not i386
		#endif
		int main() { return 0; }
		")
	try_compile(IS_I386 ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/temp.c)
	if (IS_I386)
		message(STATUS "Using i386 assembly files")
		set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_ASM_I386_SOURCES})
		set(MAIN_COMPILE_FLAGS ${MAIN_COMPILE_FLAGS} -DUSE_ASM_I386=1)
		#  CMake doesn't support automatic building of assembly files so
		# we must add a custom command and add object files to the list
		# of source files
		foreach(SRC ${MAIN_ASM_I386_SOURCES})
			get_filename_component(FILE_BASE ${SRC} NAME_WE)
			set(OBJ ${CMAKE_CURRENT_BINARY_DIR}/${FILE_BASE}.o)
			add_custom_command(OUTPUT ${OBJ}
				DEPENDS ${SRC} ${CMAKE_CURRENT_SOURCE_DIR}/asm_i386.h
				COMMAND ${CMAKE_C_COMPILER} -x assembler-with-cpp ${ASMFLAGS}
					-c ${CMAKE_CURRENT_SOURCE_DIR}/${SRC} -o ${OBJ}
			)
			set(MAIN_SOURCES ${MAIN_SOURCES} ${OBJ})
		endforeach(SRC)
	endif (IS_I386)
endif(ENABLE_ASM)

#
#  **** Platform specific library checks ****
#

if(WIN32)
	# ---------------- Windows ----------------
	set(MAIN_SOURCES ${MAIN_SOURCES} ${MAIN_WIN32_SOURCES})
	set(MAIN_LIBS gdi32 ole32 winmm ${MAIN_LIBS})

	#  CMake doesn't support automatic building of resource files so
	# we must add a custom command and add object files to the list
	# of source files
	set(RC_SRC ${CMAKE_CURRENT_SOURCE_DIR}/vavoom.rc)
	set(RC_OBJ ${CMAKE_CURRENT_BINARY_DIR}/vavoom.o)
	add_custom_command(OUTPUT ${RC_OBJ}
		DEPENDS ${RC_SRC}
		COMMAND windres --include-dir=${CMAKE_CURRENT_SOURCE_DIR} -o ${RC_OBJ} -i ${RC_SRC}
	)
	set(MAIN_SOURCES ${MAIN_SOURCES} ${RC_OBJ})

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
			ARGS "${ALLEGRO_CONFIG_EXECUTABLE} --cflags"
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
			ARGS "${ALLEGRO_CONFIG_EXECUTABLE} --libs"
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

add_executable(vavoom ${MAIN_SOURCES})
if(ENABLE_WRAPPERS)
set_target_properties(vavoom PROPERTIES OUTPUT_NAME ../vavoom.bin)
else(ENABLE_WRAPPERS)
set_target_properties(vavoom PROPERTIES OUTPUT_NAME ../vavoom)
endif(ENABLE_WRAPPERS)
if (MAIN_COMPILE_FLAGS)
	set_target_properties(vavoom PROPERTIES COMPILE_FLAGS ${MAIN_COMPILE_FLAGS})
endif (MAIN_COMPILE_FLAGS)
target_link_libraries(vavoom ${MAIN_LIBS})
add_dependencies(vavoom timidity glbsp libglvis)

install(TARGETS vavoom DESTINATION ${BINDIR})

#
#	Icon
#
install(FILES vavoom.png DESTINATION ${DATADIR})

if(ENABLE_WRAPPERS)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/vavoom
"#!/bin/sh
# Needed to make symlinks/shortcuts work.
# the binaries must run with correct working directory
\"${CMAKE_INSTALL_PREFIX}/${BINDIR}/vavoom.bin\" $* -basedir \"${CMAKE_INSTALL_PREFIX}/${DATADIR}\" ${IWADDIR}
exit $?
")
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/vavoom DESTINATION ${BINDIR})
endif(ENABLE_WRAPPERS)

endif(ENABLE_CLIENT)

#---------------------------------------
#
#	Dedicated server
#
#---------------------------------------

if(ENABLE_SERVER)

if(WIN32)
	set(SERVER_SOURCES ${SERVER_COMMON_SOURCES} ${SERVER_WIN32_SOURCES})

	#  CMake doesn't support automatic building of resource files so
	# we must add a custom command and add object files to the list
	# of source files
	set(RC_SRC ${CMAKE_CURRENT_SOURCE_DIR}/vavoom.rc)
	set(RC_OBJ ${CMAKE_CURRENT_BINARY_DIR}/vavoom_sv.o)
	add_custom_command(OUTPUT ${RC_OBJ}
		DEPENDS ${RC_SRC}
		COMMAND windres --include-dir=${CMAKE_CURRENT_SOURCE_DIR} -o ${RC_OBJ} -i ${RC_SRC}
	)
	set(SERVER_SOURCES ${SERVER_SOURCES} ${RC_OBJ})
else(WIN32)
	set(SERVER_SOURCES ${SERVER_COMMON_SOURCES} ${SERVER_UNIX_SOURCES})
endif(WIN32)

add_executable(vavoom-dedicated ${SERVER_SOURCES})
if(ENABLE_WRAPPERS)
	set_target_properties(vavoom-dedicated PROPERTIES OUTPUT_NAME ../vavoom-dedicated.bin)
else(ENABLE_WRAPPERS)
	set_target_properties(vavoom-dedicated PROPERTIES OUTPUT_NAME ../vavoom-dedicated)
endif(ENABLE_WRAPPERS)
set_target_properties(vavoom-dedicated PROPERTIES COMPILE_FLAGS -DSERVER)
target_link_libraries(vavoom-dedicated glbsp ${ZLIB_LIBRARY} ${NET_LIBRARIES})
add_dependencies(vavoom-dedicated glbsp)

install(TARGETS vavoom-dedicated DESTINATION ${BINDIR})

if(ENABLE_WRAPPERS)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/vavoom-dedicated
"#!/bin/sh
# Needed to make symlinks/shortcuts work.
# the binaries must run with correct working directory
\"${CMAKE_INSTALL_PREFIX}/${BINDIR}/vavoom-dedicated.bin\" $* -basedir \"${CMAKE_INSTALL_PREFIX}/${DATADIR}\" ${IWADDIR}
exit $?
")
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/vavoom-dedicated DESTINATION ${BINDIR})
endif(ENABLE_WRAPPERS)

endif(ENABLE_SERVER)
