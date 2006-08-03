SUBDIRS = timidity

INSTALL_DIR = $(DESTDIR)$(datadir)/vavoom

#---------------------------------------
#
#	Main executable objects
#
#---------------------------------------

OBJDIR=../obj

#-- Common files --
COMMON_OBJS = \
	$(OBJDIR)/am_map.$(OBJEXT) \
	$(OBJDIR)/args.$(OBJEXT) \
	$(OBJDIR)/chat.$(OBJEXT) \
	$(OBJDIR)/cheats.$(OBJEXT) \
	$(OBJDIR)/cl_demo.$(OBJEXT) \
	$(OBJDIR)/cl_input.$(OBJEXT) \
	$(OBJDIR)/cl_main.$(OBJEXT) \
	$(OBJDIR)/cl_parse.$(OBJEXT) \
	$(OBJDIR)/cl_poly.$(OBJEXT) \
	$(OBJDIR)/cl_trace.$(OBJEXT) \
	$(OBJDIR)/cmd.$(OBJEXT) \
	$(OBJDIR)/console.$(OBJEXT) \
	$(OBJDIR)/crc.$(OBJEXT) \
	$(OBJDIR)/cvar.$(OBJEXT) \
	$(OBJDIR)/d_aclip.$(OBJEXT) \
	$(OBJDIR)/d_alias.$(OBJEXT) \
	$(OBJDIR)/d_data.$(OBJEXT) \
	$(OBJDIR)/d_draw.$(OBJEXT) \
	$(OBJDIR)/d_edge.$(OBJEXT) \
	$(OBJDIR)/d_main.$(OBJEXT) \
	$(OBJDIR)/d_part.$(OBJEXT) \
	$(OBJDIR)/d_polyse.$(OBJEXT) \
	$(OBJDIR)/d_scache.$(OBJEXT) \
	$(OBJDIR)/d_span.$(OBJEXT) \
	$(OBJDIR)/d_sprite.$(OBJEXT) \
	$(OBJDIR)/d_surf.$(OBJEXT) \
	$(OBJDIR)/d_tex.$(OBJEXT) \
	$(OBJDIR)/d_aclipa.$(OBJEXT) \
	$(OBJDIR)/d_aliasa.$(OBJEXT) \
	$(OBJDIR)/d_edgea.$(OBJEXT) \
	$(OBJDIR)/d_polysa.$(OBJEXT) \
	$(OBJDIR)/d_varsa.$(OBJEXT) \
	$(OBJDIR)/d_zspan.$(OBJEXT) \
	$(OBJDIR)/d8_part.$(OBJEXT) \
	$(OBJDIR)/d8_poly.$(OBJEXT) \
	$(OBJDIR)/d8_s16.$(OBJEXT) \
	$(OBJDIR)/d8_span.$(OBJEXT) \
	$(OBJDIR)/d8_spr.$(OBJEXT) \
	$(OBJDIR)/d8_surf.$(OBJEXT) \
	$(OBJDIR)/d16_part.$(OBJEXT) \
	$(OBJDIR)/d16_poly.$(OBJEXT) \
	$(OBJDIR)/d16_s16.$(OBJEXT) \
	$(OBJDIR)/d16_span.$(OBJEXT) \
	$(OBJDIR)/d16_spr.$(OBJEXT) \
	$(OBJDIR)/d16_surf.$(OBJEXT) \
	$(OBJDIR)/d32_part.$(OBJEXT) \
	$(OBJDIR)/d32_poly.$(OBJEXT) \
	$(OBJDIR)/d32_s16.$(OBJEXT) \
	$(OBJDIR)/d32_span.$(OBJEXT) \
	$(OBJDIR)/d32_spr.$(OBJEXT) \
	$(OBJDIR)/d32_surf.$(OBJEXT) \
	$(OBJDIR)/debug.$(OBJEXT) \
	$(OBJDIR)/finale.$(OBJEXT) \
	$(OBJDIR)/files.$(OBJEXT) \
	$(OBJDIR)/host.$(OBJEXT) \
	$(OBJDIR)/iline.$(OBJEXT) \
	$(OBJDIR)/imission.$(OBJEXT) \
	$(OBJDIR)/in_input.$(OBJEXT) \
	$(OBJDIR)/infostr.$(OBJEXT) \
	$(OBJDIR)/l_glbsp.$(OBJEXT) \
	$(OBJDIR)/l_glvis.$(OBJEXT) \
	$(OBJDIR)/level.$(OBJEXT) \
	$(OBJDIR)/mapinfo.$(OBJEXT) \
	$(OBJDIR)/maths.$(OBJEXT) \
	$(OBJDIR)/menu.$(OBJEXT) \
	$(OBJDIR)/message.$(OBJEXT) \
	$(OBJDIR)/misc.$(OBJEXT) \
	$(OBJDIR)/name.$(OBJEXT) \
	$(OBJDIR)/net_dgrm.$(OBJEXT) \
	$(OBJDIR)/net_loop.$(OBJEXT) \
	$(OBJDIR)/net_main.$(OBJEXT) \
	$(OBJDIR)/net_null.$(OBJEXT) \
	$(OBJDIR)/p_setup.$(OBJEXT) \
	$(OBJDIR)/p_thinker.$(OBJEXT) \
	$(OBJDIR)/pr_cmds.$(OBJEXT) \
	$(OBJDIR)/pr_exec.$(OBJEXT) \
	$(OBJDIR)/r_bsp.$(OBJEXT) \
	$(OBJDIR)/r_light.$(OBJEXT) \
	$(OBJDIR)/r_main.$(OBJEXT) \
	$(OBJDIR)/r_model.$(OBJEXT) \
	$(OBJDIR)/r_sky.$(OBJEXT) \
	$(OBJDIR)/r_surf.$(OBJEXT) \
	$(OBJDIR)/r_tex.$(OBJEXT) \
	$(OBJDIR)/r_things.$(OBJEXT) \
	$(OBJDIR)/s_data.$(OBJEXT) \
	$(OBJDIR)/s_eaxutl.$(OBJEXT) \
	$(OBJDIR)/s_qmus2mid.$(OBJEXT) \
	$(OBJDIR)/s_sound.$(OBJEXT) \
	$(OBJDIR)/s_streamplayer.$(OBJEXT) \
	$(OBJDIR)/s_tmidty.$(OBJEXT) \
	$(OBJDIR)/s_wav.$(OBJEXT) \
	$(OBJDIR)/sbar.$(OBJEXT) \
	$(OBJDIR)/sc_man.$(OBJEXT) \
	$(OBJDIR)/screen.$(OBJEXT) \
	$(OBJDIR)/str.$(OBJEXT) \
	$(OBJDIR)/stream.$(OBJEXT) \
	$(OBJDIR)/sv_acs.$(OBJEXT) \
	$(OBJDIR)/sv_ent.$(OBJEXT) \
	$(OBJDIR)/sv_main.$(OBJEXT) \
	$(OBJDIR)/sv_poly.$(OBJEXT) \
	$(OBJDIR)/sv_save.$(OBJEXT) \
	$(OBJDIR)/sv_sight.$(OBJEXT) \
	$(OBJDIR)/sv_swtch.$(OBJEXT) \
	$(OBJDIR)/sv_tick.$(OBJEXT) \
	$(OBJDIR)/sv_user.$(OBJEXT) \
	$(OBJDIR)/sv_world.$(OBJEXT) \
	$(OBJDIR)/text.$(OBJEXT) \
	$(OBJDIR)/ui_gc.$(OBJEXT) \
	$(OBJDIR)/ui_modal.$(OBJEXT) \
	$(OBJDIR)/ui_root.$(OBJEXT) \
	$(OBJDIR)/ui_win.$(OBJEXT) \
	$(OBJDIR)/vclass.$(OBJEXT) \
	$(OBJDIR)/vobject.$(OBJEXT) \
	$(OBJDIR)/wad.$(OBJEXT) \
	$(OBJDIR)/zone.$(OBJEXT)

#-- Common OpenGL files --
GL_OBJS = \
	$(OBJDIR)/gl_draw.$(OBJEXT) \
	$(OBJDIR)/gl_main.$(OBJEXT) \
	$(OBJDIR)/gl_poly.$(OBJEXT) \
	$(OBJDIR)/gl_tex.$(OBJEXT)

#-- OpenAL files --
AL_OBJS = \
	$(OBJDIR)/s_al.$(OBJEXT)

#-- Vorbis files --
VORBIS_OBJS = \
	$(OBJDIR)/s_vorbis.$(OBJEXT)

#-- MP3 files --
MP3_OBJS = \
	$(OBJDIR)/s_mp3.$(OBJEXT)

#-- MikMod files --
MIKMOD_OBJS = \
	$(OBJDIR)/s_mikmod.$(OBJEXT)

#-- FLAC files --
FLAC_OBJS = \
	$(OBJDIR)/s_flac.$(OBJEXT)

#-- DOS --
SYS_OBJS_DJGPP = \
	$(OBJDIR)/cd_dos.$(OBJEXT) \
	$(OBJDIR)/d_alleg.$(OBJEXT) \
	$(OBJDIR)/in_dos.$(OBJEXT) \
	$(OBJDIR)/mplib.$(OBJEXT) \
	$(OBJDIR)/mplpc.$(OBJEXT) \
	$(OBJDIR)/net_bw.$(OBJEXT) \
	$(OBJDIR)/net_ipx.$(OBJEXT) \
	$(OBJDIR)/net_mp.$(OBJEXT) \
	$(OBJDIR)/npxsetup.$(OBJEXT) \
	$(OBJDIR)/s_alleg.$(OBJEXT) \
	$(OBJDIR)/s_allegm.$(OBJEXT) \
	$(OBJDIR)/sys_i386.$(OBJEXT) \
	$(OBJDIR)/sys_dos.$(OBJEXT)
GL_SYS_OBJ_DJGPP = $(OBJDIR)/gl_alleg.$(OBJEXT)

#-- Linux with Allegro --
SYS_OBJS_UNIX_ALLEGRO = \
	$(OBJDIR)/d_alleg.$(OBJEXT) \
	$(OBJDIR)/in_alleg.$(OBJEXT) \
	$(OBJDIR)/net_udp.$(OBJEXT) \
	$(OBJDIR)/s_alleg.$(OBJEXT) \
	$(OBJDIR)/s_allegm.$(OBJEXT) \
	$(OBJDIR)/sys_i386.$(OBJEXT) \
	$(OBJDIR)/sys_lin.$(OBJEXT)
GL_SYS_OBJ_UNIX_ALLEGRO = $(OBJDIR)/gl_agl.$(OBJEXT)

#-- Linux with SDL --
SYS_OBJS_UNIX_SDL = \
	$(OBJDIR)/d_sdl.$(OBJEXT) \
	$(OBJDIR)/in_sdl.$(OBJEXT) \
	$(OBJDIR)/net_udp.$(OBJEXT) \
	$(OBJDIR)/s_sdl.$(OBJEXT) \
	$(OBJDIR)/s_sdlm.$(OBJEXT) \
	$(OBJDIR)/sys_i386.$(OBJEXT) \
	$(OBJDIR)/sys_sdl.$(OBJEXT)
GL_SYS_OBJ_UNIX_SDL = $(OBJDIR)/gl_sdl.$(OBJEXT)

#-- CD audio drivers for various UNIX platforms --
CDAUDIO_OBJ_LINUX = $(OBJDIR)/cd_linux.$(OBJEXT)
CDAUDIO_OBJ_BSD = $(OBJDIR)/cd_bsd.$(OBJEXT)

#-- Windows --
SYS_OBJS_WIN32 = \
	$(OBJDIR)/cd_win32.$(OBJEXT) \
	$(OBJDIR)/d_win32.$(OBJEXT) \
	$(OBJDIR)/d3d_draw.$(OBJEXT) \
	$(OBJDIR)/d3d_info.$(OBJEXT) \
	$(OBJDIR)/d3d_main.$(OBJEXT) \
	$(OBJDIR)/d3d_poly.$(OBJEXT) \
	$(OBJDIR)/d3d_tex.$(OBJEXT) \
	$(OBJDIR)/in_win32.$(OBJEXT) \
	$(OBJDIR)/net_wins.$(OBJEXT) \
	$(OBJDIR)/net_wipx.$(OBJEXT) \
	$(OBJDIR)/s_win32.$(OBJEXT) \
	$(OBJDIR)/s_win32m.$(OBJEXT) \
	$(OBJDIR)/sys_i386.$(OBJEXT) \
	$(OBJDIR)/sys_win.$(OBJEXT)
GL_SYS_OBJ_WIN32 = $(OBJDIR)/gl_win32.$(OBJEXT)

#---------------------------------------
#
#	Dedicated server objcts
#
#---------------------------------------

SV_OBJDIR=../obj/sv

SV_COMMON_OBJ_FILES = \
	$(SV_OBJDIR)/args.$(OBJEXT) \
	$(SV_OBJDIR)/cmd.$(OBJEXT) \
	$(SV_OBJDIR)/crc.$(OBJEXT) \
	$(SV_OBJDIR)/cvar.$(OBJEXT) \
	$(SV_OBJDIR)/debug.$(OBJEXT) \
	$(SV_OBJDIR)/files.$(OBJEXT) \
	$(SV_OBJDIR)/host.$(OBJEXT) \
	$(SV_OBJDIR)/infostr.$(OBJEXT) \
	$(SV_OBJDIR)/level.$(OBJEXT) \
	$(SV_OBJDIR)/mapinfo.$(OBJEXT) \
	$(SV_OBJDIR)/maths.$(OBJEXT) \
	$(SV_OBJDIR)/message.$(OBJEXT) \
	$(SV_OBJDIR)/misc.$(OBJEXT) \
	$(SV_OBJDIR)/name.$(OBJEXT) \
	$(SV_OBJDIR)/net_dgrm.$(OBJEXT) \
	$(SV_OBJDIR)/net_loop.$(OBJEXT) \
	$(SV_OBJDIR)/net_main.$(OBJEXT) \
	$(SV_OBJDIR)/net_null.$(OBJEXT) \
	$(SV_OBJDIR)/p_setup.$(OBJEXT) \
	$(SV_OBJDIR)/p_thinker.$(OBJEXT) \
	$(SV_OBJDIR)/pr_cmds.$(OBJEXT) \
	$(SV_OBJDIR)/pr_exec.$(OBJEXT) \
	$(SV_OBJDIR)/r_tex.$(OBJEXT) \
	$(SV_OBJDIR)/s_data.$(OBJEXT) \
	$(SV_OBJDIR)/sc_man.$(OBJEXT) \
	$(SV_OBJDIR)/str.$(OBJEXT) \
	$(SV_OBJDIR)/stream.$(OBJEXT) \
	$(SV_OBJDIR)/sv_acs.$(OBJEXT) \
	$(SV_OBJDIR)/sv_ent.$(OBJEXT) \
	$(SV_OBJDIR)/sv_main.$(OBJEXT) \
	$(SV_OBJDIR)/sv_poly.$(OBJEXT) \
	$(SV_OBJDIR)/sv_save.$(OBJEXT) \
	$(SV_OBJDIR)/sv_sight.$(OBJEXT) \
	$(SV_OBJDIR)/sv_swtch.$(OBJEXT) \
	$(SV_OBJDIR)/sv_tick.$(OBJEXT) \
	$(SV_OBJDIR)/sv_user.$(OBJEXT) \
	$(SV_OBJDIR)/sv_world.$(OBJEXT) \
	$(SV_OBJDIR)/vclass.$(OBJEXT) \
	$(SV_OBJDIR)/vobject.$(OBJEXT) \
	$(SV_OBJDIR)/wad.$(OBJEXT) \
	$(SV_OBJDIR)/zone.$(OBJEXT)

SV_SYS_OBJS_DJGPP = \
	$(SV_OBJDIR)/mplib.$(OBJEXT) \
	$(SV_OBJDIR)/mplpc.$(OBJEXT) \
	$(SV_OBJDIR)/net_bw.$(OBJEXT) \
	$(SV_OBJDIR)/net_ipx.$(OBJEXT) \
	$(SV_OBJDIR)/net_mp.$(OBJEXT) \
	$(SV_OBJDIR)/npxsetup.$(OBJEXT) \
	$(SV_OBJDIR)/sys_bsd.$(OBJEXT)

SV_SYS_OBJS_UNIX = \
	$(SV_OBJDIR)/net_udp.$(OBJEXT) \
	$(SV_OBJDIR)/sys_bsd.$(OBJEXT)

SV_SYS_OBJS_WIN32 = \
	$(SV_OBJDIR)/net_wins.$(OBJEXT) \
	$(SV_OBJDIR)/net_wipx.$(OBJEXT) \
	$(SV_OBJDIR)/sys_wind.$(OBJEXT)

# ---------------------------------------

.PHONY: all exe svexe clean install

all: $(TARGETS)

# ---------------------------------------

exe: ../$(MAIN_EXE) vavoom

../$(MAIN_EXE): $(OBJ_FILES) $(LIB_FILES)
	$(CXX) $(MAIN_LDFLAGS) -o $@ $^ $(MAIN_LIBS)

$(OBJDIR)/%.$(OBJEXT) : $(srcdir)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJDIR)/%.$(OBJEXT) : $(srcdir)/%.cpp $(srcdir)/*.h
	@mkdir -p $(OBJDIR)
	$(CXX) -c $(MAIN_CPPFLAGS) $(MAIN_CXXFLAGS) -o $@ $<

$(OBJDIR)/%.$(OBJEXT) : $(srcdir)/%.s $(srcdir)/asm_i386.h
	@mkdir -p $(OBJDIR)
	$(CC) -c -x assembler-with-cpp $(ASMFLAGS) -o $@ $<

vavoom: Makefile
	echo "#!/bin/sh" > $@
	echo "# Needed to make symlinks/shortcuts work." >> $@
	echo "# the binaries must run with correct working directory" >> $@
	echo "cd \"$(datadir)/vavoom\"" >> $@
	echo "\"$(bindir)/$(MAIN_EXE)\" \$$* $(IWADDIR_PARM)" >> $@
	echo "exit \$$?" >> $@

# ---------------------------------------

timidity/libtimidity.a: $(srcdir)/timidity/*.cpp $(srcdir)/timidity/*.h
	$(MAKE) -C timidity

../utils/glbsp/libglbsp.a:
	$(MAKE) -C ../utils/glbsp

../utils/glvis/libglvis.a:
	$(MAKE) -C ../utils/glvis libglvis.a

# ---------------------------------------

svexe: ../$(SERVER_EXE) vavoom-dedicated

../$(SERVER_EXE): $(SV_OBJ_FILES)
	$(CXX) $(SV_LDFLAGS) -o $@ $(SV_OBJ_FILES) $(SV_LIBS)

$(SV_OBJDIR)/%.$(OBJEXT) : $(srcdir)/%.c
	@mkdir -p $(SV_OBJDIR)
	$(CC) $(SV_CPPFLAGS) -c $(CFLAGS) -DSERVER -o $@ $<

$(SV_OBJDIR)/%.$(OBJEXT) : $(srcdir)/%.cpp $(srcdir)/*.h
	@mkdir -p $(SV_OBJDIR)
	$(CXX) $(SV_CPPFLAGS) $(SV_CXXFLAGS) -DSERVER -c -o $@ $<

$(SV_OBJDIR)/%.$(OBJEXT) : $(srcdir)/%.s
	@mkdir -p $(SV_OBJDIR)
	$(CC) -c -x assembler-with-cpp $(ASMFLAGS) -DSERVER -o $@ $<

vavoom-dedicated: Makefile
	echo "#!/bin/sh" > $@
	echo "# Needed to make symlinks/shortcuts work." >> $@
	echo "# the binaries must run with correct working directory" >> $@
	echo "cd \"$(datadir)/vavoom\"" >> $@
	echo "\"$(bindir)/$(SERVER_EXE)\" \$$* $(IWADDIR_PARM)" >> $@
	echo "exit \$$?" >> $@

# ---------------------------------------

install: $(INSTALL_TARGETS)
	$(INSTALL) -d $(INSTALL_DIR)
	$(INSTALL_DATA) $(srcdir)/vavoom.png $(INSTALL_DIR)

installexe:
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) ../$(MAIN_EXE) $(DESTDIR)$(bindir)
	$(INSTALL_SCRIPT) vavoom $(DESTDIR)$(bindir)

installsvexe:
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) ../$(SERVER_EXE) $(DESTDIR)$(bindir)
	$(INSTALL_SCRIPT) vavoom-dedicated $(DESTDIR)$(bindir)

# ---------------------------------------

clean:
	-rm $(OBJDIR)/*.o
	-rm $(OBJDIR)/cl/*.o
	-rm $(OBJDIR)/sv/*.o

# ---------------------------------------
#
#	Generation of asm files for Windows
#
# ---------------------------------------

ASM_FILES = \
	d_aclipa.asm \
	d_aliasa.asm \
	d_edgea.asm \
	d_polysa.asm \
	d_zspan.asm \
	d_varsa.asm \
	d8_part.asm \
	d8_poly.asm \
	d8_s16.asm \
	d8_span.asm \
	d8_spr.asm \
	d8_surf.asm \
	d16_part.asm \
	d16_poly.asm \
	d16_s16.asm \
	d16_span.asm \
	d16_spr.asm \
	d16_surf.asm \
	d32_part.asm \
	d32_poly.asm \
	d32_s16.asm \
	d32_span.asm \
	d32_spr.asm \
	d32_surf.asm \
	pr_execa.asm \
	sys_i386.asm

asm: $(ASM_FILES)

%.asm : %.s asm_i386.h gas2tasm$(EXEEXT)
	$(CC) -x assembler-with-cpp -E -P -DGAS2TASM $< -o - | gas2tasm$(EXEEXT) > $@

gas2tasm$(EXEEXT) : gas2tasm.c
	$(CC) -O3 -ffast-math -fomit-frame-pointer -s -o $@ $<
