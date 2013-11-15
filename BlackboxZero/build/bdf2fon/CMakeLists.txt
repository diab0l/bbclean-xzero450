
add_executable(bdf2fnt bdf2fnt.c)
set_target_properties(bdf2fnt PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")

add_executable(fnt2fon fnt2fon.c)
set_target_properties(fnt2fon PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")

message("dir ${bdf2fnt_BINARY}")

message("Debug variable dump:")
get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
   message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
#ALWAYS_32 = 1
