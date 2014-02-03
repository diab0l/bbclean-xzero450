# bbLeanSkinEng32.dll on 64b system
cmake_minimum_required(VERSION 2.8)

set(target_name32 "bbLeanSkinEng32")

add_library(${target_name32}
  SHARED
  engine/hookctl.cpp
  engine/subclass.cpp
)

target_link_libraries(${target_name32} blackbox bblib bbPlugin)
target_link_libraries(${target_name32} version comctl32)
install(TARGETS ${target_name32} RUNTIME DESTINATION plugins/bbLeanSkin)
set_property(TARGET ${target_name32} PROPERTY FOLDER "plugins/bbLeanSkin")


###############################################################################
# debug stuff
message("Debug variable dump:")
get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
   message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
