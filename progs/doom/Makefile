ifndef VCC
VCC = vcc
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

$(OUTDIR)/clprogs.dat: $(VCC) *.vc cgame/*.vc $(COMMON_DIR)/*.vc $(COMMON_DIR)/shared/*.vc $(COMMON_DIR)/client/*.vc
	$(VCC) -I$(COMMON_DIR) clprogs.vc $@

$(OUTDIR)/svprogs.dat: $(VCC) *.vc server/*.vc game/*.vc game1/*.vc $(COMMON_DIR)/*.vc $(COMMON_DIR)/shared/*.vc $(COMMON_DIR)/server/*.vc
	$(VCC) -I$(COMMON_DIR) server/svprogs.vc $@

#---------------------------------------
# Debugging
#---------------------------------------

debug: clprogs.dat server/svprogs.dat
debugcl: clprogs.dat
debugsv: server/svprogs.dat

deldebug:
	rm clprogs.dat server/svprogs.dat

clprogs.dat:
	$(VCC) -I$(COMMON_DIR) clprogs.vc $@ -ddebug.txt $(DUMP_ASM)

server/svprogs.dat:
	$(VCC) -I$(COMMON_DIR) server/svprogs.vc $@ -dserver/debug.txt $(DUMP_ASM)

