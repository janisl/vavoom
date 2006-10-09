VCC=../../utils/bin/vcc$(EXEEXT)
OUTDIR=../../basev/common/progs

noinst_DATA = $(OUTDIR)/engine.dat $(OUTDIR)/linespec.dat

$(OUTDIR)/engine.dat: $(VCC) $(srcdir)/*.vc $(srcdir)/engine/*.vc
	@mkdir -p $(OUTDIR)
	$(VCC) -P$(OUTDIR) $(srcdir)/engine/classes.vc $@

$(OUTDIR)/linespec.dat: $(VCC) $(OUTDIR)/engine.dat $(srcdir)/linespec/*.vc
	$(VCC) -P$(OUTDIR) $(srcdir)/linespec/classes.vc $@

EXTRA_DIST = *.vc engine/*.vc linespec/*.vc
