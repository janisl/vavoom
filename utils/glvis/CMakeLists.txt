include_directories(../common)

add_library(glvis STATIC
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

add_executable(../bin/glvis
	glvis.cpp
)
add_dependencies(../bin/glvis glvis)
target_link_libraries(../bin/glvis glvis)

#EXTRA_DIST = glvis.txt
