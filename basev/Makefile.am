basevdir = $(datadir)/vavoom/basev
nobase_basev_DATA = \
	common/wad0.wad \
	doom/wad0.wad \
	doom1/wad0.wad \
	doom2/wad0.wad \
	heretic/wad0.wad \
	hexen/wad0.wad \
	plutonia/wad0.wad \
	strife/wad0.wad \
	tnt/wad0.wad
nobase_dist_basev_DATA = \
	games.txt \
	common/default.cfg \
	common/startup.vs \
	doom1/base.txt \
	doom2/base.txt \
	plutonia/base.txt \
	tnt/base.txt

EXTRA_DIST = \
	common/wad0.ls \
	doom/jldoom.wad \
	doom/palette.pcx \
	doom/wad0.ls \
	doom/scripts/animdefs.txt \
	doom/scripts/skyboxes.txt \
	doom/scripts/sndinfo.txt \
	doom/scripts/sndseq.txt \
	doom/scripts/terrains.txt \
	doom1/wad0.ls \
	doom1/scripts/fin1text.txt \
	doom1/scripts/fin2text.txt \
	doom1/scripts/fin3text.txt \
	doom1/scripts/fin4text.txt \
	doom1/scripts/mapinfo.txt \
	doom1/scripts/sndinfo.txt \
	doom2/wad0.ls \
	doom2/scripts/fin1text.txt \
	doom2/scripts/fin2text.txt \
	doom2/scripts/fin3text.txt \
	doom2/scripts/fin4text.txt \
	doom2/scripts/mapinfo.txt \
	doom2/scripts/sec1text.txt \
	doom2/scripts/sec2text.txt \
	heretic/hticdat.wad \
	heretic/palette.pcx \
	heretic/wad0.ls \
	heretic/scripts/animdefs.txt \
	heretic/scripts/fin1text.txt \
	heretic/scripts/fin2text.txt \
	heretic/scripts/fin3text.txt \
	heretic/scripts/fin4text.txt \
	heretic/scripts/fin5text.txt \
	heretic/scripts/mapinfo.txt \
	heretic/scripts/skyboxes.txt \
	heretic/scripts/sndinfo.txt \
	heretic/scripts/sndseq.txt \
	heretic/scripts/terrains.txt \
	hexen/hexendat.wad \
	hexen/palette.pcx \
	hexen/wad0.ls \
	hexen/scripts/animdefs.txt \
	hexen/scripts/skyboxes.txt \
	hexen/scripts/sndinfo.txt \
	hexen/scripts/terrains.txt \
	plutonia/wad0.ls \
	plutonia/scripts/fin1text.txt \
	plutonia/scripts/fin2text.txt \
	plutonia/scripts/fin3text.txt \
	plutonia/scripts/fin4text.txt \
	plutonia/scripts/mapinfo.txt \
	plutonia/scripts/sec1text.txt \
	plutonia/scripts/sec2text.txt \
	strife/palette.pcx \
	strife/strifdat.wad \
	strife/wad0.ls \
	strife/scripts/animdefs.txt \
	strife/scripts/mapinfo.txt \
	strife/scripts/skyboxes.txt \
	strife/scripts/sndinfo.txt \
	strife/scripts/sndseq.txt \
	strife/scripts/terrains.txt \
	tnt/wad0.ls \
	tnt/scripts/fin1text.txt \
	tnt/scripts/fin2text.txt \
	tnt/scripts/fin3text.txt \
	tnt/scripts/fin4text.txt \
	tnt/scripts/mapinfo.txt \
	tnt/scripts/sec1text.txt \
	tnt/scripts/sec2text.txt

common/wad0.wad: $(srcdir)/common/wad0.ls ../utils/bin/vlumpy$(EXEEXT) \
		common/progs/common.dat common/progs/clcommon.dat common/progs/svcommon.dat
	../utils/bin/vlumpy$(EXEEXT) -D$@ $<

doom/wad0.wad: $(srcdir)/doom/wad0.ls ../utils/bin/vlumpy$(EXEEXT)
	../utils/bin/vlumpy$(EXEEXT) -D$@ $<

doom1/wad0.wad: $(srcdir)/doom1/wad0.ls ../utils/bin/vlumpy$(EXEEXT) \
		doom1/progs/shared.dat doom1/progs/clprogs.dat doom1/progs/svprogs.dat
	../utils/bin/vlumpy$(EXEEXT) -D$@ $<

doom2/wad0.wad: $(srcdir)/doom2/wad0.ls ../utils/bin/vlumpy$(EXEEXT) \
		doom2/progs/shared.dat doom2/progs/clprogs.dat doom2/progs/svprogs.dat
	../utils/bin/vlumpy$(EXEEXT) -D$@ $<

heretic/wad0.wad: $(srcdir)/heretic/wad0.ls ../utils/bin/vlumpy$(EXEEXT) \
		heretic/progs/shared.dat heretic/progs/clprogs.dat heretic/progs/svprogs.dat
	../utils/bin/vlumpy$(EXEEXT) -D$@ $<

hexen/wad0.wad: $(srcdir)/hexen/wad0.ls ../utils/bin/vlumpy$(EXEEXT) \
		hexen/progs/shared.dat hexen/progs/clprogs.dat hexen/progs/svprogs.dat
	../utils/bin/vlumpy$(EXEEXT) -D$@ $<

plutonia/wad0.wad: $(srcdir)/plutonia/wad0.ls ../utils/bin/vlumpy$(EXEEXT)
	../utils/bin/vlumpy$(EXEEXT) -D$@ $<

strife/wad0.wad: $(srcdir)/strife/wad0.ls ../utils/bin/vlumpy$(EXEEXT) \
		strife/progs/shared.dat strife/progs/clprogs.dat strife/progs/svprogs.dat strife/strfhelp.o
	../utils/bin/vlumpy$(EXEEXT) -D$@ $<

strife/strfhelp.o: $(srcdir)/../utils/editing/acs/*.acs ../utils/bin/acc$(EXEEXT)
	../utils/bin/acc$(EXEEXT) $(srcdir)/../utils/editing/acs/strfhelp.acs strife/strfhelp.o

tnt/wad0.wad: $(srcdir)/tnt/wad0.ls ../utils/bin/vlumpy$(EXEEXT)
	../utils/bin/vlumpy$(EXEEXT) -D$@ $<
