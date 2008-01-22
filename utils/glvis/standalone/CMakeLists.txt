add_library(glvislib STATIC
	cmdlib.cpp
	cmdlib.h
	fmapdefs.h
	fwaddefs.h
	wadlib.cpp
	wadlib.h
	flow.cpp
	glvis.h
	glvisint.h
	level.cpp
	vector.h
)

add_executable(glvis
	glvis.cpp
)
add_dependencies(glvis glvislib)
target_link_libraries(glvis glvislib)

#EXTRA_DIST = glvis.txt
