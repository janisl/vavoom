ifndef VCC
VCC=vcc
endif

all: shared.dat svprogs.dat clprogs.dat

del:
	rm clprogs.dat svprogs.dat

shared.dat: ../doom/doomdefs.vc
	$(VCC) -I../common ../doom/doomdefs.vc shared.dat -P../../basev/common/progs

clprogs.dat: clprogs.vc ../doom/*.vc ../doom/cgame/*.vc ../common/*.vc
	$(VCC) -I../common -I../doom clprogs -P../../basev/common/progs

svprogs.dat: svprogs.vc ../doom/*.vc ../doom/game/*.vc ../common/*.vc *.vc
	$(VCC) -I../common -I../doom svprogs -P../../basev/common/progs

info:
	../../utils/bin/makeinfo -deh hack.deh > hack.log

wad:
	vlumpy hack.ls
