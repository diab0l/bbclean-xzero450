#INSTALL_FILES = $(BIN) readme.txt
#INSTALL_IF_NEW = bbLeanBar.rc

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)

set(bbLeanBar_RESOURCES bbLeanBar.rc)
set(bbLeanBar_SOURCES
	bbLeanBar.cpp
)

add_library(bbLeanBar SHARED ${bbLeanBar_SOURCES})
target_link_libraries(bbLeanBar blackbox bblib bbPlugin)
target_link_libraries(bbLeanBar version comctl32)

