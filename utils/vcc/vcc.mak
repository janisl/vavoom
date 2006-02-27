# ---------------------------------------------------------------------------
!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif

PROJECT = vcc.exe
OBJFILES = cmdlib.obj constexp.obj error.obj expressn.obj info.obj \
    modifiers.obj name.obj parse.obj parse1.obj pcode.obj stream.obj \
    token.obj types.obj vcc.obj \
    cpp.obj eval.obj hideset.obj include.obj \
    lex.obj macro.obj nlist.obj tokens.obj unix.obj
# ---------------------------------------------------------------------------
SYSDEFINES = _NO_VCL;_ASSERTE;NO_STRICT
INCLUDEPATH = ../common
LIBPATH = $(BCB)\lib
WARNINGS= -w-8026 -w-8027 -w-par -w-8061
# ---------------------------------------------------------------------------
CFLAG1 = -O2 -w -VF -X- -a4 -b -k -vi -q -c
LFLAGS = -ap -Tpe -x -Gn -v -q
# ---------------------------------------------------------------------------
ALLOBJ = c0x32.obj $(OBJFILES)
ALLLIB = import32.lib cw32.lib
# ---------------------------------------------------------------------------

.autodepend
# ---------------------------------------------------------------------------
!if "$(USERDEFINES)" != ""
AUSERDEFINES = -d$(USERDEFINES:;= -d)
!else
AUSERDEFINES =
!endif

.PATH.CPP = .;../common;vcpp

# ---------------------------------------------------------------------------
$(PROJECT): $(OBJFILES)
    $(BCB)\BIN\ilink32 @&&!
    $(LFLAGS) -L$(LIBPATH) +
    $(ALLOBJ), +
    $(PROJECT),, +
    $(ALLLIB)
!
# ---------------------------------------------------------------------------
.cpp.obj:
    $(BCB)\BIN\bcc32 $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n$(@D) {$< }
