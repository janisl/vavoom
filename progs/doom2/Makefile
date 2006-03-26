ifndef VCC
VCC=vcc
endif

ifndef OUTDIR
OUTDIR=.
endif

ifndef COMMON_DIR
COMMON_DIR=../common
endif

ifndef COMMON_PROGS_DIR
COMMON_PROGS_DIR=../../basev/common/progs
endif

all: $(OUTDIR)/shared.dat $(OUTDIR)/clprogs.dat $(OUTDIR)/svprogs.dat
shared: $(OUTDIR)/shared.dat
clprogs: $(OUTDIR)/clprogs.dat
svprogs: $(OUTDIR)/svprogs.dat

del:
	rm $(OUTDIR)/shared.dat $(OUTDIR)/clprogs.dat $(OUTDIR)/svprogs.dat

$(OUTDIR)/shared.dat: $(VCC) shared.vc ../doom/*.vc $(COMMON_DIR)/*.vc $(COMMON_DIR)/shared/*.vc
	$(VCC) -I$(COMMON_DIR) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) shared.vc $@

$(OUTDIR)/clprogs.dat: $(VCC) clprogs.vc ../doom/*.vc ../doom/cgame/*.vc $(COMMON_DIR)/*.vc $(COMMON_DIR)/shared/*.vc $(COMMON_DIR)/client/*.vc
	$(VCC) -I$(COMMON_DIR) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) clprogs.vc $@

$(OUTDIR)/svprogs.dat: $(VCC) svprogs.vc ../doom/*.vc ../doom/game/*.vc ../doom/game1/*.vc ../doom/game2/*.vc $(COMMON_DIR)/*.vc $(COMMON_DIR)/shared/*.vc $(COMMON_DIR)/server/*.vc
	$(VCC) -I$(COMMON_DIR) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) svprogs.vc $@
