include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)

set(bbPlugin_RESOURCES bbLeanBar.rc)
set(bbPlugin_SOURCES
	bbPlugin.cpp
	drawico.cpp
	sysmenu.cpp
	tooltips.cpp
)

add_library(bbPlugin STATIC ${bbPlugin_SOURCES})
target_link_libraries(bbPlugin blackbox bblib)

