set(VCC ${CMAKE_BINARY_DIR}/../utils/bin/vcc)
set(OUTDIR ${CMAKE_BINARY_DIR}/../basev/strife/progs)
set(COMMON_PROGS_DIR ${CMAKE_BINARY_DIR}/../basev/common/progs)

add_custom_command(OUTPUT ${OUTDIR}/svprogs.dat
	COMMAND mkdir ARGS -p ${OUTDIR}
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/svprogs.vc ${OUTDIR}/svprogs.dat
	DEPENDS ${VCC} *.vc game/*.vc)
add_custom_target(strife_svprogs ALL DEPENDS ${OUTDIR}/svprogs.dat)
add_dependencies(strife_svprogs linespec)

add_custom_command(OUTPUT ${OUTDIR}/clprogs.dat
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/clprogs.vc ${OUTDIR}/clprogs.dat
	DEPENDS ${VCC} *.vc cgame/*.vc)
add_custom_target(strife_clprogs ALL DEPENDS ${OUTDIR}/clprogs.dat)
add_dependencies(strife_clprogs strife_svprogs uibase)
