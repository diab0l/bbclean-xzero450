project(fuzzydoc)
add_executable(fuzzydoc fuzzydoc.c)
set_target_properties(fuzzydoc PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
set_target_properties(fuzzydoc PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
)

