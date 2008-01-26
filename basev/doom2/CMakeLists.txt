configure_file(basepak.ls.in ${CMAKE_CURRENT_BINARY_DIR}/basepak.ls)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/basepak.pk3
	COMMAND ${VLUMPY} ARGS basepak.ls
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/basepak.ls ${VLUMPY}
		basepak.ls.in
		mapinfo.txt
		${CMAKE_CURRENT_BINARY_DIR}/progs/shared.dat
		${CMAKE_CURRENT_BINARY_DIR}/progs/clprogs.dat
		${CMAKE_CURRENT_BINARY_DIR}/progs/svprogs.dat
	)
add_custom_target(doom2_basepak ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/basepak.pk3)

#nobase_dist_doom2_DATA = base.txt
