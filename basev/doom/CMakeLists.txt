configure_file(basepak.ls.in ${CMAKE_CURRENT_BINARY_DIR}/basepak.ls)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/basepak.pk3
	COMMAND ${VLUMPY} ARGS basepak.ls
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/basepak.ls vlumpy
		graphics/autopage.lmp
		graphics/croshai1.lmp
		graphics/croshai2.lmp
		graphics/croshai3.lmp
		basepak.ls.in
		bigfont.pcx
		animated.lmp
		animdefs.txt
		dehinfo.txt
		lockdefs.txt
		palette.pcx
		skyboxes.txt
		sndinfo.txt
		sndseq.txt
	)
add_custom_target(doom_basepak ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/basepak.pk3)
