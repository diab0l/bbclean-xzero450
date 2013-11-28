
set(bbInterface_SOURCES
	AgentMaster.cpp
	AgentType_Bitmap.cpp
	AgentType_Broam.cpp
	AgentType_CompoundText.cpp
	AgentType_Graph.cpp
	AgentType_Mixer.cpp
	AgentType_Run.cpp
	AgentType_StaticText.cpp
	AgentType_SwitchedState.cpp
	AgentType_System.cpp
	AgentType_SystemMonitor.cpp
	AgentType_TGA.cpp
	AgentType_Winamp.cpp
	ConfigMaster.cpp
	ControlMaster.cpp
	ControlType_Button.cpp
	ControlType_Label.cpp
	ControlType_Slider.cpp
	DialogMaster.cpp
	ListMaster.cpp
	MenuMaster.cpp
	MessageMaster.cpp
	ModuleMaster.cpp
	PluginMaster.cpp
	SlitManager.cpp
	SnapWindow.cpp
	StyleMaster.cpp
	Tooltip.cpp
	WindowMaster.cpp
)

set(bbInterface_RESOURCES bbinterface.rc)

#add_definitions(-DUNICODE)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
include_directories(${CMAKE_SOURCE_DIR}/3rd_party/iTunesCOMWindowsSDK)
add_library(bbInterface SHARED ${bbInterface_SOURCES})
target_link_libraries(bbInterface blackbox bblib bbPlugin)
target_link_libraries(bbInterface version comctl32 gdiplus winmm msimg32)

#INSTALL_IF_NEW = bbinterface.rc
#install(FILES changes.txt DESTINATION plugins/bbInterface)
install(TARGETS bbInterface RUNTIME DESTINATION plugins/bbInterface)
#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/bbinterface/${bbinterface_RESOURCES}")
#	install(FILES ${bbinterface_RESOURCES} DESTINATION plugins/bbinterface)
#endif()

