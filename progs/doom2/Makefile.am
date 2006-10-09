VCC=../../utils/bin/vcc$(EXEEXT)
OUTDIR=../../basev/doom2/progs
COMMON_DIR=$(srcdir)/../common
COMMON_PROGS_DIR=../../basev/common/progs

noinst_DATA = $(OUTDIR)/shared.dat $(OUTDIR)/svprogs.dat $(OUTDIR)/clprogs.dat

$(OUTDIR)/shared.dat: $(VCC) $(srcdir)/shared.vc $(srcdir)/../doom/*.vc $(COMMON_PROGS_DIR)/engine.dat
	@mkdir -p $(OUTDIR)
	$(VCC) -I$(COMMON_DIR) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) $(srcdir)/shared.vc $@

$(OUTDIR)/svprogs.dat: $(VCC) $(OUTDIR)/shared.dat $(srcdir)/svprogs.vc \
		$(srcdir)/../doom/*.vc $(srcdir)/../doom/game/*.vc $(srcdir)/../doom/game1/*.vc \
		$(srcdir)/../doom/game2/*.vc $(COMMON_PROGS_DIR)/linespec.dat
	$(VCC) -I$(COMMON_DIR) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) $(srcdir)/svprogs.vc $@

$(OUTDIR)/clprogs.dat: $(VCC) $(srcdir)/clprogs.vc $(srcdir)/../doom/*.vc \
		$(srcdir)/../doom/cgame/*.vc $(OUTDIR)/svprogs.dat
	$(VCC) -I$(COMMON_DIR) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) $(srcdir)/clprogs.vc $@

EXTRA_DIST = *.vc compile.bat
