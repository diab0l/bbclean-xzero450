set(BBPager_SOURCES
	BBPager.cpp
	Drawing.cpp
	Settings.cpp
	StdAfx.cpp
	Style.cpp
)

set(BBPager_CONFIGS bbpager.rc)

#FIXME: complains about missing #include "afxres.h"
set(BBPager_RESOURCES
  #   resource.rc 
)

set(CMAKE_RC_COMPILER_INIT windres)
ENABLE_LANGUAGE(RC)


include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(BBPager SHARED ${BBPager_SOURCES} ${BBPager_RESOURCES})
target_link_libraries(BBPager blackbox bblib bbPlugin)
target_link_libraries(BBPager version comctl32)

set_property(TARGET BBPager PROPERTY FOLDER "plugins/BBPager")

#INSTALL_IF_NEW = bbpager.rc
install(FILES changes.txt DESTINATION plugins/BBPager)
install(TARGETS BBPager RUNTIME DESTINATION plugins/BBPager)
#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/BBPager/${BBPager_CONFIGS}")
#	install(FILES ${BBPager_CONFIGS} DESTINATION plugins/BBPager)
#endif()

