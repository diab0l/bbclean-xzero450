#INSTALL_FILES = $(BIN) -as bbLeanBar+.txt readme.txt
#INSTALL_IF_NEW = bbLeanBar.rc
set(bbLeanBar2_RESOURCES bbLeanBar.rc)
set(bbLeanBar2_SOURCES
	bbLeanBar+.cpp
	bbPlugin+.cpp
	drawico+.cpp
	moreutils+.cpp
	sysmenu+.cpp
	tooltips+.cpp
)

add_definitions(-D__LeanBarPlus__)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(bbLeanBar+ SHARED ${bbLeanBar2_SOURCES})
target_link_libraries(bbLeanBar+ blackbox bblib bbPlugin)
target_link_libraries(bbLeanBar+ version comctl32)

