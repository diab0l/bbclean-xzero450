project(BBSysMeter)
set(BBSysMeter_SOURCES
  bbsysmeter.cpp
  getStats.cpp
)

set(BBSysMeter_RESOURCES Resource.rc)
#set(CMAKE_RC_COMPILER_INIT windres)
#ENABLE_LANGUAGE(RC)

#readme.txt
#Mortar.ico
#BBSysMeter.html

#add_definitions(-DUNICODE)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(BBSysMeter SHARED ${BBSysMeter_SOURCES})
target_link_libraries(BBSysMeter blackbox bblib bbPlugin)
target_link_libraries(BBSysMeter version comctl32 gdiplus winmm msimg32)

set_property(TARGET BBSysMeter PROPERTY FOLDER "plugins/BBSysMeter")

#INSTALL_IF_NEW = BBSysMeter.rc
#install(FILES changes.txt DESTINATION plugins/BBSysMeter)
install(TARGETS BBSysMeter RUNTIME DESTINATION plugins/BBSysMeter)
#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/BBSysMeter/${BBSysMeter_RESOURCES}")
#	install(FILES ${BBSysMeter_RESOURCES} DESTINATION plugins/BBSysMeter)
#endif()

