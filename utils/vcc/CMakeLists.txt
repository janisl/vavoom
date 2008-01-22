include_directories(../common)

add_executable(../bin/vcc
	../common/array.h
	../common/cmdlib.cpp
	../common/cmdlib.h
	error.cpp
	expression.cpp
	expression.h
	lexer.cpp
	lexer.h
	modifiers.cpp
	name.cpp
	name.h
	names.h
	parse.cpp
	pcode.cpp
	statement.cpp
	statement.h
	stream.cpp
	stream.h
	types.cpp
	vcc.cpp
	vcc.h
)

#EXTRA_DIST = vcc.txt
