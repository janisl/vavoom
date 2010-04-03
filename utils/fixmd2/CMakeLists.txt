include_directories(../common)

add_executable(fixmd2
	../common/cmdlib.cpp
	../common/cmdlib.h
	../common/fmd2defs.h
	fixmd2.cpp
)
set_target_properties(fixmd2 PROPERTIES OUTPUT_NAME ../bin/fixmd2)
target_link_libraries(fixmd2 core)
add_dependencies(fixmd2 core)
