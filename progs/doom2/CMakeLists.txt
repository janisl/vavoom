set(OUTDIR ${CMAKE_BINARY_DIR}/basev/doom2/progs)
set(COMMON_PROGS_DIR ${CMAKE_BINARY_DIR}/basev/common/progs)

add_custom_command(OUTPUT ${OUTDIR}/shared.dat
	COMMAND mkdir ARGS -p ${OUTDIR}
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/shared.vc ${OUTDIR}/shared.dat
	DEPENDS vcc shared.vc ../doom/*.vc ${COMMON_PROGS_DIR}/engine.dat)
add_custom_target(doom2_shared ALL DEPENDS ${OUTDIR}/shared.dat)
add_dependencies(doom2_shared vcc engine)

add_custom_command(OUTPUT ${OUTDIR}/svprogs.dat
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/svprogs.vc ${OUTDIR}/svprogs.dat
	DEPENDS vcc svprogs.vc ../doom/*.vc ../doom/game/*.vc ../doom/game1/*.vc ../doom/game2/*.vc
	${COMMON_PROGS_DIR}/linespec.dat ${OUTDIR}/shared.dat)
add_custom_target(doom2_svprogs ALL DEPENDS ${OUTDIR}/svprogs.dat)
add_dependencies(doom2_svprogs vcc linespec doom2_shared)

add_custom_command(OUTPUT ${OUTDIR}/clprogs.dat
	COMMAND ${VCC} ARGS -P${COMMON_PROGS_DIR} -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/clprogs.vc ${OUTDIR}/clprogs.dat
	DEPENDS vcc clprogs.vc ../doom/*.vc ../doom/cgame/*.vc ${COMMON_PROGS_DIR}/uibase.dat ${OUTDIR}/svprogs.dat)
add_custom_target(doom2_clprogs ALL DEPENDS ${OUTDIR}/clprogs.dat)
add_dependencies(doom2_clprogs vcc doom2_svprogs uibase)
