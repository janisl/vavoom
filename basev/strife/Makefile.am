strifedir = $(datadir)/vavoom/basev/strife

nobase_strife_DATA = basepak.pk3

EXTRA_DIST = \
	palette.pcx \
	graphics/autopage.lmp \
	graphics/croshai1.lmp \
	graphics/croshai2.lmp \
	graphics/croshai3.lmp \
	basepak.ls.in \
	bigfont.pcx \
	animdefs.txt \
	lockdefs.txt \
	mapinfo.txt \
	skyboxes.txt \
	sndinfo.txt \
	sndseq.txt \
	terrain.txt

VLUMPY = ../../utils/bin/vlumpy$(EXEEXT)
ACC = ../../utils/bin/acc$(EXEEXT)

basepak.pk3: basepak.ls $(EXTRA_DIST) $(VLUMPY) \
		progs/clprogs.dat progs/svprogs.dat strfhelp.o
	$(VLUMPY) $<

strfhelp.o: $(srcdir)/../../utils/editing/acs/*.acs $(ACC)
	$(ACC) $(srcdir)/../../utils/editing/acs/strfhelp.acs strfhelp.o

MOSTLYCLEANFILES = basepak.pk3 strfhelp.o
