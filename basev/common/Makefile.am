commondir = $(datadir)/vavoom/basev/common

nobase_common_DATA = basepak.pk3

EXTRA_DIST = \
	basepak.ls.in \
	default.cfg \
	x11r6rgb.txt \
	startup.vs \
	terrain.txt \
	textures/-noflat-.png

VLUMPY = ../../utils/bin/vlumpy$(EXEEXT)

basepak.pk3: basepak.ls $(VLUMPY) \
		progs/engine.dat progs/linespec.dat
	$(VLUMPY) $<
