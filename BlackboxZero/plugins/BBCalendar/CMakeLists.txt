project(bbCalendar)
set(bbCalendar_SOURCES
  bbcalendar.cpp
)

set(bbCalendar_RESOURCES Resource.rc)
#set(CMAKE_RC_COMPILER_INIT windres)
#ENABLE_LANGUAGE(RC)

#readme.txt
#Mortar.ico
#BBCalendar.html

#add_definitions(-DUNICODE)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(bbCalendar SHARED ${bbCalendar_SOURCES})
target_link_libraries(bbCalendar blackbox bblib bbPlugin)
target_link_libraries(bbCalendar version comctl32 gdiplus winmm msimg32)

set_property(TARGET bbCalendar PROPERTY FOLDER "plugins/bbCalendar")

#INSTALL_IF_NEW = bbCalendar.rc
#install(FILES changes.txt DESTINATION plugins/bbCalendar)
install(TARGETS bbCalendar RUNTIME DESTINATION plugins/bbCalendar)
#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/bbCalendar/${bbCalendar_RESOURCES}")
#	install(FILES ${bbCalendar_RESOURCES} DESTINATION plugins/bbCalendar)
#endif()

