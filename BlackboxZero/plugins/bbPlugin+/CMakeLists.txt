include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)

set(bbPlugin+_RESOURCES bbLeanBar.rc)
set(bbPlugin+_SOURCES
	bbPlugin+.cpp
	drawico+.cpp
	sysmenu+.cpp
	tooltips+.cpp
)

add_library(bbPlugin+ STATIC ${bbPlugin+_SOURCES})

