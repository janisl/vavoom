ifndef VCC
VCC=vcc
endif

ifndef OUTDIR
OUTDIR=.
endif

ifndef COMMON_DIR
COMMON_DIR=../common
endif

all: $(OUTDIR)/clprogs.dat $(OUTDIR)/svprogs.dat
clprogs: $(OUTDIR)/clprogs.dat
svprogs: $(OUTDIR)/svprogs.dat

del:
	rm $(OUTDIR)/clprogs.dat $(OUTDIR)/svprogs.dat

$(OUTDIR)/clprogs.dat: $(VCC) clprogs.vc ../doom/*.vc ../doom/cgame/*.vc $(COMMON_DIR)/*.vc $(COMMON_DIR)/shared/*.vc $(COMMON_DIR)/client/*.vc
	$(VCC) -I$(COMMON_DIR) clprogs.vc $@

$(OUTDIR)/svprogs.dat: $(VCC) svprogs.vc ../doom/*.vc ../doom/server/*.vc ../doom/game/*.vc ../doom/game1/*.vc ../doom/game2/*.vc $(COMMON_DIR)/*.vc $(COMMON_DIR)/shared/*.vc $(COMMON_DIR)/server/*.vc
	$(VCC) -I$(COMMON_DIR) svprogs.vc $@

#---------------------------------------
# Debugging
#---------------------------------------

debug: clprogs.dat svprogs.dat
debugcl: clprogs.dat
debugsv: svprogs.dat

deldebug:
	rm clprogs.dat svprogs.dat

clprogs.dat:
	$(VCC) -I$(COMMON_DIR) clprogs.vc $@ -dcldebug.txt $(DUMP_ASM)

svprogs.dat:
	$(VCC) -I$(COMMON_DIR) svprogs.vc $@ -dsvdebug.txt $(DUMP_ASM)

