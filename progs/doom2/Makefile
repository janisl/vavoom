ifndef VCC
VCC=vcc
endif

all: ../../basev/doom2/progs/clprogs.dat ../../basev/doom2/progs/svprogs.dat
clprogs: ../../basev/doom2/progs/clprogs.dat
svprogs: ../../basev/doom2/progs/svprogs.dat

del:
	rm ../../basev/doom2/progs/clprogs.dat ../../basev/doom2/progs/svprogs.dat

../../basev/doom2/progs/clprogs.dat: clprogs.vc ../doom/*.vc ../doom/client/*.vc ../common/*.vc
	$(VCC) clprogs.vc $@

../../basev/doom2/progs/svprogs.dat: svprogs.vc ../doom/*.vc ../doom/server/*.vc ../common/*.vc
	$(VCC) svprogs.vc $@

