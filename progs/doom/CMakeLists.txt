set(VCC ${CMAKE_BINARY_DIR}/../utils/bin/vcc)
set(OUTDIR ${CMAKE_BINARY_DIR}/../basev/doom1/progs)
set(COMMON_PROGS_DIR ${CMAKE_BINARY_DIR}/../basev/common/progs)

add_custom_command(OUTPUT ${OUTDIR}/shared.dat
	COMMAND mkdir ARGS -p ${OUTDIR}
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/doomdefs.vc ${OUTDIR}/shared.dat
	DEPENDS ${VCC} *.vc)
add_custom_target(doom_shared ALL DEPENDS ${OUTDIR}/shared.dat)
add_dependencies(doom_shared engine)

add_custom_command(OUTPUT ${OUTDIR}/svprogs.dat
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/svprogs.vc ${OUTDIR}/svprogs.dat
	DEPENDS ${VCC} *.vc game/*.vc game1/*.vc game2/*.vc)
add_custom_target(doom_svprogs ALL DEPENDS ${OUTDIR}/svprogs.dat)
add_dependencies(doom_svprogs linespec doom_shared)

add_custom_command(OUTPUT ${OUTDIR}/clprogs.dat
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/clprogs.vc ${OUTDIR}/clprogs.dat
	DEPENDS ${VCC} *.vc cgame/*.vc)
add_custom_target(doom_clprogs ALL DEPENDS ${OUTDIR}/clprogs.dat)
add_dependencies(doom_clprogs doom_svprogs uibase)
