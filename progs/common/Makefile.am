VCC=../../utils/vcc/vcc$(EXEEXT)
OUTDIR=../../basev/common/progs

noinst_DATA = $(OUTDIR)/common.dat $(OUTDIR)/clcommon.dat $(OUTDIR)/svcommon.dat

$(OUTDIR)/common.dat: $(VCC) $(srcdir)/*.vc $(srcdir)/shared/*.vc
	@mkdir -p $(OUTDIR)
	$(VCC) -P$(OUTDIR) $(srcdir)/common.vc $@

$(OUTDIR)/clcommon.dat: $(VCC) $(OUTDIR)/common.dat $(srcdir)/*.vc $(srcdir)/shared/*.vc $(srcdir)/client/*.vc
	$(VCC) -P$(OUTDIR) $(srcdir)/clcommon.vc $@

$(OUTDIR)/svcommon.dat: $(VCC) $(OUTDIR)/common.dat $(srcdir)/*.vc $(srcdir)/shared/*.vc $(srcdir)/server/*.vc
	$(VCC) -P$(OUTDIR) $(srcdir)/svcommon.vc $@

EXTRA_DIST = *.vc shared/*.vc client/*.vc server/*.vc
