find_package(ZLIB REQUIRED)

include_directories(../common ${ZLIB_INCLUDE_DIR})

add_executable(../bin/vlumpy
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
target_link_libraries(../bin/vlumpy ${ZLIB_LIBRARY})
