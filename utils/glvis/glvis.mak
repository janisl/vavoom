!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif

OBJS = cmdlib.obj wadlib.obj glvis.obj level.obj flow.obj

all: glvis.exe

glvis.exe: $(OBJS)
    ilink32 -ap -x -Gn -q -L$(BCB)\lib c0x32.obj $(OBJS), $*,, import32.lib cw32.lib

.cpp.obj:
    bcc32 -c -O2 -q {$< }

