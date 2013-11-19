project(bsetshell)

set(bsetshell_SOURCES
	bsetshell.cpp
)
set(bsetshell_RESOURCES
	resource.res
)

add_executable(bsetshell WIN32 ${bsetshell_SOURCES}) 
target_link_libraries(bsetshell bblib)
target_link_libraries(bsetshell version comctl32)

if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/${bsetshell_RESOURCES}")
	install(FILES ${bsetshell_RESOURCES} DESTINATION .)
endif()

install(TARGETS bsetshell RUNTIME DESTINATION .)


