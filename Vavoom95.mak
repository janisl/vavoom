# ---------------------------------------------------------------------------
!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif

# ---------------------------------------------------------------------------
# IDE SECTION
# ---------------------------------------------------------------------------
# The following section of the project makefile is managed by the BCB IDE.
# It is recommended to use the IDE to change any of the values in this
# section.
# ---------------------------------------------------------------------------

VERSION = BCB.05.03
# ---------------------------------------------------------------------------
PROJECT = Vavoom95.exe
OBJFILES = obj\chat.obj obj\cheats.obj \
    obj\cmd.obj obj\console.obj obj\crc.obj obj\cvar.obj obj\debug.obj \
    obj\finale.obj obj\host.obj obj\iline.obj obj\imission.obj \
    obj\sys_i386.obj obj\misc.obj obj\p_info.obj obj\p_setup.obj \
    obj\pr_cmds.obj obj\pr_exec.obj \
    obj\sc_man.obj \
    obj\sizebuf.obj obj\sys_win.obj \
    obj\cl_main.obj obj\cl_parse.obj obj\cl_poly.obj obj\cl_trace.obj \
    obj\cl_input.obj obj\pr_execa.obj obj\maths.obj obj\cl_demo.obj \
    obj\wad.obj obj\zone.obj obj\message.obj obj\infostr.obj obj\files.obj \
    \
    obj\net_dgrm.obj obj\net_loop.obj obj\net_main.obj obj\net_win.obj \
    obj\net_wins.obj obj\net_wipx.obj \
    \
    obj\sv_acs.obj \
    obj\sv_main.obj obj\sv_poly.obj obj\sv_save.obj obj\sv_sight.obj \
    obj\sv_swtch.obj obj\sv_user.obj obj\sv_tick.obj obj\sv_world.obj \
    \
    obj\in_input.obj obj\in_win32.obj \
    \
    obj\cd_win32.obj obj\s_data.obj obj\s_sound.obj obj\s_win32.obj \
    obj\s_win32m.obj obj\sn_sonix.obj \
    \
    obj\am_map.obj obj\menu.obj obj\sbar.obj obj\screen.obj obj\text.obj \
    obj\v_draw.obj obj\model.obj \
        \
    obj\r_bsp.obj obj\r_light.obj obj\r_main.obj obj\r_sky.obj \
    obj\r_surf.obj obj\r_tex.obj obj\r_things.obj \
    \
    obj\d_aclip.obj obj\d_alias.obj obj\d_data.obj obj\d_draw.obj \
    obj\d_edge.obj obj\d_main.obj obj\d_part.obj obj\d_polyse.obj \
    obj\d_scache.obj obj\d_span.obj obj\d_sprite.obj obj\d_surf.obj \
    obj\d_tex.obj obj\d_win32.obj \
    obj\d_aclipa.obj obj\d_aliasa.obj obj\d_edgea.obj obj\d_polysa.obj \
    obj\d_varsa.obj obj\d_zspan.obj \
    obj\d8_part.obj obj\d8_poly.obj obj\d8_s16.obj obj\d8_span.obj \
    obj\d8_spr.obj obj\d8_surf.obj \
    obj\d16_part.obj obj\d16_poly.obj obj\d16_s16.obj obj\d16_span.obj \
    obj\d16_spr.obj obj\d16_surf.obj \
    obj\d32_part.obj obj\d32_poly.obj obj\d32_s16.obj obj\d32_span.obj \
    obj\d32_spr.obj obj\d32_surf.obj \
    \
    obj\d3d_draw.obj obj\d3d_info.obj obj\d3d_main.obj obj\d3d_poly.obj \
    obj\d3d_tex.obj \
    \
    obj\gl_draw.obj obj\gl_main.obj obj\gl_poly.obj obj\gl_tex.obj \
    obj\gl_win32.obj

RESFILES = source\vavoom.RES
MAINSOURCE = Vavoom95.bpf
RESDEPEN = $(RESFILES)
LIBFILES =
IDLFILES =
IDLGENFILES =
LIBRARIES =
PACKAGES =
SPARELIBS =
DEFFILE =
# ---------------------------------------------------------------------------
PATHCPP = .;source
PATHASM = .;source
PATHPAS = .;
PATHRC = .;
DEBUGLIBPATH = $(BCB)\lib\debug
RELEASELIBPATH = $(BCB)\lib\release
USERDEFINES = _RWSTD_NO_EXCEPTIONS
SYSDEFINES = _NO_VCL
INCLUDEPATH = "c:\mssdk\include\"
LIBPATH = $(BCB)\lib
WARNINGS= -w-par -w-8027 -w-8026
# ---------------------------------------------------------------------------
CFLAG1 = -O2 -x- -RT- -X- -a4 -4 -b -k- -vi -q -c
IDLCFLAGS = -I"c:\mssdk\include\." -src_suffix cpp -DNDEBUG \
    -D_RWSTD_NO_EXCEPTIONS
PFLAGS = -N2obj -N0obj -$Y- -$L- -$D-
RFLAGS = /l 0x426 /d "NDEBUG" /i$(BCB)\include;$(BCB)\include\mfc
AFLAGS = /ml /w2 /zn
LFLAGS = -Iobj -D"" -aa -Tpe -x -Gn -w -q
# ---------------------------------------------------------------------------
ALLOBJ = c0w32.obj $(OBJFILES)
ALLRES = $(RESFILES)
ALLLIB = $(LIBFILES) $(LIBRARIES) import32.lib cw32.lib
# ---------------------------------------------------------------------------
!ifdef IDEOPTIONS

[Version Info]
IncludeVerInfo=0
AutoIncBuild=0
MajorVer=1
MinorVer=0
Release=0
Build=0
Debug=0
PreRelease=0
Special=0
Private=0
DLL=0

[Version Info Keys]
CompanyName=
FileDescription=
FileVersion=1.0.0.0
InternalName=
LegalCopyright=
LegalTrademarks=
OriginalFilename=
ProductName=
ProductVersion=1.0.0.0
Comments=

[Debugging]
DebugSourceDirs=$(BCB)\source\vcl

!endif





# ---------------------------------------------------------------------------
# MAKE SECTION
# ---------------------------------------------------------------------------
# This section of the project file is not used by the BCB IDE.  It is for
# the benefit of building from the command-line using the MAKE utility.
# ---------------------------------------------------------------------------

.autodepend
# ---------------------------------------------------------------------------
!if "$(USERDEFINES)" != ""
AUSERDEFINES = -d$(USERDEFINES:;= -d)
!else
AUSERDEFINES =
!endif

!if !$d(BCC32)
BCC32 = bcc32
!endif

!if !$d(CPP32)
CPP32 = cpp32
!endif

!if !$d(DCC32)
DCC32 = dcc32
!endif

!if !$d(TASM32)
TASM32 = tasm32
!endif

!if !$d(LINKER)
LINKER = ilink32
!endif

!if !$d(BRCC32)
BRCC32 = brcc32
!endif


# ---------------------------------------------------------------------------
!if $d(PATHCPP)
.PATH.CPP = $(PATHCPP)
.PATH.C   = $(PATHCPP)
!endif

!if $d(PATHPAS)
.PATH.PAS = $(PATHPAS)
!endif

!if $d(PATHASM)
.PATH.ASM = $(PATHASM)
!endif

!if $d(PATHRC)
.PATH.RC  = $(PATHRC)
!endif
# ---------------------------------------------------------------------------
$(PROJECT): $(IDLGENFILES) $(OBJFILES) $(RESDEPEN) $(DEFFILE)
    $(BCB)\BIN\$(LINKER) @&&!
    $(LFLAGS) -L$(LIBPATH) +
    $(ALLOBJ), +
    $(PROJECT),, +
    $(ALLLIB), +
    $(DEFFILE), +
    $(ALLRES)
!
# ---------------------------------------------------------------------------
.pas.hpp:
    $(BCB)\BIN\$(DCC32) $(PFLAGS) -U$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -O$(INCLUDEPATH) --BCB {$< }

.pas.obj:
    $(BCB)\BIN\$(DCC32) $(PFLAGS) -U$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -O$(INCLUDEPATH) --BCB {$< }

.cpp.obj:
    echo $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n$(@D) {$< } > obj\cline
    $(BCC32) @obj\cline

.c.obj:
    $(BCB)\BIN\$(BCC32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n$(@D) {$< }

.c.i:
    $(BCB)\BIN\$(CPP32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n. {$< }

.cpp.i:
    $(BCB)\BIN\$(CPP32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n. {$< }

.asm.obj:
    $(BCB)\BIN\$(TASM32) $(AFLAGS) -i$(INCLUDEPATH:;= -i) $(AUSERDEFINES) -d$(SYSDEFINES:;= -d) $<, $@

.rc.res:
    $(BCB)\BIN\$(BRCC32) $(RFLAGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -fo$@ $<
# ---------------------------------------------------------------------------




