VCC=../../utils/vcc/vcc$(EXEEXT)
OUTDIR=../../basev/heretic/progs
COMMON_DIR=$(srcdir)/../common
COMMON_PROGS_DIR=../../basev/common/progs

noinst_DATA = $(OUTDIR)/shared.dat $(OUTDIR)/clprogs.dat $(OUTDIR)/svprogs.dat

$(OUTDIR)/shared.dat: $(VCC) $(srcdir)/*.vc $(COMMON_PROGS_DIR)/common.dat
	@mkdir -p $(OUTDIR)
	$(VCC) -I$(COMMON_DIR) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) $(srcdir)/herdefs.vc $@

$(OUTDIR)/clprogs.dat: $(VCC) $(OUTDIR)/shared.dat $(srcdir)/*.vc $(srcdir)/cgame/*.vc \
		$(COMMON_PROGS_DIR)/clcommon.dat
	$(VCC) -I$(COMMON_DIR) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) $(srcdir)/clprogs.vc $@

$(OUTDIR)/svprogs.dat: $(VCC) $(OUTDIR)/shared.dat $(srcdir)/*.vc $(srcdir)/game/*.vc \
		$(COMMON_PROGS_DIR)/svcommon.dat
	$(VCC) -I$(COMMON_DIR) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) $(srcdir)/svprogs.vc $@

EXTRA_DIST = *.vc cgame/*.vc game/*.vc compile.bat
