set(BBPager_SOURCES
	BBPager.cpp
	Drawing.cpp
	Settings.cpp
	StdAfx.cpp
	Style.cpp
)

set(BBPager_RESOURCES bbpager.rc)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(BBPager SHARED ${BBPager_SOURCES})
target_link_libraries(BBPager blackbox bblib bbPlugin)
target_link_libraries(BBPager version comctl32)

set_property(TARGET BBPager PROPERTY FOLDER "plugins/BBPager")

#INSTALL_IF_NEW = bbpager.rc
install(FILES changes.txt DESTINATION plugins/BBPager)
install(TARGETS BBPager RUNTIME DESTINATION plugins/BBPager)
#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/BBPager/${BBPager_RESOURCES}")
#	install(FILES ${BBPager_RESOURCES} DESTINATION plugins/BBPager)
#endif()

