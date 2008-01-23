include_directories(../common)

add_library(libglvis STATIC
	../common/cmdlib.cpp
	../common/cmdlib.h
	../common/fmapdefs.h
	../common/fwaddefs.h
	../common/wadlib.cpp
	../common/wadlib.h
	flow.cpp
	glvis.h
	glvisint.h
	level.cpp
	vector.h
)
set_target_properties(libglvis PROPERTIES OUTPUT_NAME glvis)

add_executable(../bin/glvis
	glvis.cpp
)
add_dependencies(../bin/glvis libglvis)
target_link_libraries(../bin/glvis libglvis)

#EXTRA_DIST = glvis.txt
