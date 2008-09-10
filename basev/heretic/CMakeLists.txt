configure_file(basepak.ls.in ${CMAKE_CURRENT_BINARY_DIR}/basepak.ls)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/basepak.pk3
	COMMAND ${VLUMPY} ARGS basepak.ls
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/basepak.ls vlumpy
		graphics/croshai1.lmp
		graphics/croshai2.lmp
		graphics/croshai3.lmp
		graphics/fonta30.lmp
		graphics/fonta63.lmp
		palette.pcx
		basepak.ls.in
		animdefs.txt
		lockdefs.txt
		mapinfo.txt
		skyboxes.txt
		sndinfo.txt
		sndseq.txt
		sprite_rename.txt
		terrain.txt
		${CMAKE_SOURCE_DIR}/progs/heretic/cgame/*.vc
		${CMAKE_SOURCE_DIR}/progs/heretic/game/*.vc
	)
add_custom_target(heretic_basepak ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/basepak.pk3)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/basepak.pk3
	DESTINATION ${DATADIR}/basev/heretic)
