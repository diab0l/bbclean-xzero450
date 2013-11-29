set(bbWorkspaceWheel_SOURCES
	bbWorkspaceWheel.cpp
)

set(bbWorkspaceWheel_RESOURCES bbWorkspaceWheel.rc)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(bbWorkspaceWheel SHARED ${bbWorkspaceWheel_SOURCES})
target_link_libraries(bbWorkspaceWheel blackbox bblib bbPlugin)
target_link_libraries(bbWorkspaceWheel version comctl32)

set_property(TARGET bbWorkspaceWheel PROPERTY FOLDER "plugins/bbWorkspaceWheel")

install(FILES readme.txt DESTINATION plugins/bbWorkspaceWheel)
install(TARGETS bbWorkspaceWheel RUNTIME DESTINATION plugins/bbWorkspaceWheel)
#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/bbWorkspaceWheel/${bbWorkspaceWheel_RESOURCES}")
#	install(FILES ${bbWorkspaceWheel_RESOURCES} DESTINATION plugins/bbWorkspaceWheel)
#endif()

