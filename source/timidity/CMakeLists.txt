add_library(timidity STATIC
	common.cpp
	common.h
	config.h
	controls.cpp
	controls.h
	filter.cpp
	filter.h
	instrum.cpp
	instrum.h
	mix.cpp
	mix.h
	output.cpp
	output.h
	playmidi.cpp
	playmidi.h
	readmidi.cpp
	readmidi.h
	resample.cpp
	resample.h
	tables.cpp
	tables.h
	timidity.cpp
	timidity.h
)

# EXTRA_DIST = \
# 	FAQ \
# 	README
