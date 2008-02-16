configure_file(basepak.ls.in ${CMAKE_CURRENT_BINARY_DIR}/basepak.ls)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/basepak.pk3
	COMMAND ${VLUMPY} ARGS basepak.ls
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/basepak.ls vlumpy
		basepak.ls.in
		confont.pcx
		default.cfg
		language.en
		lockdefs.txt
		startup.vs
		terrain.txt
		textcolo.txt
		vfxdefs.txt
		x11r6rgb.txt
		fxdefs/common/effects.txt
		fxdefs/doom/monsters.txt
		fxdefs/doom/pickups.txt
		fxdefs/doom/weapons.txt
		fxdefs/heretic/effects.txt
		fxdefs/heretic/monsters.txt
		fxdefs/heretic/pickups.txt
		fxdefs/heretic/weapons.txt
		graphics/mc_arrow.png
		sounds/dsempty.lmp
		sounds/dsquake.flac
		sounds/spark1.flac
		sounds/spark2.flac
		sounds/spark3.flac
		sprites/tnt1a0.lmp
		sprites/unkna0.png
		textures/-noflat-.png
		${CMAKE_CURRENT_BINARY_DIR}/progs/engine.dat
		${CMAKE_CURRENT_BINARY_DIR}/progs/linespec.dat
		${CMAKE_CURRENT_BINARY_DIR}/progs/uibase.dat
	)
add_custom_target(common_basepak ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/basepak.pk3)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/basepak.pk3
	DESTINATION ${DATADIR}/basev/common)
