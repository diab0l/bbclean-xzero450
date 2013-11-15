project(blackbox)

include("${CMAKE_SOURCE_DIR}/build/version.cmake")

add_custom_target(bbversion_h ALL)
add_custom_command(TARGET bbversion_h COMMAND ${CMAKE_COMMAND}
         -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
         -P ${CMAKE_SOURCE_DIR}/build/version.cmake)

set(blackbox_SOURCES
	BBApi.cpp
	BBRoot.cpp
	BBVWM.cpp
	BImage.cpp
	Blackbox.cpp
	ConfigMenu.cpp
	Desk.cpp
	DesktopMenu.cpp
	MessageManager.cpp
	PluginManager.cpp
	Settings.cpp
	Toolbar.cpp
	Tray.cpp
	Utils.cpp
	Workspaces.cpp
	Menu/CommandItem.cpp
	Menu/Contextmenu.cpp
	Menu/Dragsource.cpp
	Menu/Droptarget.cpp
	Menu/FolderItem.cpp
	Menu/Menu.cpp
	Menu/MenuItem.cpp
	Menu/MenuMaker.cpp
	Menu/RecentItem.cpp
	Menu/SpecialFolder.cpp
	Menu/TitleItem.cpp
)

set (blackbox_HEADERS
	BB.h
	BBApi.h
	BBSendData.h
	bbversion.h
	BBVWM.h
	BImage.h
	Desk.h
	MessageManager.h
	PluginManager.h
	Settings.h
	Stylestruct.h
	Toolbar.h
	Tray.h
	win0x500.h
	Workspaces.h
	Menu/Menu.h
	Menu/MenuMaker.h
	Menu/RecentItem.h
)

set(blackbox_RESOURCES resource.res)

add_definitions(-D__BBCORE__)
include_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(blackbox WIN32
	${blackbox_SOURCES}
	${blackbox_HEADERS}
	${blackbox_RESOURCES}
)

set_target_properties(blackbox PROPERTIES ENABLE_EXPORTS ON)
target_link_libraries(blackbox bblib)
target_link_libraries(blackbox version comctl32)

# old stuff from makefile
#VPATH = Menu
#INSTALL_FILES = $(BIN) -to docs Menu/menu-bullets.bmp nls-c.txt

#include $(TOP)/build/makefile.inc

