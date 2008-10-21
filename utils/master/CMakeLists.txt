if(ENABLE_MASTER)
	include_directories(../common)

	add_executable(vavoom-master
		../common/array.h
		../common/cmdlib.cpp
		../common/cmdlib.h
		main.cpp
		net_udp.cpp
	)
	set_target_properties(vavoom-master PROPERTIES OUTPUT_NAME ../bin/vavoom-master)
endif(ENABLE_MASTER)
