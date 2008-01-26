set(OUTDIR ${CMAKE_BINARY_DIR}/basev/heretic/progs)
set(COMMON_PROGS_DIR ${CMAKE_BINARY_DIR}/basev/common/progs)

add_custom_command(OUTPUT ${OUTDIR}/svprogs.dat
	COMMAND mkdir ARGS -p ${OUTDIR}
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/svprogs.vc ${OUTDIR}/svprogs.dat
	DEPENDS vcc *.vc game/*.vc ${COMMON_PROGS_DIR}/linespec.dat)
add_custom_target(heretic_svprogs ALL DEPENDS ${OUTDIR}/svprogs.dat)
add_dependencies(heretic_svprogs vcc linespec)

add_custom_command(OUTPUT ${OUTDIR}/clprogs.dat
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/clprogs.vc ${OUTDIR}/clprogs.dat
	DEPENDS vcc *.vc cgame/*.vc ${COMMON_PROGS_DIR}/uibase.dat ${OUTDIR}/svprogs.dat)
add_custom_target(heretic_clprogs ALL DEPENDS ${OUTDIR}/clprogs.dat)
add_dependencies(heretic_clprogs vcc heretic_svprogs uibase)
