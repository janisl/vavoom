tntdir = $(datadir)/vavoom/basev/tnt

nobase_tnt_DATA = basepak.pk3
nobase_dist_tnt_DATA = base.txt

EXTRA_DIST = \
	basepak.ls.in \
	mapinfo.txt

VLUMPY = ../../utils/bin/vlumpy$(EXEEXT)

basepak.pk3: basepak.ls $(VLUMPY)
	$(VLUMPY) $<
