doomdir = $(datadir)/vavoom/basev/doom

nobase_doom_DATA = basepak.pk3

EXTRA_DIST = \
	graphics/autopage.lmp \
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
	graphics/fontb59.lmp \
	graphics/fontb60.lmp \
	graphics/fontb61.lmp \
	graphics/fontb62.lmp \
	graphics/fontb63.lmp \
	graphics/fontb64.lmp \
	graphics/fontb65.lmp \
	graphics/fontb66.lmp \
	graphics/fontb67.lmp \
	graphics/fontb68.lmp \
	graphics/fontb69.lmp \
	graphics/fontb70.lmp \
	graphics/fontb71.lmp \
	graphics/fontb72.lmp \
	graphics/fontb73.lmp \
	graphics/fontb74.lmp \
	graphics/fontb75.lmp \
	graphics/fontb76.lmp \
	graphics/fontb77.lmp \
	graphics/fontb78.lmp \
	graphics/fontb79.lmp \
	graphics/fontb80.lmp \
	graphics/fontb81.lmp \
	graphics/fontb82.lmp \
	graphics/fontb83.lmp \
	graphics/fontb84.lmp \
	graphics/fontb85.lmp \
	graphics/fontb86.lmp \
	graphics/fontb87.lmp \
	graphics/fontb88.lmp \
	graphics/fontb89.lmp \
	graphics/fontb90.lmp \
	graphics/fontb91.lmp \
	graphics/fontb92.lmp \
	graphics/fontb93.lmp \
	graphics/fontb94.lmp \
	graphics/fontb95.lmp \
	graphics/croshai1.lmp \
	graphics/croshai2.lmp \
	graphics/croshai3.lmp \
	palette.pcx \
	basepak.ls.in \
	language.en \
	animated.lmp \
	animdefs.txt \
	dehinfo.txt \
	skyboxes.txt \
	sndinfo.txt \
	sndseq.txt

VLUMPY = ../../utils/bin/vlumpy$(EXEEXT)

basepak.pk3: basepak.ls $(EXTRA_DIST) $(VLUMPY)
	$(VLUMPY) $<

MOSTLYCLEANFILES = basepak.pk3
