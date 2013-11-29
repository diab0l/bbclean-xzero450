project(fuzzydoc)
add_executable(fuzzydoc fuzzydoc.c)
set_target_properties(fuzzydoc PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
set_target_properties(fuzzydoc PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
)

if ( CMAKE_COMPILER_IS_GNUCC )
    #set_property( TARGET main APPEND_STRING PROPERTY COMPILE_FLAGS -Wall )
endif ( CMAKE_COMPILER_IS_GNUCC )
if ( MSVC )
    set_property( TARGET fuzzydoc APPEND_STRING PROPERTY COMPILE_FLAGS /W1 )
endif ( MSVC )
