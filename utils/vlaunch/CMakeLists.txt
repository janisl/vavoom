if(ENABLE_LAUNCHER)

	find_package(wxWidgets REQUIRED)
	add_definitions(${wxWidgets_DEFINITIONS})
	include_directories(${wxWidgets_INCLUDE_DIRS})

	add_executable(vlaunch
		vlaunch.cpp
	)
	target_link_libraries(vlaunch ${wxWidgets_LIBRARIES})
	set_target_properties(vlaunch PROPERTIES OUTPUT_NAME ../bin/vlaunch)

#EXTRA_DIST = \
#	vavoom.xpm \
#	vlaunch.ico \
#	vlaunch.rc \
#	vlaunch.sln \
#	vlaunch.vcproj \
#	vlaunch.xpm

endif(ENABLE_LAUNCHER)
