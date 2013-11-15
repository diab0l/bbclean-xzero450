set(bbKeys_RESOURCES bbKeys.rc)
set(bbKeys_SOURCES
	bbKeys.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
add_library(bbKeys SHARED ${bbKeys_SOURCES})
target_link_libraries(bbKeys blackbox)

install(FILES readme.txt DESTINATION plugins/bbKeys)
install(TARGETS bbKeys RUNTIME DESTINATION plugins/bbKeys)
if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/bbKeys/${bbKeys_RESOURCES}")
	install(FILES ${bbKeys_RESOURCES} DESTINATION plugins/bbKeys)
endif()
