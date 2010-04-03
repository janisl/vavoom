include_directories(../common ${ZLIB_INCLUDE_DIR})

add_executable(vlumpy
	../common/cmdlib.cpp
	../common/cmdlib.h
	../common/ftexdefs.h
	../common/fwaddefs.h
	../common/imglib.cpp
	../common/imglib.h
	../common/scrlib.cpp
	../common/scrlib.h
	../common/wadlib.cpp
	../common/wadlib.h
	ioapi.cpp
	ioapi.h
	vlumpy.cpp
	zip.cpp
	zip.h
)
target_link_libraries(vlumpy core ${ZLIB_LIBRARY})
set_target_properties(vlumpy PROPERTIES OUTPUT_NAME ../bin/vlumpy)
add_dependencies(vlumpy core)
