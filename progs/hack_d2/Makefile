ifndef VCC
VCC=vcc
endif

all: clprogs.dat svprogs.dat

del:
	rm clprogs.dat svprogs.dat

clprogs.dat: clprogs.c ../doom/*.c ../doom/client/*.c ../common/*.c
	$(VCC) -I../doom/client clprogs

svprogs.dat: svprogs.c ../doom/*.c ../doom/server/*.c ../common/*.c
	$(VCC) -I../doom/server svprogs

info:
	makeinfo -deh hack.deh > hack.log

