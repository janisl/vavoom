configure_file(basepak.ls.in ${CMAKE_CURRENT_BINARY_DIR}/basepak.ls)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/strfhelp.o
	COMMAND ${ACC} ARGS ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/editing/acs/strfhelp.acs strfhelp.o
	DEPENDS ../../utils/editing/acs/*.acs acc
	)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/basepak.pk3
	COMMAND ${VLUMPY} ARGS basepak.ls
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/basepak.ls vlumpy
		palette.pcx
		graphics/autopage.lmp
		graphics/croshai1.lmp
		graphics/croshai2.lmp
		graphics/croshai3.lmp
		basepak.ls.in
		bigfont.pcx
		animdefs.txt
		lockdefs.txt
		mapinfo.txt
		skyboxes.txt
		sndinfo.txt
		sndseq.txt
		terrain.txt
		${CMAKE_CURRENT_BINARY_DIR}/strfhelp.o
		${CMAKE_CURRENT_BINARY_DIR}/progs/clprogs.dat
		${CMAKE_CURRENT_BINARY_DIR}/progs/svprogs.dat
	)
add_custom_target(strife_basepak ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/strfhelp.o
	${CMAKE_CURRENT_BINARY_DIR}/basepak.pk3)
