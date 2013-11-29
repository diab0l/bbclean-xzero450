
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)

set(SystemBarEx_RESOURCES systembarex.rc)
set(SystemBarEx_SOURCES
	bbAbout.cpp
	bbTooltip.cpp
	IconItem.cpp
	SystemBarEx.cpp
	TinyDropTarg.cpp
)

add_library(SystemBarEx SHARED ${SystemBarEx_SOURCES})
target_link_libraries(SystemBarEx blackbox bblib bbPlugin)
target_link_libraries(SystemBarEx version comctl32)

set_property(TARGET SystemBarEx PROPERTY FOLDER "plugins/SystemBarEx")

#install(FILES readme.txt DESTINATION plugins/SystemBarEx)
install(TARGETS SystemBarEx RUNTIME DESTINATION plugins/SystemBarEx)
if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/SystemBarEx/${SystemBarEx_RESOURCES}")
	install(FILES ${SystemBarEx_RESOURCES} DESTINATION plugins/SystemBarEx)
endif()

