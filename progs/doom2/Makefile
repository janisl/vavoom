ifndef VCC
VCC=vcc
endif

ifndef OUTDIR
OUTDIR=.
endif

all: $(OUTDIR)/clprogs.dat $(OUTDIR)/svprogs.dat
clprogs: $(OUTDIR)/clprogs.dat
svprogs: $(OUTDIR)/svprogs.dat

del:
	rm $(OUTDIR)/clprogs.dat $(OUTDIR)/svprogs.dat

$(OUTDIR)/clprogs.dat: clprogs.vc ../doom/*.vc ../doom/client/*.vc ../common/*.vc
	$(VCC) -I../common -Icommon clprogs.vc $@

$(OUTDIR)/svprogs.dat: svprogs.vc ../doom/*.vc ../doom/server/*.vc ../common/*.vc
	$(VCC) -I../common -Icommon svprogs.vc $@

