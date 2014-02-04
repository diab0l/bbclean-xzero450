#INSTALL_FILES = $(BIN) -as bbLeanBar+.txt readme.txt
#INSTALL_IF_NEW = bbLeanBar.rc
set(bbLeanBar2_SOURCES
	bbLeanBar+.cpp
	bbLeanBar+.cpp
)

set(bbLeanBar2_CONFIGS
  bbLeanBar.rc
)

set(bbLeanBar2_RESOURCES bbLeanBar+_private.rc)

set(CMAKE_RC_COMPILER_INIT windres)
ENABLE_LANGUAGE(RC)

add_definitions(-D__LeanBarPlus__)
add_definitions(-DNO_BBLIB)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin+)
add_library(bbLeanBar+ SHARED ${bbLeanBar2_SOURCES} ${bbLeanBar2_RESOURCES})
target_link_libraries(bbLeanBar+ blackbox bblib bbPlugin+)
target_link_libraries(bbLeanBar+ version comctl32)

set_property(TARGET bbLeanBar+ PROPERTY FOLDER "plugins/bbLeanBar+")

install(TARGETS bbLeanBar+ RUNTIME DESTINATION plugins/bbLeanBar+)
if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/bbLeanBar/${bbLeanBar2_CONFIGS}")
	install(FILES ${bbLeanBar2_CONFIGS} DESTINATION plugins/bbLeanBar+)
endif()
