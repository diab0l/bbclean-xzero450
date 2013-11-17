project(bdf2fon)
add_executable(bdf2fnt bdf2fnt.c)
set_target_properties(bdf2fnt PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
set_target_properties(bdf2fnt PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
)
add_executable(fnt2fon fnt2fon.c)
set_target_properties(fnt2fon PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
set_target_properties(fnt2fon PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
)

message("bdf2fon in ${CMAKE_BINARY_DIR}")

#ALWAYS_32 = 1
