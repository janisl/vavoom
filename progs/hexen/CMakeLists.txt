set(OUTDIR ${CMAKE_BINARY_DIR}/basev/hexen/progs)
set(COMMON_PROGS_DIR ${CMAKE_BINARY_DIR}/basev/common/progs)

add_custom_command(OUTPUT ${OUTDIR}/shared.dat
	COMMAND mkdir ARGS -p ${OUTDIR}
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/hexndefs.vc ${OUTDIR}/shared.dat
	DEPENDS vcc *.vc)
add_custom_target(hexen_shared ALL DEPENDS ${OUTDIR}/shared.dat)
add_dependencies(hexen_shared vcc engine)

add_custom_command(OUTPUT ${OUTDIR}/svprogs.dat
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/svprogs.vc ${OUTDIR}/svprogs.dat
	DEPENDS vcc *.vc game/*.vc)
add_custom_target(hexen_svprogs ALL DEPENDS ${OUTDIR}/svprogs.dat)
add_dependencies(hexen_svprogs vcc linespec hexen_shared)

add_custom_command(OUTPUT ${OUTDIR}/clprogs.dat
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/clprogs.vc ${OUTDIR}/clprogs.dat
	DEPENDS vcc *.vc cgame/*.vc)
add_custom_target(hexen_clprogs ALL DEPENDS ${OUTDIR}/clprogs.dat)
add_dependencies(hexen_clprogs vcc hexen_svprogs uibase)
