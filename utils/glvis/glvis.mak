!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif

LIB_OBJS = cmdlib.obj wadlib.obj level.obj flow.obj
OBJS = glvis.obj

.path.cpp = .;../common

all: glvis.exe

glvis.exe: $(OBJS) glvis.lib
    ilink32 -ap -x -Gn -q -L$(BCB)\lib c0x32.obj $(OBJS) glvis.lib, $*,, import32.lib cw32.lib

glvis.lib: $(LIB_OBJS)
    tlib $@ /a $(LIB_OBJS)

.cpp.obj:
    bcc32 -c -O2 -q -I../common -I$(BCB)/include {$< }

