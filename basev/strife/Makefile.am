strifedir = $(datadir)/vavoom/basev/strife

nobase_strife_DATA = basepak.pk3

EXTRA_DIST = \
	palette.pcx \
	graphics/autopage.lmp \
	graphics/croshai1.lmp \
	graphics/croshai2.lmp \
	graphics/croshai3.lmp \
	graphics/fontb01.lmp \
	graphics/fontb02.lmp \
	graphics/fontb03.lmp \
	graphics/fontb04.lmp \
	graphics/fontb05.lmp \
	graphics/fontb06.lmp \
	graphics/fontb07.lmp \
	graphics/fontb08.lmp \
	graphics/fontb09.lmp \
	graphics/fontb10.lmp \
	graphics/fontb11.lmp \
	graphics/fontb12.lmp \
	graphics/fontb13.lmp \
	graphics/fontb14.lmp \
	graphics/fontb15.lmp \
	graphics/fontb16.lmp \
	graphics/fontb17.lmp \
	graphics/fontb18.lmp \
	graphics/fontb19.lmp \
	graphics/fontb20.lmp \
	graphics/fontb21.lmp \
	graphics/fontb22.lmp \
	graphics/fontb23.lmp \
	graphics/fontb24.lmp \
	graphics/fontb25.lmp \
	graphics/fontb26.lmp \
	graphics/fontb27.lmp \
	graphics/fontb28.lmp \
	graphics/fontb29.lmp \
	graphics/fontb30.lmp \
	graphics/fontb31.lmp \
	graphics/fontb32.lmp \
	graphics/fontb33.lmp \
	graphics/fontb34.lmp \
	graphics/fontb35.lmp \
	graphics/fontb36.lmp \
	graphics/fontb37.lmp \
	graphics/fontb38.lmp \
	graphics/fontb39.lmp \
	graphics/fontb40.lmp \
	graphics/fontb41.lmp \
	graphics/fontb42.lmp \
	graphics/fontb43.lmp \
	graphics/fontb44.lmp \
	graphics/fontb45.lmp \
	graphics/fontb46.lmp \
	graphics/fontb47.lmp \
	graphics/fontb48.lmp \
	graphics/fontb49.lmp \
	graphics/fontb50.lmp \
	graphics/fontb51.lmp \
	graphics/fontb52.lmp \
	graphics/fontb53.lmp \
	graphics/fontb54.lmp \
	graphics/fontb55.lmp \
	graphics/fontb56.lmp \
	graphics/fontb57.lmp \
	graphics/fontb58.lmp \
	basepak.ls.in \
	language.en \
	animdefs.txt \
	lockdefs.txt \
	mapinfo.txt \
	skyboxes.txt \
	sndinfo.txt \
	sndseq.txt \
	terrain.txt

VLUMPY = ../../utils/bin/vlumpy$(EXEEXT)
ACC = ../../utils/bin/acc$(EXEEXT)

basepak.pk3: basepak.ls $(VLUMPY) \
		progs/clprogs.dat progs/svprogs.dat strfhelp.o
	$(VLUMPY) $<

strfhelp.o: $(srcdir)/../../utils/editing/acs/*.acs $(ACC)
	$(ACC) $(srcdir)/../../utils/editing/acs/strfhelp.acs strfhelp.o

MOSTLYCLEANFILES = basepak.pk3 strfhelp.o
