commondir = $(datadir)/vavoom/basev/common

nobase_common_DATA = basepak.pk3

EXTRA_DIST = \
	basepak.ls.in \
	default.cfg \
	lockdefs.txt \
	startup.vs \
	terrain.txt \
	x11r6rgb.txt \
	sprites/tnt1a0.lmp \
	textures/-noflat-.png

VLUMPY = ../../utils/bin/vlumpy$(EXEEXT)

basepak.pk3: basepak.ls $(EXTRA_DIST) $(VLUMPY) \
		progs/engine.dat progs/linespec.dat
	$(VLUMPY) $<

MOSTLYCLEANFILES = basepak.pk3
