ifndef VCC
VCC = vcc
endif

ifndef OUTDIR
OUTDIR=.
endif

all: $(OUTDIR)/common.dat $(OUTDIR)/clcommon.dat $(OUTDIR)/svcommon.dat

del:
	rm $(OUTDIR)/common.dat $(OUTDIR)/clcommon.dat $(OUTDIR)/svcommon.dat

$(OUTDIR)/common.dat: $(VCC) *.vc shared/*.vc
	$(VCC) -P$(OUTDIR) common.vc $@

$(OUTDIR)/clcommon.dat: $(VCC) *.vc shared/*.vc client/*.vc
	$(VCC) -P$(OUTDIR) clcommon.vc $@

$(OUTDIR)/svcommon.dat: $(VCC) *.vc shared/*.vc server/*.vc
	$(VCC) -P$(OUTDIR) svcommon.vc $@
