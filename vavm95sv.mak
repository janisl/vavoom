# ---------------------------------------------------------------------------
!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif

# ---------------------------------------------------------------------------
CFLAGS = -O2 -Vx -Ve -X- -a8 -4 -b -k- -vi -tWC -tWM- -c -w -DSERVER;_NO_VCL
AFLAGS = /ml /zn
LFLAGS = -Iobj\sv -ap -Tpe -x -Gn

PROJECT = vavm95sv.exe
OBJFILES = obj\sv\zone.obj obj\sv\crc.obj obj\sv\cvar.obj obj\sv\debug.obj \
    obj\sv\host.obj obj\sv\maths.obj obj\sv\message.obj obj\sv\misc.obj \
    obj\sv\net_dgrm.obj obj\sv\net_loop.obj obj\sv\net_main.obj \
    obj\sv\net_win.obj obj\sv\net_wins.obj obj\sv\net_wipx.obj \
    obj\sv\mapinfo.obj obj\sv\p_setup.obj obj\sv\pr_cmds.obj \
    obj\sv\pr_exec.obj obj\sv\sc_man.obj obj\sv\sizebuf.obj \
    obj\sv\sv_main.obj obj\sv\sv_save.obj obj\sv\wad.obj obj\sv\cmd.obj \
    obj\sv\sys_wind.obj obj\sv\sv_user.obj obj\sv\sv_poly.obj \
    obj\sv\sv_sight.obj obj\sv\sv_swtch.obj obj\sv\sv_tick.obj \
    obj\sv\sv_acs.obj obj\sv\sv_ent.obj obj\sv\sv_world.obj \
    obj\sv\infostr.obj obj\sv\r_tex.obj obj\sv\files.obj obj\sv\s_data.obj \
    obj\sv\name.obj obj\sv\vobject.obj obj\sv\vclass.obj obj\sv\net_null.obj \
    obj\sv\pr_execa.obj
# ---------------------------------------------------------------------------
.autodepend
# ---------------------------------------------------------------------------
.PATH.CPP = source
.PATH.ASM = source
# ---------------------------------------------------------------------------
$(PROJECT): $(OBJFILES)
    ilink32 @&&!
    $(LFLAGS) -L$(BCB)\lib c0x32.obj $(OBJFILES), $(PROJECT),, +
    import32.lib cw32.lib
!
# ---------------------------------------------------------------------------
.cpp.obj:
    bcc32 $(CFLAGS) -n$(@D) {$< }

.asm.obj:
    tasm32 $(AFLAGS) $<, $@
# ---------------------------------------------------------------------------
