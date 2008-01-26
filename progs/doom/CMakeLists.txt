set(OUTDIR ${CMAKE_BINARY_DIR}/basev/doom1/progs)
set(COMMON_PROGS_DIR ${CMAKE_BINARY_DIR}/basev/common/progs)

add_custom_command(OUTPUT ${OUTDIR}/shared.dat
	COMMAND mkdir ARGS -p ${OUTDIR}
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/doomdefs.vc ${OUTDIR}/shared.dat
	DEPENDS vcc *.vc ${COMMON_PROGS_DIR}/engine.dat)
add_custom_target(doom_shared ALL DEPENDS ${OUTDIR}/shared.dat)
add_dependencies(doom_shared vcc engine)

add_custom_command(OUTPUT ${OUTDIR}/svprogs.dat
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/svprogs.vc ${OUTDIR}/svprogs.dat
	DEPENDS vcc *.vc game/*.vc game1/*.vc game2/*.vc ${COMMON_PROGS_DIR}/linespec.dat ${OUTDIR}/shared.dat)
add_custom_target(doom_svprogs ALL DEPENDS ${OUTDIR}/svprogs.dat)
add_dependencies(doom_svprogs vcc linespec doom_shared)

add_custom_command(OUTPUT ${OUTDIR}/clprogs.dat
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/clprogs.vc ${OUTDIR}/clprogs.dat
	DEPENDS vcc *.vc cgame/*.vc ${COMMON_PROGS_DIR}/uibase.dat ${OUTDIR}/svprogs.dat)
add_custom_target(doom_clprogs ALL DEPENDS ${OUTDIR}/clprogs.dat)
add_dependencies(doom_clprogs vcc doom_svprogs uibase)
