#---------------------------------------
#
#	Some configurations
#
#---------------------------------------

# Uncomment to compile a debug version
#DEBUG = 1

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
	obj/sys_dosa.o \
	obj/sys_dos.o
ifndef NOGL
SYS_OBJS += obj/gl_alleg.o
endif
LIBS := -lalleg -lstdcxx
else
SYS_OBJS = \
	obj/cd_linux.o \
	obj/in_alleg.o \
	obj/net_bsd.o \
	obj/net_udp.o \
	obj/sys_lina.o \
	obj/sys_lin.o
ifndef NOGL
SYS_OBJS += obj/gl_x.o
endif
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
	obj/d_alleg.o \
	obj/d_data.o \
	obj/d_draw.o \
	obj/d_main.o \
	obj/d_part.o \
	obj/d_scache.o \
	obj/d_scan.o \
	obj/d_span.o \
	obj/d_sprite.o \
	obj/d_surf.o \
	obj/d_tex.o \
	obj/d_wipe.o \
	obj/d_varsa.o \
	obj/d_zspan.o \
	obj/d8_part.o \
	obj/d8_s16.o \
	obj/d8_span.o \
	obj/d8_spr.o \
	obj/d8_surf.o \
	obj/d16_part.o \
	obj/d16_s16.o \
	obj/d16_span.o \
	obj/d16_spr.o \
	obj/d16_surf.o \
	obj/d32_part.o \
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
	obj/maths.o \
	obj/menu.o \
	obj/message.o \
	obj/misc.o \
	obj/msgbox.o \
	obj/net_dgrm.o \
	obj/net_loop.o \
	obj/net_main.o \
	obj/p_info.o \
	obj/p_setup.o \
	obj/pr_cmds.o \
	obj/pr_exec.o \
	obj/pr_execa.o \
	obj/r_border.o \
	obj/r_bsp.o \
	obj/r_light.o \
	obj/r_main.o \
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
	obj/v_draw.o \
	obj/wad.o \
	obj/zone.o

ifndef NOGL
OBJ_FILES += \
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
	jl/doomx/palette.wad \
	jl/heretic/palette.wad \
	jl/hexen/palette.wad \
	jl/strife/palette.wad \
	jl/jldoom.wad \
	jl/doomdat.wad \
	jl/doom2dat.wad \
	jl/tntdat.wad \
	jl/plutdat.wad \
	jl/d2progs.wad \
	jl/hticdat.wad \
	jl/hexendat.wad \
	jl/strifdat.wad

# ---------------------------------------

ifdef DEBUG

C_ARGS = -W -Wall -O3 -ffast-math -fomit-frame-pointer -c
CPP_ARGS = -W -Wall -O -ffast-math -g -fno-rtti -fno-exceptions -c
ASM_ARGS = -x assembler-with-cpp -W -Wall -c
LINK_ARGS = -Wall

else

C_ARGS = -W -Wall -O3 -ffast-math -fomit-frame-pointer -mpentiumpro -c
CPP_ARGS = -W -Wall -O3 -ffast-math -fomit-frame-pointer \
	-fno-rtti -fno-exceptions -mpentiumpro -c
ASM_ARGS = -x assembler-with-cpp -W -Wall -c
LINK_ARGS = -Wall -s

endif

# ---------------------------------------

.PHONY: all suid crash sv progs data clean

# ---------------------------------------

all: Vavoom$(EXE)

Vavoom$(EXE): $(OBJ_FILES)
	gcc $(LINK_ARGS) -o $@ $(OBJ_FILES) $(LIBS)

suid:
	chown root.root Vavoom
	chmod 4755 Vavoom

crash:
	addr2line -e Vavoom -f < crash.txt >> jl/debug.txt

obj/%.o : source/%.c
	gcc $(C_ARGS) -o $@ $<

obj/%.o : source/%.cpp source/*.h
	gcc $(CPP_ARGS) -o $@ $<

obj/%.o : source/%.s source/asm_i386.h
	gcc $(ASM_ARGS) -o $@ $<

# ---------------------------------------

sv: VavoomSV$(EXE)

VavoomSV$(EXE): $(SV_OBJ_FILES)
	gcc $(LINK_ARGS) -o $@ $(SV_OBJ_FILES) $(SV_LIBS)

obj/sv/%.o : source/%.c
	gcc $(C_ARGS) -DSERVER -o $@ $<

obj/sv/%.o : source/%.cpp source/*.h
	gcc $(CPP_ARGS) -DSERVER -o $@ $<

obj/sv/%.o : source/%.s
	gcc $(ASM_ARGS) -DSERVER -o $@ $<

# ---------------------------------------

progs: utils/vcc/vcc$(EXE)
	$(MAKE) VCC=../../utils/vcc/vcc$(EXE) -C progs/doom
	$(MAKE) VCC=../../utils/vcc/vcc$(EXE) -C progs/doom2
	$(MAKE) VCC=../../utils/vcc/vcc$(EXE) -C progs/heretic
	$(MAKE) VCC=../../utils/vcc/vcc$(EXE) -C progs/hexen
	$(MAKE) VCC=../../utils/vcc/vcc$(EXE) -C progs/strife

data: progs $(WAD_FILES)

jl/%/palette.wad: jl/%/palette.ls utils/vlumpy/vlumpy$(EXE)
	utils/vlumpy/vlumpy$(EXE) $<

jl/%.wad: jl/%.ls utils/vlumpy/vlumpy$(EXE)
	utils/vlumpy/vlumpy$(EXE) $<

utils/vcc/vcc$(EXE):
	$(MAKE) -C utils/vcc

utils/vlumpy/vlumpy$(EXE):
	$(MAKE) -C utils/vlumpy

jl/doomdat.wad : progs/doom/clprogs.dat progs/doom/svprogs.dat
jl/d2progs.wad : progs/doom2/clprogs.dat progs/doom2/svprogs.dat
jl/hticdat.wad : progs/heretic/clprogs.dat progs/heretic/svprogs.dat
jl/hexendat.wad : progs/hexen/clprogs.dat progs/hexen/svprogs.dat
jl/strifdat.wad : progs/strife/clprogs.dat progs/strife/svprogs.dat

# ---------------------------------------

clean:
	rm obj/*.o obj/*.obj obj/cl/*.o obj/cl/*.obj obj/sv/*.o obj/sv/*.obj *.tds

# ---------------------------------------
#
#	Generation of asm files for Windows
#
# ---------------------------------------

ASM_FILES = \
	source/d_zspan.asm \
	source/d_varsa.asm \
	source/d8_part.asm \
	source/d8_s16.asm \
	source/d8_span.asm \
	source/d8_spr.asm \
	source/d8_surf.asm \
	source/d16_part.asm \
	source/d16_s16.asm \
	source/d16_span.asm \
	source/d16_spr.asm \
	source/d16_surf.asm \
	source/d32_part.asm \
	source/d32_span.asm \
	source/d32_spr.asm \
	source/d32_surf.asm \
	source/pr_execa.asm

asm: $(ASM_FILES)

source/%.asm : source/%.s source/asm_i386.h source/gas2tasm.exe
	gcc -x assembler-with-cpp -E -P -DGAS2TASM $< -o source/temp.i
	cp source/template.asm $@
	source/gas2tasm.exe < source/temp.i >> $@
	rm source/temp.i

source/gas2tasm.exe : source/gas2tasm.c
	gcc -O3 -ffast-math -fomit-frame-pointer -m486 -s -o $@ $<

