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
	doom/language.en \
	doom/animdefs.txt \
	doom/skyboxes.txt \
	doom/sndinfo.txt \
	doom/sndseq.txt \
	doom/terrains.txt \
	doom1/basepak.ls.in \
	doom1/mapinfo.txt \
	doom1/sndinfo.txt \
	doom2/basepak.ls.in \
	doom2/mapinfo.txt \
	heretic/graphics/*.lmp \
	heretic/palette.pcx \
	heretic/basepak.ls.in \
	heretic/language.en \
	heretic/animdefs.txt \
	heretic/mapinfo.txt \
	heretic/skyboxes.txt \
	heretic/sndinfo.txt \
	heretic/sndseq.txt \
	heretic/terrains.txt \
	hexen/graphics/*.lmp \
	hexen/palette.pcx \
	hexen/basepak.ls.in \
	hexen/language.en \
	hexen/animdefs.txt \
	hexen/mapinfo.txt \
	hexen/skyboxes.txt \
	hexen/sndinfo.txt \
	hexen/terrains.txt \
	plutonia/basepak.ls.in \
	plutonia/mapinfo.txt \
	strife/palette.pcx \
	strife/graphics/*.lmp \
	strife/basepak.ls.in \
	strife/language.en \
	strife/animdefs.txt \
	strife/mapinfo.txt \
	strife/skyboxes.txt \
	strife/sndinfo.txt \
	strife/sndseq.txt \
	strife/terrains.txt \
	tnt/basepak.ls.in \
	tnt/scripts/mapinfo.txt

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
