
set(BBMagnify_SOURCES
  bbmagnify.cpp
  getStats.cpp
)

set(BBMagnify_RESOURCES Resource.rc)
#set(blackbox_RESOURCES resource.rc)
#set(CMAKE_RC_COMPILER_INIT windres)
#ENABLE_LANGUAGE(RC)

#readme.txt
#Mortar.ico
#BBMagnify.html

#add_definitions(-DUNICODE)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(BBMagnify SHARED ${BBMagnify_SOURCES} ${BBMagnify_RESOURCES})
target_link_libraries(BBMagnify blackbox bblib bbPlugin)
target_link_libraries(BBMagnify version comctl32 gdiplus winmm msimg32)

set_property(TARGET BBMagnify PROPERTY FOLDER "plugins/BBMagnify")

#INSTALL_IF_NEW = BBMagnify.rc
#install(FILES changes.txt DESTINATION plugins/BBMagnify)
install(TARGETS BBMagnify RUNTIME DESTINATION plugins/BBMagnify)
#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/BBMagnify/${BBMagnify_RESOURCES}")
#	install(FILES ${BBMagnify_RESOURCES} DESTINATION plugins/BBMagnify)
#endif()

