project(bdf2fon)
add_executable(bdf2fnt bdf2fnt.c)
set_target_properties(bdf2fnt PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
set_target_properties(bdf2fnt PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
)
set_property(TARGET bdf2fnt PROPERTY FOLDER "tools/bdf2fnt")
add_executable(fnt2fon fnt2fon.c)
set_target_properties(fnt2fon PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
set_target_properties(fnt2fon PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
)
set_property(TARGET fnt2fon PROPERTY FOLDER "tools/fnt2fon")

if ( CMAKE_COMPILER_IS_GNUCC )
    #set_property( TARGET main APPEND_STRING PROPERTY COMPILE_FLAGS -Wall )
endif ( CMAKE_COMPILER_IS_GNUCC )
if ( MSVC )
    set_property( TARGET bdf2fnt APPEND_STRING PROPERTY COMPILE_FLAGS /W1 )
    set_property( TARGET fnt2fon APPEND_STRING PROPERTY COMPILE_FLAGS /W1 )
endif ( MSVC )

message("bdf2fon in ${CMAKE_BINARY_DIR}")

#ALWAYS_32 = 1
