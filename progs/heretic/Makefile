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

$(OUTDIR)/clprogs.dat: *.vc client/*.vc cgame/*.vc $(COMMON_DIR)/*.vc $(COMMON_DIR)/shared/*.vc $(COMMON_DIR)/client/*.vc
	$(VCC) -I$(COMMON_DIR) client/clprogs.vc $@

$(OUTDIR)/svprogs.dat: *.vc server/*.vc game/*.vc $(COMMON_DIR)/*.vc $(COMMON_DIR)/shared/*.vc $(COMMON_DIR)/server/*.vc
	$(VCC) -I$(COMMON_DIR) server/svprogs.vc $@

#---------------------------------------
# Debugging
#---------------------------------------

debug: client/clprogs.dat server/svprogs.dat
debugcl: client/clprogs.dat
debugsv: server/svprogs.dat

deldebug:
	rm client/clprogs.dat server/svprogs.dat

client/clprogs.dat:
	$(VCC) -I$(COMMON_DIR) client/clprogs.vc $@ -dclient/debug.txt $(DUMP_ASM)

server/svprogs.dat:
	$(VCC) -I$(COMMON_DIR) server/svprogs.vc $@ -dserver/debug.txt $(DUMP_ASM)

