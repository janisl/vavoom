ifndef VCC
VCC=vcc
endif

all: clprogs.dat svprogs.dat

del:
	rm clprogs.dat svprogs.dat

clprogs.dat: clprogs.vc ../doom/*.vc ../doom/client/*.vc ../common/*.vc
	$(VCC) -I../doom/client clprogs

svprogs.dat: svprogs.vc ../doom/*.vc ../doom/server/*.vc ../common/*.vc
	$(VCC) -I../doom/server svprogs

info:
	makeinfo -deh hack.deh > hack.log

