doom2dir = $(datadir)/vavoom/basev/doom2

nobase_doom2_DATA = basepak.pk3
nobase_dist_doom2_DATA = base.txt

EXTRA_DIST = \
	basepak.ls.in \
	mapinfo.txt

VLUMPY = ../../utils/bin/vlumpy$(EXEEXT)

basepak.pk3: basepak.ls $(VLUMPY) \
		progs/shared.dat progs/clprogs.dat progs/svprogs.dat
	$(VLUMPY) $<

MOSTLYCLEANFILES = basepak.pk3
