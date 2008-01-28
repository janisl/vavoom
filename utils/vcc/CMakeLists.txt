include_directories(../common)

add_executable(vcc
	../../source/array.h
	../common/cmdlib.cpp
	../common/cmdlib.h
	error.cpp
	expression.cpp
	expression.h
	lexer.cpp
	lexer.h
	modifiers.cpp
	../../source/name.cpp
	../../source/name.h
	../../source/names.h
	parse.cpp
	pcode.cpp
	statement.cpp
	statement.h
	../../source/stream.cpp
	../../source/stream.h
	types.cpp
	vcc.cpp
	vcc.h
)
set_target_properties(vcc PROPERTIES OUTPUT_NAME ../bin/vcc)
set_target_properties(vcc PROPERTIES COMPILE_FLAGS -DIN_VCC)

#EXTRA_DIST = vcc.txt
