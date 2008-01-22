include_directories(../common)

add_executable(../bin/fixmd2
	../common/cmdlib.cpp
	../common/cmdlib.h
	../common/fmd2defs.h
	fixmd2.cpp
)
