plutoniadir = $(datadir)/vavoom/basev/plutonia

nobase_plutonia_DATA = basepak.pk3
nobase_dist_plutonia_DATA = base.txt

EXTRA_DIST = \
	basepak.ls.in \
	mapinfo.txt

VLUMPY = ../../utils/bin/vlumpy$(EXEEXT)

basepak.pk3: basepak.ls $(VLUMPY)
	$(VLUMPY) $<

MOSTLYCLEANFILES = basepak.pk3
