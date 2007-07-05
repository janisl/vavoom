doom1dir = $(datadir)/vavoom/basev/doom1

nobase_doom1_DATA = basepak.pk3
nobase_dist_doom1_DATA = base.txt

EXTRA_DIST = \
	basepak.ls.in \
	mapinfo.txt \
	sndinfo.txt

VLUMPY = ../../utils/bin/vlumpy$(EXEEXT)

basepak.pk3: basepak.ls $(VLUMPY) \
		progs/shared.dat progs/clprogs.dat progs/svprogs.dat
	$(VLUMPY) $<
