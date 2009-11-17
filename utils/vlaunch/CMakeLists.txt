if(ENABLE_LAUNCHER)

	find_package(wxWidgets REQUIRED)
	include(${wxWidgets_USE_FILE})

	set(VLAUNCH_SRC vlaunch.cpp)

	if(WIN32)
		#  CMake doesn't support automatic building of resource files so
		# we must add a custom command and add object files to the list
		# of source files
		set(RC_SRC ${CMAKE_CURRENT_SOURCE_DIR}/vlaunch.rc)
		set(RC_OBJ ${CMAKE_CURRENT_BINARY_DIR}/vlaunch.o)
		add_custom_command(OUTPUT ${RC_OBJ}
			DEPENDS ${RC_SRC}
			COMMAND windres --include-dir=${CMAKE_CURRENT_SOURCE_DIR} -o ${RC_OBJ} -i ${RC_SRC}
		)
		set(VLAUNCH_SRC ${VLAUNCH_SRC} ${RC_OBJ})
	endif(WIN32)

	add_executable(vlaunch
		${VLAUNCH_SRC}
	)
	target_link_libraries(vlaunch ${wxWidgets_LIBRARIES})

	install(TARGETS vlaunch DESTINATION ${BINDIR})

#EXTRA_DIST = \
#	vavoom.xpm \
#	vlaunch.ico \
#	vlaunch.rc \
#	vlaunch.sln \
#	vlaunch.vcproj \
#	vlaunch.xpm

endif(ENABLE_LAUNCHER)
