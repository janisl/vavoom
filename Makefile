#---------------------------------------
#
#	Some configurations
#
#---------------------------------------

# Uncomment to compile a debug version
DEBUG = 1

# Uncomment to compile without OpenGL driver
#NOGL = 1

# Uncomment if for OpenGL you must link to libMesaGL.a and not to libGL.a
#MESAGL = 1

#---------------------------------------
#
#	Executable extension
#
#---------------------------------------

ifdef DJGPP
EXE = .exe
else
EXE =
endif

#---------------------------------------
#
#	Main executable objects and libs
#
#---------------------------------------

ifdef DJGPP
SYS_OBJS = \
	obj/cd_dos.o \
	obj/in_dos.o \
	obj/mplib.o \
	obj/mplpc.o \
	obj/net_bw.o \
	obj/net_dos.o \
	obj/net_ipx.o \
	obj/net_mp.o \
	obj/npxsetup.o \
	obj/sys_i386.o \
	obj/sys_dos.o
GL_SYS_OBJ = obj/gl_alleg.o
LIBS := -lalleg -lstdcxx
else
SYS_OBJS = \
	obj/cd_linux.o \
	obj/in_alleg.o \
	obj/net_bsd.o \
	obj/net_udp.o \
	obj/sys_i386.o \
	obj/sys_lin.o
GL_SYS_OBJ = obj/gl_x.o
LIBS := `allegro-config --libs` -lm -lstdc++
#LIBS := `allegro-config --static` -lm -lstdc++ -static
endif

OBJ_FILES = $(SYS_OBJS) \
	obj/am_map.o \
	obj/chat.o \
	obj/cheats.o \
	obj/cl_demo.o \
	obj/cl_input.o \
	obj/cl_main.o \
	obj/cl_parse.o \
	obj/cl_poly.o \
	obj/cl_trace.o \
	obj/cmd.o \
	obj/console.o \
	obj/crc.o \
	obj/cvar.o \
	obj/d_aclip.o \
	obj/d_alias.o \
	obj/d_alleg.o \
	obj/d_data.o \
	obj/d_draw.o \
	obj/d_edge.o \
	obj/d_main.o \
	obj/d_part.o \
	obj/d_polyse.o \
	obj/d_scache.o \
	obj/d_span.o \
	obj/d_sprite.o \
	obj/d_surf.o \
	obj/d_tex.o \
	obj/d_aclipa.o \
	obj/d_aliasa.o \
	obj/d_edgea.o \
	obj/d_polysa.o \
	obj/d_varsa.o \
	obj/d_zspan.o \
	obj/d8_part.o \
	obj/d8_poly.o \
	obj/d8_s16.o \
	obj/d8_span.o \
	obj/d8_spr.o \
	obj/d8_surf.o \
	obj/d16_part.o \
	obj/d16_poly.o \
	obj/d16_s16.o \
	obj/d16_span.o \
	obj/d16_spr.o \
	obj/d16_surf.o \
	obj/d32_part.o \
	obj/d32_poly.o \
	obj/d32_s16.o \
	obj/d32_span.o \
	obj/d32_spr.o \
	obj/d32_surf.o \
	obj/debug.o \
	obj/finale.o \
	obj/files.o \
	obj/host.o \
	obj/iline.o \
	obj/imission.o \
	obj/in_input.o \
	obj/infostr.o \
	obj/l_glbsp.o \
	obj/l_glvis.o \
	obj/mapinfo.o \
	obj/maths.o \
	obj/menu.o \
	obj/message.o \
	obj/misc.o \
	obj/net_dgrm.o \
	obj/net_loop.o \
	obj/net_main.o \
	obj/p_setup.o \
	obj/pr_cmds.o \
	obj/pr_exec.o \
	obj/pr_execa.o \
	obj/r_bsp.o \
	obj/r_light.o \
	obj/r_main.o \
	obj/r_model.o \
	obj/r_sky.o \
	obj/r_surf.o \
	obj/r_tex.o \
	obj/r_things.o \
	obj/s_alleg.o \
	obj/s_allegm.o \
	obj/s_data.o \
	obj/s_sound.o \
	obj/sbar.o \
	obj/sc_man.o \
	obj/screen.o \
	obj/sizebuf.o \
	obj/sn_sonix.o \
	obj/sv_acs.o \
	obj/sv_main.o \
	obj/sv_poly.o \
	obj/sv_save.o \
	obj/sv_sight.o \
	obj/sv_swtch.o \
	obj/sv_tick.o \
	obj/sv_user.o \
	obj/sv_world.o \
	obj/text.o \
	obj/wad.o \
	obj/zone.o

LIB_FILES = \
	utils/glbsp/plugin/libglbsp.a \
	utils/glvis/libglvis.a

ifndef NOGL
OBJ_FILES += \
	$(GL_SYS_OBJ) \
	obj/gl_draw.o \
	obj/gl_main.o \
	obj/gl_poly.o \
	obj/gl_tex.o
ifdef MESAGL
LIBS := -lMesaGL $(LIBS)
else
LIBS := -lGL $(LIBS)
endif
endif

#---------------------------------------
#
#	Dedicated server objcts and libs
#
#---------------------------------------

ifdef DJGPP
SV_SYS_OBJS = \
	obj/sv/mplib.o \
	obj/sv/mplpc.o \
	obj/sv/net_bw.o \
	obj/sv/net_dos.o \
	obj/sv/net_ipx.o \
	obj/sv/net_mp.o \
	obj/sv/npxsetup.o
SV_LIBS = -lstdcxx
else
SV_SYS_OBJS = \
	obj/sv/net_bsd.o \
	obj/sv/net_udp.o
SV_LIBS = -lm -lstdc++
endif

SV_OBJ_FILES = $(SV_SYS_OBJS) \
	obj/sv/cmd.o \
	obj/sv/crc.o \
	obj/sv/cvar.o \
	obj/sv/debug.o \
	obj/sv/files.o \
	obj/sv/host.o \
	obj/sv/infostr.o \
	obj/sv/maths.o \
	obj/sv/message.o \
	obj/sv/misc.o \
	obj/sv/net_dgrm.o \
	obj/sv/net_loop.o \
	obj/sv/net_main.o \
	obj/sv/p_info.o \
	obj/sv/p_setup.o \
	obj/sv/pr_cmds.o \
	obj/sv/pr_exec.o \
	obj/sv/pr_execa.o \
	obj/sv/r_tex.o \
	obj/sv/s_data.o \
	obj/sv/sc_man.o \
	obj/sv/sizebuf.o \
	obj/sv/sv_acs.o \
	obj/sv/sv_main.o \
	obj/sv/sv_poly.o \
	obj/sv/sv_save.o \
	obj/sv/sv_sight.o \
	obj/sv/sv_swtch.o \
	obj/sv/sv_tick.o \
	obj/sv/sv_user.o \
	obj/sv/sv_world.o \
	obj/sv/sys_bsd.o \
	obj/sv/wad.o \
	obj/sv/zone.o

#---------------------------------------
#
#	WAD files
#
#---------------------------------------

WAD_FILES = \
	basev/doom/wad0.wad \
	basev/doom1/wad0.wad \
	basev/doom2/wad0.wad \
	basev/tnt/wad0.wad \
	basev/plutonia/wad0.wad \
	basev/heretic/wad0.wad \
	basev/hexen/wad0.wad \
	basev/strife/wad0.wad

# ---------------------------------------

C_ARGS   = -c -W -Wall -mpentiumpro -ffast-math
CPP_ARGS = -c -W -Wall -mpentiumpro -ffast-math
ASM_ARGS = -c -W -Wall -x assembler-with-cpp
LINK_ARGS = -Wall

ifdef DEBUG

C_ARGS   += -O -g
CPP_ARGS += -O -g

else

C_ARGS   += -O3 -fomit-frame-pointer
CPP_ARGS += -O3 -fomit-frame-pointer
LINK_ARGS += -s

endif

# ---------------------------------------

.PHONY: all exe suid crash sv svexe utils progs data clean install

# ---------------------------------------

all: exe utils data

exe: Vavoom$(EXE)

Vavoom$(EXE): $(OBJ_FILES) $(LIB_FILES)
	gcc $(LINK_ARGS) -o $@ $^ $(LIBS)

suid:
	chown root.root Vavoom
	chmod 4755 Vavoom

crash:
	addr2line -e Vavoom -f < crash.txt >> basev/debug.txt

obj/%.o : source/%.c
	gcc $(C_ARGS) -o $@ $<

obj/%.o : source/%.cpp source/*.h
	gcc $(CPP_ARGS) -o $@ $<

obj/%.o : source/%.s source/asm_i386.h
	gcc $(ASM_ARGS) -o $@ $<

# ---------------------------------------

utils/glbsp/plugin/libglbsp.a:
	$(MAKE) -C utils/glbsp/plugin

utils/glvis/libglvis.a:
	$(MAKE) -C utils/glvis libglvis.a

# ---------------------------------------

sv: svexe data

svexe: VavoomSV$(EXE)

VavoomSV$(EXE): $(SV_OBJ_FILES)
	gcc $(LINK_ARGS) -o $@ $(SV_OBJ_FILES) $(SV_LIBS)

obj/sv/%.o : source/%.c
	gcc $(C_ARGS) -DSERVER -o $@ $<

obj/sv/%.o : source/%.cpp source/*.h
	gcc $(CPP_ARGS) -DSERVER -o $@ $<

obj/sv/%.o : source/%.s
	gcc $(ASM_ARGS) -DSERVER -o $@ $<

# ---------------------------------------

utils:
	$(MAKE) -C utils/vcc
	$(MAKE) -C utils/vlumpy
	$(MAKE) -C utils/glvis

progs: utils/bin/vcc$(EXE)
	$(MAKE) VCC=../../utils/bin/vcc$(EXE) OUTDIR=../../basev/doom1/progs -C progs/doom
	$(MAKE) VCC=../../utils/bin/vcc$(EXE) OUTDIR=../../basev/doom2/progs -C progs/doom2
	$(MAKE) VCC=../../utils/bin/vcc$(EXE) OUTDIR=../../basev/heretic/progs -C progs/heretic
	$(MAKE) VCC=../../utils/bin/vcc$(EXE) OUTDIR=../../basev/hexen/progs -C progs/hexen
	$(MAKE) VCC=../../utils/bin/vcc$(EXE) OUTDIR=../../basev/strife/progs -C progs/strife

data: progs $(WAD_FILES)

basev/%/wad0.wad: basev/%/wad0.ls utils/bin/vlumpy$(EXE)
	utils/bin/vlumpy$(EXE) $<

utils/bin/vcc$(EXE):
	$(MAKE) -C utils/vcc

utils/bin/vlumpy$(EXE):
	$(MAKE) -C utils/vlumpy

basev/doom1/wad0.wad : basev/doom1/progs/clprogs.dat basev/doom1/progs/svprogs.dat
basev/doom2/wad0.wad : basev/doom2/progs/clprogs.dat basev/doom2/progs/svprogs.dat
basev/heretic/wad0.wad : basev/heretic/progs/clprogs.dat basev/heretic/progs/svprogs.dat
basev/hexen/wad0.wad : basev/hexen/progs/clprogs.dat basev/hexen/progs/svprogs.dat
basev/strife/wad0.wad : basev/strife/progs/clprogs.dat basev/strife/progs/svprogs.dat

# ---------------------------------------

ifndef INSTALL
INSTALL = ginstall
endif

ifndef INSTALL_DIR
INSTALL_DIR = /usr/local/games/Vavoom
endif

ifndef INSTALL_UTILS_DIR
INSTALL_UTILS_DIR = /usr/local/bin
endif

INSTALL_PARMS=-g root -o root -m 0755
INSTALL_EXEPARMS=-g root -o root -m 4755
INSTALL_DIRPARMS=-m 0777 -d

INSTALL_DIRS = $(INSTALL_DIR) \
		$(INSTALL_DIR)/basev \
		$(INSTALL_DIR)/basev/doom \
		$(INSTALL_DIR)/basev/doom1 \
		$(INSTALL_DIR)/basev/doom2 \
		$(INSTALL_DIR)/basev/tnt \
		$(INSTALL_DIR)/basev/plutonia \
		$(INSTALL_DIR)/basev/heretic \
		$(INSTALL_DIR)/basev/hexen \
		$(INSTALL_DIR)/basev/strife

install: installdata installutils
	$(INSTALL) $(INSTALL_EXEPARMS) Vavoom$(EXE) $(INSTALL_DIR)

installsv: installdata
	$(INSTALL) $(INSTALL_PARMS) VavoomSV$(EXE) $(INSTALL_DIR)

installdata:
	$(INSTALL) $(INSTALL_DIRPARMS) $(INSTALL_DIRS)
	$(INSTALL) $(INSTALL_PARMS) basev/default.cfg basev/startup.vs basev/games.txt $(INSTALL_DIR)/basev
	$(INSTALL) $(INSTALL_PARMS) basev/doom/wad0.wad $(INSTALL_DIR)/basev/doom
	$(INSTALL) $(INSTALL_PARMS) basev/doom1/wad0.wad basev/doom1/base.txt $(INSTALL_DIR)/basev/doom1
	$(INSTALL) $(INSTALL_PARMS) basev/doom2/wad0.wad basev/doom2/base.txt $(INSTALL_DIR)/basev/doom2
	$(INSTALL) $(INSTALL_PARMS) basev/tnt/wad0.wad basev/tnt/base.txt $(INSTALL_DIR)/basev/tnt
	$(INSTALL) $(INSTALL_PARMS) basev/plutonia/wad0.wad basev/plutonia/base.txt $(INSTALL_DIR)/basev/plutonia
	$(INSTALL) $(INSTALL_PARMS) basev/heretic/wad0.wad $(INSTALL_DIR)/basev/heretic
	$(INSTALL) $(INSTALL_PARMS) basev/hexen/wad0.wad $(INSTALL_DIR)/basev/hexen
	$(INSTALL) $(INSTALL_PARMS) basev/strife/wad0.wad $(INSTALL_DIR)/basev/strife

installutils:
	$(INSTALL) $(INSTALL_PARMS) utils/glbsp/glbsp$(EXE) utils/glvis/glvis$(EXE) $(INSTALL_UTILS_DIR)

# ---------------------------------------

clean:
	rm obj/*.o obj/*.obj obj/cl/*.o obj/cl/*.obj obj/sv/*.o obj/sv/*.obj *.tds

# ---------------------------------------
#
#	Generation of asm files for Windows
#
# ---------------------------------------

ASM_FILES = \
	source/d_aclipa.asm \
	source/d_aliasa.asm \
	source/d_edgea.asm \
	source/d_polysa.asm \
	source/d_zspan.asm \
	source/d_varsa.asm \
	source/d8_part.asm \
	source/d8_poly.asm \
	source/d8_s16.asm \
	source/d8_span.asm \
	source/d8_spr.asm \
	source/d8_surf.asm \
	source/d16_part.asm \
	source/d16_poly.asm \
	source/d16_s16.asm \
	source/d16_span.asm \
	source/d16_spr.asm \
	source/d16_surf.asm \
	source/d32_part.asm \
	source/d32_poly.asm \
	source/d32_s16.asm \
	source/d32_span.asm \
	source/d32_spr.asm \
	source/d32_surf.asm \
	source/pr_execa.asm \
	source/sys_i386.asm

asm: $(ASM_FILES)

source/%.asm : source/%.s source/asm_i386.h source/gas2tasm.exe
	gcc -x assembler-with-cpp -E -P -DGAS2TASM $< -o - | source/gas2tasm.exe > $@

source/gas2tasm.exe : source/gas2tasm.c
	gcc -O3 -ffast-math -fomit-frame-pointer -s -o $@ $<

