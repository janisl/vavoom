VCC=../../utils/bin/vcc$(EXEEXT)
OUTDIR=../../basev/common/progs

noinst_DATA = $(OUTDIR)/engine.dat $(OUTDIR)/svcommon.dat

$(OUTDIR)/engine.dat: $(VCC) $(srcdir)/*.vc $(srcdir)/engine/*.vc
	@mkdir -p $(OUTDIR)
	$(VCC) -P$(OUTDIR) $(srcdir)/engine/classes.vc $@

$(OUTDIR)/svcommon.dat: $(VCC) $(OUTDIR)/engine.dat $(srcdir)/*.vc $(srcdir)/server/*.vc
	$(VCC) -P$(OUTDIR) $(srcdir)/svcommon.vc $@

EXTRA_DIST = *.vc engine/*.vc server/*.vc
