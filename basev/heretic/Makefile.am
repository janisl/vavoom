hereticdir = $(datadir)/vavoom/basev/heretic

nobase_heretic_DATA = basepak.pk3

EXTRA_DIST = \
	graphics/croshai1.lmp \
	graphics/croshai2.lmp \
	graphics/croshai3.lmp \
	graphics/fonta30.lmp \
	graphics/fonta63.lmp \
	palette.pcx \
	basepak.ls.in \
	animdefs.txt \
	lockdefs.txt \
	mapinfo.txt \
	skyboxes.txt \
	sndinfo.txt \
	sndseq.txt \
	terrain.txt

VLUMPY = ../../utils/bin/vlumpy$(EXEEXT)

basepak.pk3: basepak.ls $(VLUMPY) \
		progs/clprogs.dat progs/svprogs.dat
	$(VLUMPY) $<

MOSTLYCLEANFILES = basepak.pk3
