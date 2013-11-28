#INSTALL_FILES = $(BIN) readme.txt
#INSTALL_IF_NEW = bbFoomp.rc

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)

set(bbFoomp_RESOURCES bbFoomp.rc)
set(bbFoomp_SOURCES
	src/bbfoomp.cpp
)

add_library(bbFoomp SHARED ${bbFoomp_SOURCES})
target_link_libraries(bbFoomp blackbox bblib bbPlugin)
target_link_libraries(bbFoomp version comctl32)

#install(FILES readme.txt DESTINATION plugins/bbFoomp)
install(TARGETS bbFoomp RUNTIME DESTINATION plugins/bbFoomp)
if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/bbFoomp/${bbFoomp_RESOURCES}")
	install(FILES ${bbFoomp_RESOURCES} DESTINATION plugins/bbFoomp)
endif()

