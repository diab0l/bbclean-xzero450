project(BBDigitalEx)
set(BBDigitalEx_SOURCES
  bbdigitalex.cpp
  #  MessageBox.cpp
)

set(BBDigitalEx_RESOURCES Resource.rc)
#set(blackbox_RESOURCES resource.rc)
#set(CMAKE_RC_COMPILER_INIT windres)
#ENABLE_LANGUAGE(RC)

#readme.txt
#Mortar.ico
#BBDigitalEx.html

#add_definitions(-DUNICODE)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(BBDigitalEx SHARED ${BBDigitalEx_SOURCES})
target_link_libraries(BBDigitalEx blackbox bblib bbPlugin)
target_link_libraries(BBDigitalEx version comctl32 gdiplus winmm msimg32)

set_property(TARGET BBDigitalEx PROPERTY FOLDER "plugins/BBDigitalEx")

#INSTALL_IF_NEW = BBDigitalEx.rc
#install(FILES changes.txt DESTINATION plugins/BBDigitalEx)
install(TARGETS BBDigitalEx RUNTIME DESTINATION plugins/BBDigitalEx)
#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/BBDigitalEx/${BBDigitalEx_RESOURCES}")
#	install(FILES ${BBDigitalEx_RESOURCES} DESTINATION plugins/BBDigitalEx)
#endif()

