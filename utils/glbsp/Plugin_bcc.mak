#
# glBSP Makefile for Borland C++ Plugin
#
!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif
MAIN=.
OUTPUT=.
CFLAGS=-O2 -tWC -pc
DEFINES=-DWIN32 -DGLBSP_PLUGIN

OBJS=$(OUTPUT)\analyze.obj  \
     $(OUTPUT)\blockmap.obj \
     $(OUTPUT)\glbsp.obj    \
     $(OUTPUT)\level.obj    \
     $(OUTPUT)\node.obj     \
     $(OUTPUT)\reject.obj   \
     $(OUTPUT)\seg.obj      \
     $(OUTPUT)\system.obj   \
     $(OUTPUT)\util.obj     \
     $(OUTPUT)\wad.obj
 
#
# Main Source Compilation
# ^^^^^^^^^^^^^^^^^^^^^^^
{$(MAIN)\}.c{$(OUTPUT)\}.obj:
  bcc32 $(CFLAGS) $(DEFINES) -c -n$(OUTPUT) {$< }

#
# Make the library
# ^^^^^^^^^^^^^^^^
libglbsp.lib: $(OBJS)
  tlib $@ /a $(OBJS)

