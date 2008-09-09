set(OUTDIR ${CMAKE_BINARY_DIR}/basev/heretic/progs)
set(COMMON_PROGS_DIR ${CMAKE_BINARY_DIR}/basev/common/progs)

add_custom_command(OUTPUT ${OUTDIR}/game.dat
	COMMAND mkdir ARGS -p ${OUTDIR}
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/game/classes.vc ${OUTDIR}/game.dat
	DEPENDS vcc game/*.vc ${COMMON_PROGS_DIR}/linespec.dat)
add_custom_target(heretic_game ALL DEPENDS ${OUTDIR}/game.dat)
add_dependencies(heretic_game vcc linespec)

add_custom_command(OUTPUT ${OUTDIR}/cgame.dat
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/cgame/classes.vc ${OUTDIR}/cgame.dat
	DEPENDS vcc cgame/*.vc ${COMMON_PROGS_DIR}/uibase.dat ${OUTDIR}/game.dat)
add_custom_target(heretic_cgame ALL DEPENDS ${OUTDIR}/cgame.dat)
add_dependencies(heretic_cgame vcc heretic_game uibase)
