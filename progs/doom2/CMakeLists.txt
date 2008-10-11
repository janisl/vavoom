set(OUTDIR1 ${CMAKE_BINARY_DIR}/basev/doom1/progs)
set(OUTDIR ${CMAKE_BINARY_DIR}/basev/doom2/progs)
set(COMMON_PROGS_DIR ${CMAKE_BINARY_DIR}/basev/common/progs)

add_custom_command(OUTPUT ${OUTDIR}/cgame.dat
	COMMAND mkdir ARGS -p ${OUTDIR}
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR1} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/cgame.vc ${OUTDIR}/cgame.dat -I${CMAKE_CURRENT_SOURCE_DIR}/../doom/cgame
	DEPENDS vcc cgame.vc ../doom/cgame/*.vc ${COMMON_PROGS_DIR}/uibase.dat ${OUTDIR1}/game.dat)
add_custom_target(doom2_cgame ALL DEPENDS ${OUTDIR}/cgame.dat)
add_dependencies(doom2_cgame vcc doom_game uibase)
