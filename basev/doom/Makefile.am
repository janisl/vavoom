doomdir = $(datadir)/vavoom/basev/doom

nobase_doom_DATA = basepak.pk3

EXTRA_DIST = \
	graphics/autopage.lmp \
	graphics/croshai1.lmp \
	graphics/croshai2.lmp \
	graphics/croshai3.lmp \
	basepak.ls.in \
	bigfont.pcx \
	animated.lmp \
	animdefs.txt \
	dehinfo.txt \
	lockdefs.txt \
	palette.pcx \
	skyboxes.txt \
	sndinfo.txt \
	sndseq.txt

VLUMPY = ../../utils/bin/vlumpy$(EXEEXT)

basepak.pk3: basepak.ls $(EXTRA_DIST) $(VLUMPY)
	$(VLUMPY) $<

MOSTLYCLEANFILES = basepak.pk3
