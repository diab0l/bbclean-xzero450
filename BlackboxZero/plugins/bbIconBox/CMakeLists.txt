set(bbIconBox_SOURCES
	bbIconBox.cpp
	LoadFolder.cpp
	DropTarget.cpp
	Utils.cpp
	winlist.cpp
	#	resource.res
)

#resource.res: drag1.cur drag2.cur

set(bbIconBox_RESOURCES bbIconBox.rc)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(bbIconBox SHARED ${bbIconBox_SOURCES})
target_link_libraries(bbIconBox blackbox bblib bbPlugin)
target_link_libraries(bbIconBox version comctl32)

install(FILES readme.txt DESTINATION plugins/bbIconBox)
install(TARGETS bbIconBox RUNTIME DESTINATION plugins/bbIconBox)
if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/bbIconBox/${bbIconBox_RESOURCES}")
	install(FILES ${bbIconBox_RESOURCES} DESTINATION plugins/bbIconBox)
endif()

