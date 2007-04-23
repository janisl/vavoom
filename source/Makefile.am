SUBDIRS = timidity

CPPFLAGS =
CXXFLAGS =
DEFS =
LDFLAGS =
LIBS =

CCASFLAGS = -x assembler-with-cpp $(ASMFLAGS)

bin_PROGRAMS =
bin_SCRIPTS =

#---------------------------------------
#
#	Main executable
#
#---------------------------------------

if ENABLE_CLIENT
bin_PROGRAMS += ../$(MAIN_EXE)
if MAIN_PLATFORM_UNIX_ALLEGRO
bin_SCRIPTS += vavoom
endif
if MAIN_PLATFORM_UNIX_SDL
bin_SCRIPTS += vavoom
endif
endif

_____MAIN_EXE__CPPFLAGS = @MAIN_CPPFLAGS@
_____MAIN_EXE__CXXFLAGS = @MAIN_CXXFLAGS@
_____MAIN_EXE__DEFS =
_____MAIN_EXE__LDFLAGS = @MAIN_LDFLAGS@
_____MAIN_EXE__LDADD = @LIB_FILES@ @MAIN_LIBS@

#-- Common files --
_____MAIN_EXE__SOURCES = \
	adivtab.h \
	am_map.cpp \
	anorm_dots.h \
	anorms.h \
	args.cpp \
	args.h \
	array.h \
	asm_i386.h \
	automap.h \
	bitstream.cpp \
	bitstream.h \
	build.h \
	chat.cpp \
	chat.h \
	cheats.cpp \
	client.h \
	cl_demo.cpp \
	cl_input.cpp \
	cl_local.h \
	cl_main.cpp \
	cl_parse.cpp \
	cmd.cpp \
	cmd.h \
	common.h \
	console.cpp \
	console.h \
	crc.cpp \
	crc.h \
	cvar.cpp \
	cvar.h \
	d_aclip.cpp \
	d_alias.cpp \
	d_data.cpp \
	d_draw.cpp \
	d_edge.cpp \
	d_local.h \
	d_main.cpp \
	d_part.cpp \
	d_polyse.cpp \
	d_scache.cpp \
	d_span.cpp \
	d_sprite.cpp \
	d_surf.cpp \
	d_tex.cpp \
	debug.cpp \
	debug.h \
	dehacked.cpp \
	drawer.h \
	files.cpp \
	files.h \
	finale.cpp \
	finale.h \
	fmd2defs.h \
	fs_local.h \
	fs_dir.cpp \
	fs_wad.cpp \
	fs_zip.cpp \
	fwaddefs.h \
	gamedefs.h \
	host.cpp \
	host.h \
	iline.cpp \
	iline.h \
	imission.cpp \
	imission.h \
	in_input.cpp \
	infostr.cpp \
	infostr.h \
	input.h \
	l_glbsp.cpp \
	l_glvis.cpp \
	language.cpp \
	language.h \
	level.cpp \
	level.h \
	map.h \
	mapinfo.cpp \
	mapinfo.h \
	maths.cpp \
	maths.h \
	menu.cpp \
	menu.h \
	message.cpp \
	message.h \
	misc.cpp \
	misc.h \
	name.cpp \
	name.h \
	names.h \
	net_channel.cpp \
	net_channel_level.cpp \
	net_channel_player.cpp \
	net_channel_thinker.cpp \
	net_connection.cpp \
	net_dgrm.cpp \
	net_loc.h \
	net_loop.cpp \
	net_main.cpp \
	net_null.cpp \
	network.h \
	p_acs.cpp \
	p_acs.h \
	p_clip.cpp \
	p_clip.h \
	p_entity.cpp \
	p_entity.h \
	p_gameinfo.h \
	p_levelinfo.cpp \
	p_levelinfo.h \
	p_playerreplicationinfo.cpp \
	p_playerreplicationinfo.h \
	p_polyobj.cpp \
	p_setup.cpp \
	p_thinker.cpp \
	p_thinker.h \
	p_trace.cpp \
	p_worldinfo.cpp \
	p_worldinfo.h \
	player.h \
	pr_cmds.cpp \
	pr_exec.cpp \
	progdefs.h \
	progs.h \
	protocol.h \
	r_bsp.cpp \
	r_light.cpp \
	r_local.h \
	r_main.cpp \
	r_model.cpp \
	r_public.h \
	r_shared.h \
	r_sky.cpp \
	r_surf.cpp \
	r_tex.cpp \
	r_tex.h \
	r_tex_automap.cpp \
	r_tex_base.cpp \
	r_tex_flat.cpp \
	r_tex_imgz.cpp \
	r_tex_jpeg.cpp \
	r_tex_multipatch.cpp \
	r_tex_patch.cpp \
	r_tex_pcx.cpp \
	r_tex_png.cpp \
	r_tex_raw.cpp \
	r_tex_tga.cpp \
	r_tex_warp.cpp \
	r_things.cpp \
	s_data.cpp \
	s_local.h \
	s_qmus2mid.cpp \
	s_sound.cpp \
	s_streamplayer.cpp \
	s_tmidty.cpp \
	s_wav.cpp \
	save.h \
	sbar.cpp \
	sbar.h \
	sc_man.cpp \
	screen.cpp \
	screen.h \
	scripts.h \
	server.h \
	sound.h \
	str.cpp \
	str.h \
	stream.cpp \
	stream.h \
	sv_ent.cpp \
	sv_local.h \
	sv_main.cpp \
	sv_save.cpp \
	sv_swtch.cpp \
	sv_tick.cpp \
	sv_user.cpp \
	sv_world.cpp \
	system.h \
	text.cpp \
	text.h \
	ui.h \
	ui_gc.cpp \
	ui_gc.h \
	ui_modal.cpp \
	ui_modal.h \
	ui_root.cpp \
	ui_root.h \
	ui_win.cpp \
	ui_win.h \
	vclass.cpp \
	vclass.h \
	vector.h \
	video.h \
	vobject.cpp \
	vobject.h \
	wad.cpp \
	wad.h \
	waddefs.h \
	xml.cpp \
	xml.h \
	zone.cpp \
	zone.h

#-- i386 assembler files --
if MAIN_ASM_I386
_____MAIN_EXE__SOURCES += \
	d_aclip_i386.s \
	d_alias_i386.s \
	d_edge_i386.s \
	d_polyset_i386.s \
	d_vars_i386.s \
	d_zspan_i386.s \
	d_part_8_i386.s \
	d_polyset_8_i386.s \
	d_span16_8_i386.s \
	d_span_8_i386.s \
	d_sprite_8_i386.s \
	d_surf_8_i386.s \
	d_part_16_i386.s \
	d_polyset_16_i386.s \
	d_span16_16_i386.s \
	d_span_16_i386.s \
	d_sprite_16_i386.s \
	d_surf_16_i386.s \
	d_part_32_i386.s \
	d_polyset_32_i386.s \
	d_span16_32_i386.s \
	d_span_32_i386.s \
	d_sprite_32_i386.s \
	d_surf_32_i386.s \
	sys_i386.s
endif

#-- Common OpenGL files --
if MAIN_OPENGL
_____MAIN_EXE__SOURCES += \
	gl_draw.cpp \
	gl_local.h \
	gl_main.cpp \
	gl_poly.cpp \
	gl_tex.cpp
endif

#-- OpenAL files --
if MAIN_OPENAL
_____MAIN_EXE__SOURCES += \
	s_al.cpp
endif

#-- Vorbis files --
if MAIN_VORBIS
_____MAIN_EXE__SOURCES += \
	s_vorbis.cpp
endif

#-- MP3 files --
if MAIN_MP3
_____MAIN_EXE__SOURCES += \
	s_mp3.cpp
endif

#-- MikMod files --
if MAIN_MIKMOD
_____MAIN_EXE__SOURCES += \
	s_mikmod.cpp
endif

#-- FLAC files --
if MAIN_FLAC
_____MAIN_EXE__SOURCES += \
	s_flac.cpp
endif

#-- DOS --
if MAIN_PLATFORM_DJGPP
_____MAIN_EXE__SOURCES += \
	cd_dos.cpp \
	d_alleg.cpp \
	in_dos.cpp \
	mpdosock.h \
	mplib.c \
	mplpc.c \
	net_bw.cpp \
	net_ipx.cpp \
	net_mp.cpp \
	npxsetup.c \
	s_alleg.cpp \
	s_allegm.cpp \
	sys_dos.cpp
endif
if MAIN_OPENGL_DJGPP
_____MAIN_EXE__SOURCES += \
	gl_alleg.cpp
endif

#-- Linux with Allegro --
if MAIN_PLATFORM_UNIX_ALLEGRO
_____MAIN_EXE__SOURCES += \
	d_alleg.cpp \
	in_alleg.cpp \
	net_udp.cpp \
	s_alleg.cpp \
	s_allegm.cpp \
	sys_lin.cpp
endif
if MAIN_OPENGL_UNIX_ALLEGRO
_____MAIN_EXE__SOURCES += \
	gl_agl.cpp
endif

#-- Linux with SDL --
if MAIN_PLATFORM_UNIX_SDL
_____MAIN_EXE__SOURCES += \
	d_sdl.cpp \
	in_sdl.cpp \
	net_udp.cpp \
	s_sdl.cpp \
	s_sdlm.cpp \
	sys_sdl.cpp
endif
if MAIN_OPENGL_UNIX_SDL
_____MAIN_EXE__SOURCES += \
	gl_sdl.cpp
endif

#-- CD audio drivers for various UNIX platforms --
if MAIN_CDAUDIO_LINUX
_____MAIN_EXE__SOURCES += \
	cd_linux.cpp
endif
if MAIN_CDAUDIO_BSD
_____MAIN_EXE__SOURCES += \
	cd_bsd.cpp
endif

#-- Windows --
if MAIN_PLATFORM_WIN32
_____MAIN_EXE__SOURCES += \
	cd_win32.cpp \
	d_win32.cpp \
	d3d_draw.cpp \
	d3d_info.cpp \
	d3d_local.h \
	d3d_main.cpp \
	d3d_poly.cpp \
	d3d_tex.cpp \
	eax.h \
	in_win32.cpp \
	net_wins.cpp \
	net_wipx.cpp \
	s_win32.cpp \
	s_win32m.cpp \
	sys_win.cpp \
	vavoom.ico \
	vavoom_2.ico \
	vavoom_3.ico \
	vavoom.rc \
	winlocal.h
endif
if MAIN_OPENGL_WIN32
_____MAIN_EXE__SOURCES += \
	gl_win32.cpp
endif

vavoom: Makefile
	echo "#!/bin/sh" > $@
	echo "# Needed to make symlinks/shortcuts work." >> $@
	echo "# the binaries must run with correct working directory" >> $@
	echo "cd \"$(datadir)/vavoom\"" >> $@
	echo "\"$(bindir)/$(MAIN_EXE)\" \$$* $(IWADDIR_PARM)" >> $@
	echo "exit \$$?" >> $@

#---------------------------------------
#
#	Dedicated server
#
#---------------------------------------

if ENABLE_SERVER
bin_PROGRAMS += ../$(SERVER_EXE)
if SERVER_PLATFORM_UNIX
bin_SCRIPTS += vavoom-dedicated
endif
endif

_____SERVER_EXE__CPPFLAGS = @SV_CPPFLAGS@ -DSERVER
_____SERVER_EXE__CXXFLAGS = @SV_CXXFLAGS@
_____SERVER_EXE__DEFS =
_____SERVER_EXE__LDFLAGS = @SV_LDFLAGS@
_____SERVER_EXE__LDADD = @SV_LIBS@

_____SERVER_EXE__SOURCES = \
	args.cpp \
	bitstream.cpp \
	cmd.cpp \
	crc.cpp \
	cvar.cpp \
	debug.cpp \
	dehacked.cpp \
	files.cpp \
	fs_dir.cpp \
	fs_wad.cpp \
	fs_zip.cpp \
	host.cpp \
	infostr.cpp \
	language.cpp \
	level.cpp \
	mapinfo.cpp \
	maths.cpp \
	message.cpp \
	misc.cpp \
	name.cpp \
	net_channel.cpp \
	net_channel_level.cpp \
	net_channel_player.cpp \
	net_channel_thinker.cpp \
	net_connection.cpp \
	net_dgrm.cpp \
	net_loop.cpp \
	net_main.cpp \
	net_null.cpp \
	p_acs.cpp \
	p_clip.cpp \
	p_entity.cpp \
	p_levelinfo.cpp \
	p_playerreplicationinfo.cpp \
	p_polyobj.cpp \
	p_setup.cpp \
	p_thinker.cpp \
	p_trace.cpp \
	p_worldinfo.cpp \
	pr_cmds.cpp \
	pr_exec.cpp \
	r_tex.cpp \
	r_tex_automap.cpp \
	r_tex_base.cpp \
	r_tex_flat.cpp \
	r_tex_imgz.cpp \
	r_tex_jpeg.cpp \
	r_tex_multipatch.cpp \
	r_tex_patch.cpp \
	r_tex_pcx.cpp \
	r_tex_png.cpp \
	r_tex_raw.cpp \
	r_tex_tga.cpp \
	r_tex_warp.cpp \
	s_data.cpp \
	sc_man.cpp \
	str.cpp \
	stream.cpp \
	sv_ent.cpp \
	sv_main.cpp \
	sv_save.cpp \
	sv_swtch.cpp \
	sv_tick.cpp \
	sv_user.cpp \
	sv_world.cpp \
	vclass.cpp \
	vobject.cpp \
	wad.cpp \
	xml.cpp \
	zone.cpp

if SERVER_PLATFORM_DJGPP
_____SERVER_EXE__SOURCES += \
	mplib.c \
	mplpc.c \
	net_bw.cpp \
	net_ipx.cpp \
	net_mp.cpp \
	npxsetup.c \
	sys_bsd.cpp
endif

if SERVER_PLATFORM_UNIX
_____SERVER_EXE__SOURCES += \
	net_udp.cpp \
	sys_bsd.cpp
endif

if SERVER_PLATFORM_WIN32
_____SERVER_EXE__SOURCES += \
	net_wins.cpp \
	net_wipx.cpp \
	sys_wind.cpp \
	vavoom.rc
endif

vavoom-dedicated: Makefile
	echo "#!/bin/sh" > $@
	echo "# Needed to make symlinks/shortcuts work." >> $@
	echo "# the binaries must run with correct working directory" >> $@
	echo "cd \"$(datadir)/vavoom\"" >> $@
	echo "\"$(bindir)/$(SERVER_EXE)\" \$$* $(IWADDIR_PARM)" >> $@
	echo "exit \$$?" >> $@

# ---------------------------------------
#
#	Compilation of Windows resources
#
# ---------------------------------------

.rc.o:
	windres --include-dir=$(srcdir) -o $@ -i $<

# ---------------------------------------
#
#	Icon
#
# ---------------------------------------

basedir = $(datadir)/vavoom
dist_base_DATA = vavoom.png

# ---------------------------------------

EXTRA_DIST = \
	gas2tasm.c \
	gl_x.cpp \
	template.cpp \
	template.h
