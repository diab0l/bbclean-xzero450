project(BBXO)
set(BBXO_SOURCES
  BBXO.cpp
)

set(BBXO_RESOURCES Resource.rc)
#set(CMAKE_RC_COMPILER_INIT windres)
#ENABLE_LANGUAGE(RC)

#readme.txt
#Mortar.ico
#BBXO.html

#add_definitions(-DUNICODE)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(BBXO SHARED ${BBXO_SOURCES})
target_link_libraries(BBXO blackbox bblib bbPlugin)
target_link_libraries(BBXO version comctl32 gdiplus winmm msimg32 shlwapi comdlg32)

set_property(TARGET BBXO PROPERTY FOLDER "plugins/BBXO")

#INSTALL_IF_NEW = BBXO.rc
#install(FILES changes.txt DESTINATION plugins/BBXO)
install(TARGETS BBXO RUNTIME DESTINATION plugins/BBXO)
#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/BBXO/${BBXO_RESOURCES}")
#	install(FILES ${BBXO_RESOURCES} DESTINATION plugins/BBXO)
#endif()

