set(VCC ${CMAKE_BINARY_DIR}/../utils/bin/vcc)
set(OUTDIR ${CMAKE_BINARY_DIR}/../basev/common/progs)

add_custom_command(OUTPUT ${OUTDIR}/engine.dat
	COMMAND mkdir ARGS -p ${OUTDIR}
	COMMAND ${VCC} ARGS -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/engine/classes.vc ${OUTDIR}/engine.dat
	DEPENDS ${VCC} engine/*.vc)
add_custom_target(engine ALL DEPENDS ${OUTDIR}/engine.dat)

add_custom_command(OUTPUT ${OUTDIR}/linespec.dat
	COMMAND ${VCC} ARGS -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/linespec/classes.vc ${OUTDIR}/linespec.dat
	DEPENDS ${VCC} linespec/*.vc)
add_custom_target(linespec ALL DEPENDS ${OUTDIR}/linespec.dat)
add_dependencies(linespec engine)

add_custom_command(OUTPUT ${OUTDIR}/uibase.dat
	COMMAND ${VCC} ARGS -P${OUTDIR} ${CMAKE_CURRENT_SOURCE_DIR}/uibase/classes.vc ${OUTDIR}/uibase.dat
	DEPENDS ${VCC} uibase/*.vc)
add_custom_target(uibase ALL DEPENDS ${OUTDIR}/uibase.dat)
add_dependencies(uibase engine)
