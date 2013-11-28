set(bbRecycleBin_SOURCES
	bbRecycleBin.cpp
)

set(bbRecycleBin_RESOURCES bbRecycleBin.rc)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)

if (MINGW)
	add_definitions(-D_WIN32_IE=0x0601)
endif (MINGW)
add_library(bbRecycleBin SHARED ${bbRecycleBin_SOURCES})
target_link_libraries(bbRecycleBin blackbox bblib bbPlugin)
target_link_libraries(bbRecycleBin version comctl32 shlwapi)

install(FILES readme.txt DESTINATION plugins/bbRecycleBin)
install(TARGETS bbRecycleBin RUNTIME DESTINATION plugins/bbRecycleBin)
#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/bbRecycleBin/${bbRecycleBin_RESOURCES}")
#	install(FILES ${bbRecycleBin_RESOURCES} DESTINATION plugins/bbRecycleBin)
#endif()

