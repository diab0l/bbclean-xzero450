project(BBRSS)
set(BBRSS_SOURCES
  BBRSS.cpp
  #  MessageBox.cpp
)

set(BBRSS_RESOURCES Resource.rc)
#set(CMAKE_RC_COMPILER_INIT windres)
#ENABLE_LANGUAGE(RC)

#readme.txt
#Mortar.ico
#BBRSS.html

#add_definitions(-DUNICODE)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(BBRSS SHARED ${BBRSS_SOURCES})
target_link_libraries(BBRSS blackbox bblib bbPlugin)
target_link_libraries(BBRSS version comctl32 gdiplus winmm msimg32)

set_property(TARGET BBRSS PROPERTY FOLDER "plugins/BBRSS")

#INSTALL_IF_NEW = BBRSS.rc
#install(FILES changes.txt DESTINATION plugins/BBRSS)
install(TARGETS BBRSS RUNTIME DESTINATION plugins/BBRSS)
#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/BBRSS/${BBRSS_RESOURCES}")
#	install(FILES ${BBRSS_RESOURCES} DESTINATION plugins/BBRSS)
#endif()

