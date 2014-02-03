# bbLeanSkinEng32.dll on 64b system
cmake_minimum_required(VERSION 2.8)

set(target_name32 "bbLeanSkinEng32")

add_library(${target_name32}
  SHARED
  ../engine/hookctl.cpp
  ../engine/subclass.cpp
  ../../bbPlugin/drawico.cpp # FIXME: required DrawIconSatnHue
)

#FIXME: the paths are bit messed, this is a hotfix
include_directories(${CMAKE_SOURCE_DIR}/../../../blackbox)
include_directories(${CMAKE_SOURCE_DIR}/../../../lib)
include_directories(${CMAKE_SOURCE_DIR}/../../../plugins/bbPlugin)

target_link_libraries(${target_name32} version comctl32)
install(TARGETS ${target_name32} RUNTIME DESTINATION plugins/bbLeanSkin)


###############################################################################
# debug stuff
#message("Debug variable dump:")
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#   message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
