ifndef VCC
VCC = vcc
endif

all: clprogs.dat svprogs.dat

del:
	rm clprogs.dat svprogs.dat

clprogs.dat: *.vc client/*.vc ../common/*.vc
	$(VCC) client/clprogs.vc clprogs.dat

svprogs.dat: *.vc server/*.vc ../common/*.vc
	$(VCC) server/svprogs.vc svprogs.dat

