# ---------------------------------------------------------------------------
!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif

# ---------------------------------------------------------------------------
CFLAGS = -O2 -X- -a4 -4 -b -k- -vi -q -c -w -Ic:\mssdk\include -D_NO_VCL
RFLAGS = /l 0x426
AFLAGS = /ml /zn
LFLAGS = -Iobj -aa -Tpe -x -Gn -w -q

PROJECT = Vavoom95.exe
OBJFILES = obj\chat.obj obj\cheats.obj \
	obj\cmd.obj obj\crc.obj obj\cvar.obj obj\debug.obj \
	obj\host.obj obj\sys_win.obj \
	obj\misc.obj obj\mapinfo.obj obj\p_setup.obj obj\level.obj \
	obj\pr_cmds.obj obj\pr_exec.obj obj\str.obj obj\stream.obj \
	obj\sc_man.obj obj\maths.obj obj\sizebuf.obj obj\name.obj \
	obj\wad.obj obj\zone.obj obj\message.obj obj\infostr.obj obj\files.obj \
	\
	obj\vobject.obj obj\vclass.obj \
	\
	obj\net_dgrm.obj obj\net_loop.obj obj\net_main.obj obj\net_null.obj \
	obj\net_win.obj obj\net_wins.obj obj\net_wipx.obj \
	\
	obj\p_thinker.obj \
	\
	obj\sv_acs.obj obj\sv_ent.obj \
	obj\sv_main.obj obj\sv_poly.obj obj\sv_save.obj obj\sv_sight.obj \
	obj\sv_swtch.obj obj\sv_user.obj obj\sv_tick.obj obj\sv_world.obj \
	\
	obj\cl_demo.obj obj\cl_input.obj obj\cl_main.obj obj\cl_parse.obj \
	obj\cl_poly.obj obj\cl_trace.obj \
	\
	obj\in_input.obj obj\in_win32.obj \
	\
	obj\cd_win32.obj obj\s_al.obj obj\s_data.obj obj\s_eaxutl.obj \
	obj\s_mp3.obj obj\s_sound.obj obj\s_tmidty.obj obj\s_vorbis.obj \
	obj\s_wav.obj obj\s_win32.obj obj\s_win32m.obj obj\sn_sonix.obj \
	\
	obj\am_map.obj obj\menu.obj obj\sbar.obj obj\screen.obj obj\text.obj \
	obj\finale.obj obj\iline.obj obj\imission.obj obj\console.obj \
	obj\ui_gc.obj obj\ui_modal.obj obj\ui_root.obj obj\ui_win.obj \
	\
	obj\r_bsp.obj obj\r_light.obj obj\r_main.obj obj\r_model.obj \
	obj\r_sky.obj obj\r_surf.obj obj\r_tex.obj obj\r_things.obj \
	\
	obj\d_aclip.obj obj\d_alias.obj obj\d_data.obj obj\d_draw.obj \
	obj\d_edge.obj obj\d_main.obj obj\d_part.obj obj\d_polyse.obj \
	obj\d_scache.obj obj\d_span.obj obj\d_sprite.obj obj\d_surf.obj \
	obj\d_tex.obj obj\d_win32.obj \
	\
	obj\d3d_draw.obj obj\d3d_info.obj obj\d3d_main.obj obj\d3d_poly.obj \
	obj\d3d_tex.obj \
	\
	obj\gl_draw.obj obj\gl_main.obj obj\gl_poly.obj obj\gl_tex.obj \
	obj\gl_win32.obj \
	\
	obj\l_glbsp.obj obj\l_glvis.obj

!if !$d(NOASM)
OBJFILES = $(OBJFILES) \
	obj\sys_i386.obj obj\pr_execa.obj \
	obj\d_aclipa.obj obj\d_aliasa.obj obj\d_edgea.obj obj\d_polysa.obj \
	obj\d_varsa.obj obj\d_zspan.obj \
	obj\d8_part.obj obj\d8_poly.obj obj\d8_s16.obj obj\d8_span.obj \
	obj\d8_spr.obj obj\d8_surf.obj \
	obj\d16_part.obj obj\d16_poly.obj obj\d16_s16.obj obj\d16_span.obj \
	obj\d16_spr.obj obj\d16_surf.obj \
	obj\d32_part.obj obj\d32_poly.obj obj\d32_s16.obj obj\d32_span.obj \
	obj\d32_spr.obj obj\d32_surf.obj
!else
CFLAGS = $(CFLAGS) -DNOASM
!endif

RESFILES = obj\vavoom.res
LIBRARIES = utils\glbsp\libglbsp.lib utils\glvis\glvis.lib \
	source\timidity\timidity.lib $(BCB)\lib\openal32.lib
# ---------------------------------------------------------------------------
.autodepend
# ---------------------------------------------------------------------------
.path.cpp = source
.path.asm = source
.path.rc  = source
.path.s   = source
.path.dll = .;$(PATH)
# ---------------------------------------------------------------------------
$(PROJECT): $(OBJFILES) $(RESFILES) $(LIBRARIES)
	ilink32 @&&!
	$(LFLAGS) -L$(BCB)\lib c0w32.obj $(OBJFILES), +
	$(PROJECT),,$(LIBRARIES) libpng.lib zlib.lib libmad.lib ogg_static.lib +
	vorbis_static.lib import32.lib cw32.lib,, +
	$(RESFILES)
!
# ---------------------------------------------------------------------------
.cpp.obj:
	bcc32 $(CFLAGS) -n$(@D) {$< }

.asm.obj:
	tasm32 $(AFLAGS) $<, $@

.rc.res:
	brcc32 $(RFLAGS) -fo$@ $<

.s.asm:
	gcc -x assembler-with-cpp -E -P -DGAS2TASM $< -o obj\temp.i
	source\gas2tasm.exe < obj\temp.i > $@
	del obj\temp.i > nul

.dll.lib:
	implib -a $@ $<
# ---------------------------------------------------------------------------
