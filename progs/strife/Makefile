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

$(OUTDIR)/clprogs.dat: *.vc client/*.vc ../common/*.vc
	$(VCC) -I../common -Icommon client/clprogs.vc $@

$(OUTDIR)/svprogs.dat: *.vc server/*.vc ../common/*.vc
	$(VCC) -I../common -Icommon server/svprogs.vc $@

