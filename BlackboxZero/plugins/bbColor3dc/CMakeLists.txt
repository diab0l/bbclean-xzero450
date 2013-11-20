set(bbColor3dc_SOURCES
	bbColor3dc.cpp
)

#set(bbColor3dc_RESOURCES bbColor3dc.rc)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(bbColor3dc SHARED ${bbColor3dc_SOURCES})
target_link_libraries(bbColor3dc blackbox bblib bbPlugin)
target_link_libraries(bbColor3dc version comctl32)

install(FILES readme.txt DESTINATION plugins/bbColor3dc)
install(TARGETS bbColor3dc RUNTIME DESTINATION plugins/bbColor3dc)
#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/bbColor3dc/${bbColor3dc_RESOURCES}")
#	install(FILES ${bbColor3dc_RESOURCES} DESTINATION plugins/bbColor3dc)
#endif()

