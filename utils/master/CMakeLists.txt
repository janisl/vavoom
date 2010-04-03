if(ENABLE_MASTER)
	include_directories(../common)

	add_executable(vavoom-master
		main.cpp
	)
	set_target_properties(vavoom-master PROPERTIES OUTPUT_NAME ../bin/vavoom-master)
	target_link_libraries(vavoom-master core ${NET_LIBRARIES})
	add_dependencies(vavoom-master core)
endif(ENABLE_MASTER)
