project(BBMessageBox)
set(BBMessageBox_SOURCES
  BBMessageBox.cpp
)

#set(BBMessageBox_RESOURCES Resource.rc) # FIXME: missing include in this file
#set(CMAKE_RC_COMPILER_INIT windres)
#ENABLE_LANGUAGE(RC)

#readme.txt
#Mortar.ico
#BBMessageBox.html

#add_definitions(-DUNICODE)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(BBMessageBox SHARED ${BBMessageBox_SOURCES} ${BBMessageBox_RESOURCES})
target_link_libraries(BBMessageBox blackbox bblib bbPlugin)
target_link_libraries(BBMessageBox version comctl32 gdiplus winmm msimg32 shlwapi comdlg32)

set_property(TARGET BBMessageBox PROPERTY FOLDER "plugins/BBMessageBox")

#INSTALL_IF_NEW = BBMessageBox.rc
#install(FILES changes.txt DESTINATION plugins/BBMessageBox)
install(TARGETS BBMessageBox RUNTIME DESTINATION plugins/BBMessageBox)
#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/BBMessageBox/${BBMessageBox_RESOURCES}")
#	install(FILES ${BBMessageBox_RESOURCES} DESTINATION plugins/BBMessageBox)
#endif()

