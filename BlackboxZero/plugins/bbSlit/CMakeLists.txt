set(bbSlit_SOURCES
	bbSlit.cpp
)

set(bbSlit_RESOURCES bbSlit.rc)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(bbSlit SHARED ${bbSlit_SOURCES})
target_link_libraries(bbSlit blackbox bblib bbPlugin)
target_link_libraries(bbSlit version comctl32)

set_property(TARGET bbSlit PROPERTY FOLDER "plugins/bbSlit")

install(FILES readme.txt DESTINATION plugins/bbSlit)
install(TARGETS bbSlit RUNTIME DESTINATION plugins/bbSlit)
if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/bbSlit/${bbSlit_RESOURCES}")
	install(FILES ${bbSlit_RESOURCES} DESTINATION plugins/bbSlit)
endif()

