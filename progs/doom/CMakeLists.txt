set(OUTDIR ${CMAKE_BINARY_DIR}/basev/doom1/progs)
set(COMMON_PROGS_DIR ${CMAKE_BINARY_DIR}/basev/common/progs)

add_custom_command(OUTPUT ${OUTDIR}/svprogs.dat
	COMMAND mkdir ARGS -p ${OUTDIR}
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/game/classes.vc ${OUTDIR}/svprogs.dat
	DEPENDS vcc game/*.vc ${COMMON_PROGS_DIR}/linespec.dat)
add_custom_target(doom_svprogs ALL DEPENDS ${OUTDIR}/svprogs.dat)
add_dependencies(doom_svprogs vcc linespec)

add_custom_command(OUTPUT ${OUTDIR}/clprogs.dat
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/cgame/classes.vc ${OUTDIR}/clprogs.dat
	DEPENDS vcc cgame/*.vc ${COMMON_PROGS_DIR}/uibase.dat ${OUTDIR}/svprogs.dat)
add_custom_target(doom_clprogs ALL DEPENDS ${OUTDIR}/clprogs.dat)
add_dependencies(doom_clprogs vcc doom_svprogs uibase)
