commondir = $(datadir)/vavoom/basev/common

nobase_common_DATA = basepak.pk3

EXTRA_DIST = \
	basepak.ls.in \
	default.cfg \
	language.en \
	lockdefs.txt \
	startup.vs \
	terrain.txt \
	textcolo.txt \
	x11r6rgb.txt \
	graphics/mc_arrow.png \
	sounds/dsempty.lmp \
	sprites/tnt1a0.lmp \
	textures/-noflat-.png

VLUMPY = ../../utils/bin/vlumpy$(EXEEXT)

basepak.pk3: basepak.ls $(EXTRA_DIST) $(VLUMPY) \
		progs/engine.dat progs/linespec.dat progs/uibase.dat
	$(VLUMPY) $<

MOSTLYCLEANFILES = basepak.pk3
