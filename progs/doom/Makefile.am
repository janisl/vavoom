VCC=../../utils/vcc/vcc$(EXEEXT)
OUTDIR=../../basev/doom1/progs
COMMON_DIR=$(srcdir)/../common
COMMON_PROGS_DIR=../../basev/common/progs

noinst_DATA = $(OUTDIR)/shared.dat $(OUTDIR)/clprogs.dat $(OUTDIR)/svprogs.dat

$(OUTDIR)/shared.dat: $(VCC) $(srcdir)/*.vc $(COMMON_PROGS_DIR)/common.dat
	@mkdir -p $(OUTDIR)
	$(VCC) -I$(COMMON_DIR) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) $(srcdir)/doomdefs.vc $@

$(OUTDIR)/clprogs.dat: $(VCC) $(srcdir)/*.vc $(srcdir)/cgame/*.vc $(OUTDIR)/shared.dat $(COMMON_PROGS_DIR)/clcommon.dat
	$(VCC) -I$(COMMON_DIR) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) $(srcdir)/clprogs.vc $@

$(OUTDIR)/svprogs.dat: $(VCC) $(srcdir)/*.vc $(srcdir)/game/*.vc $(srcdir)/game1/*.vc \
		$(OUTDIR)/shared.dat $(COMMON_PROGS_DIR)/svcommon.dat
	$(VCC) -I$(COMMON_DIR) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) $(srcdir)/svprogs.vc $@

EXTRA_DIST = *.vc cgame/*.vc game/*.vc game1/*.vc game2/*.vc compile.bat
