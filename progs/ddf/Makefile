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

$(OUTDIR)/clprogs.dat: clprogs.vc ../doom/*.vc ../doom/client/*.vc $(COMMON_DIR)/*.vc
	$(VCC) -I$(COMMON_DIR) -I../doom/client clprogs.vc $@

$(OUTDIR)/svprogs.dat: *.vc ../doom/*.vc ../doom/server/*.vc $(COMMON_DIR)/*.vc
	$(VCC) -I$(COMMON_DIR) -I../doom/server svprogs.vc $@

#---------------------------------------
# Debugging
#---------------------------------------

#debug: clprogs.dat svprogs.dat
#debugcl: clprogs.dat
#debugsv: svprogs.dat

#deldebug:
#	rm clprogs.dat svprogs.dat

#clprogs.dat:
#	$(VCC) -I$(COMMON_DIR) clprogs.vc $@ -dcldebug.txt $(DUMP_ASM)

#svprogs.dat:
#	$(VCC) -I$(COMMON_DIR) svprogs.vc $@ -dsvdebug.txt $(DUMP_ASM)

