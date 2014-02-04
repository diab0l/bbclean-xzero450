
function(install_if_absent)
	foreach(f ${ARGN})
		if(NOT EXISTS \"${CMAKE_INSTALL_PREFIX}/plugins/bbLeanSkin/${f}\")
			install(FILES ${f} DESTINATION plugins/bbLeanSkin)
		endif()
	endforeach()
endfunction()
function(installdir_if_absent)
	foreach(f ${ARGN})
		if(NOT EXISTS \"${CMAKE_INSTALL_PREFIX}/plugins/bbLeanSkin/${f}\")
			install(DIRECTORY ${f} DESTINATION plugins/bbLeanSkin)
		endif()
	endforeach()
endfunction()

# bbLeanSkin.dll

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(bbLeanSkin SHARED bbLeanSkin.cpp)
target_link_libraries(bbLeanSkin blackbox bblib bbPlugin)
target_link_libraries(bbLeanSkin version comctl32)
set_property(TARGET bbLeanSkin PROPERTY FOLDER "plugins/bbLeanSkin")
install(FILES readme.txt DESTINATION plugins/bbLeanSkin)
install(TARGETS bbLeanSkin RUNTIME DESTINATION plugins/bbLeanSkin)
install_if_absent(bbLeanSkin.rc bbLeanSkinMenu.rc exclusions.rc)
installdir_if_absent(buttons)

#TODO:
#INSTALL_FILES = $(BIN) readme.txt
#INSTALL_IF_NEW = bbLeanSkin.rc bbLeanSkinMenu.rc exclusions.rc -subdir buttons buttons_11x11.bmp

# bbLeanSkinEng.dll

set(target_name "bbLeanSkinEng")
add_library(${target_name}
	SHARED
	engine/hookctl.cpp
	engine/subclass.cpp
)
target_link_libraries(${target_name} blackbox bblib bbPlugin)
target_link_libraries(${target_name} version comctl32)
install(TARGETS ${target_name} RUNTIME DESTINATION plugins/bbLeanSkin)
set_property(TARGET ${target_name} PROPERTY FOLDER "plugins/bbLeanSkin")

