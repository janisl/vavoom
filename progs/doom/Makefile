ifndef VCC
VCC = vcc
endif

all: ../../basev/doom1/progs/clprogs.dat ../../basev/doom1/progs/svprogs.dat
clprogs: ../../basev/doom1/progs/clprogs.dat
svprogs: ../../basev/doom1/progs/svprogs.dat

del:
	rm ../../basev/doom1/progs/clprogs.dat ../../basev/doom1/progs/svprogs.dat

../../basev/doom1/progs/clprogs.dat: *.vc client/*.vc ../common/*.vc
	$(VCC) client/clprogs.vc ../../basev/doom1/progs/clprogs.dat

../../basev/doom1/progs/svprogs.dat: *.vc server/*.vc ../common/*.vc
	$(VCC) server/svprogs.vc ../../basev/doom1/progs/svprogs.dat

