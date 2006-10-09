basevdir = $(datadir)/vavoom/basev
nobase_basev_DATA = \
	common/basepak.pk3 \
	doom/basepak.pk3 \
	doom1/basepak.pk3 \
	doom2/basepak.pk3 \
	heretic/basepak.pk3 \
	hexen/basepak.pk3 \
	plutonia/basepak.pk3 \
	strife/basepak.pk3 \
	tnt/basepak.pk3
nobase_dist_basev_DATA = \
	games.txt \
	doom1/base.txt \
	doom2/base.txt \
	plutonia/base.txt \
	tnt/base.txt

EXTRA_DIST = \
	common/basepak.ls.in \
	common/default.cfg \
	common/startup.vs \
	doom/graphics/*.lmp \
	doom/palette.pcx \
	doom/basepak.ls.in \
	doom/scripts/animdefs.txt \
	doom/scripts/skyboxes.txt \
	doom/scripts/sndinfo.txt \
	doom/scripts/sndseq.txt \
	doom/scripts/terrains.txt \
	doom1/basepak.ls.in \
	doom1/scripts/fin1text.txt \
	doom1/scripts/fin2text.txt \
	doom1/scripts/fin3text.txt \
	doom1/scripts/fin4text.txt \
	doom1/scripts/mapinfo.txt \
	doom1/scripts/sndinfo.txt \
	doom2/basepak.ls.in \
	doom2/scripts/fin1text.txt \
	doom2/scripts/fin2text.txt \
	doom2/scripts/fin3text.txt \
	doom2/scripts/fin4text.txt \
	doom2/scripts/mapinfo.txt \
	doom2/scripts/sec1text.txt \
	doom2/scripts/sec2text.txt \
	heretic/graphics/*.lmp \
	heretic/palette.pcx \
	heretic/basepak.ls.in \
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
	hexen/graphics/*.lmp \
	hexen/palette.pcx \
	hexen/basepak.ls.in \
	hexen/scripts/animdefs.txt \
	hexen/scripts/mapinfo.txt \
	hexen/scripts/skyboxes.txt \
	hexen/scripts/sndinfo.txt \
	hexen/scripts/terrains.txt \
	plutonia/basepak.ls.in \
	plutonia/scripts/fin1text.txt \
	plutonia/scripts/fin2text.txt \
	plutonia/scripts/fin3text.txt \
	plutonia/scripts/fin4text.txt \
	plutonia/scripts/mapinfo.txt \
	plutonia/scripts/sec1text.txt \
	plutonia/scripts/sec2text.txt \
	strife/palette.pcx \
	strife/graphics/*.lmp \
	strife/basepak.ls.in \
	strife/scripts/animdefs.txt \
	strife/scripts/mapinfo.txt \
	strife/scripts/skyboxes.txt \
	strife/scripts/sndinfo.txt \
	strife/scripts/sndseq.txt \
	strife/scripts/terrains.txt \
	tnt/basepak.ls.in \
	tnt/scripts/fin1text.txt \
	tnt/scripts/fin2text.txt \
	tnt/scripts/fin3text.txt \
	tnt/scripts/fin4text.txt \
	tnt/scripts/mapinfo.txt \
	tnt/scripts/sec1text.txt \
	tnt/scripts/sec2text.txt

VLUMPY = ../utils/bin/vlumpy$(EXEEXT)
ACC = ../utils/bin/acc$(EXEEXT)

common/basepak.pk3: common/basepak.ls $(VLUMPY) \
		common/progs/engine.dat common/progs/linespec.dat
	$(VLUMPY) $<

doom/basepak.pk3: doom/basepak.ls $(VLUMPY)
	@mkdir -p doom
	$(VLUMPY) $<

doom1/basepak.pk3: doom1/basepak.ls $(VLUMPY) \
		doom1/progs/shared.dat doom1/progs/clprogs.dat doom1/progs/svprogs.dat
	$(VLUMPY) $<

doom2/basepak.pk3: doom2/basepak.ls $(VLUMPY) \
		doom2/progs/shared.dat doom2/progs/clprogs.dat doom2/progs/svprogs.dat
	$(VLUMPY) $<

heretic/basepak.pk3: heretic/basepak.ls $(VLUMPY) \
		heretic/progs/shared.dat heretic/progs/clprogs.dat heretic/progs/svprogs.dat
	$(VLUMPY) $<

hexen/basepak.pk3: hexen/basepak.ls $(VLUMPY) \
		hexen/progs/shared.dat hexen/progs/clprogs.dat hexen/progs/svprogs.dat
	$(VLUMPY) $<

plutonia/basepak.pk3: plutonia/basepak.ls $(VLUMPY)
	@mkdir -p plutonia
	$(VLUMPY) $<

strife/basepak.pk3: strife/basepak.ls $(VLUMPY) \
		strife/progs/shared.dat strife/progs/clprogs.dat strife/progs/svprogs.dat strife/strfhelp.o
	$(VLUMPY) $<

strife/strfhelp.o: $(srcdir)/../utils/editing/acs/*.acs $(ACC)
	$(ACC) $(srcdir)/../utils/editing/acs/strfhelp.acs strife/strfhelp.o

tnt/basepak.pk3: tnt/basepak.ls $(VLUMPY)
	@mkdir -p tnt
	$(VLUMPY) $<
