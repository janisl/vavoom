!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif

OBJS = action.obj ddf2vc.obj ddf_anim.obj ddf_atk.obj ddf_boom.obj \
	ddf_colm.obj ddf_game.obj ddf_lang.obj ddf_levl.obj ddf_line.obj \
	ddf_main.obj ddf_mobj.obj ddf_mus.obj ddf_sect.obj ddf_sfx.obj \
	ddf_stat.obj ddf_swth.obj ddf_weap.obj m_argv.obj m_crc32.obj \
	scripts.obj vc_mobj.obj vc_weapn.obj z_zone.obj

.path.c = .

all: ddf2vc.exe

ddf2vc.exe: $(OBJS)
	ilink32 -ap -x -Gn -q -L$(BCB)\lib c0x32.obj $(OBJS), $*,, import32.lib cw32.lib

.c.obj:
	bcc32 -c -O2 -q -DWIN32 {$< }

