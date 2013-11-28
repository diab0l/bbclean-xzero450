project(BlackboxZero)
cmake_minimum_required(VERSION 2.8.9)
set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_INSTALL_PREFIX "c:/bbLean_M")

set (CLEAN_FILES
  bbnote.ini
  docs/bbnote.*
  bbstylemaker.rc
  docs/bbstylemaker.txt
  docs/nls-c.txt
  bsetroot.rc
  docs/bsetroot.*
  docs/*.bmp
  *.exe *.dll
)

# font tools and conversion
add_subdirectory(build/bdf2fon)
add_custom_target(fonts ALL)
set(FONTDIR "${CMAKE_SOURCE_DIR}/fonts/artwiz-latin1-1.0-src" CACHE STRING "font directory")
set(FONTOUTDIR "${CMAKE_BINARY_DIR}/converted_fonts" CACHE STRING "font directory")
set(BDF2FNT ${CMAKE_BINARY_DIR}/bdf2fnt.exe CACHE STRING "bdf2fnt path")
set(FNT2FON ${CMAKE_BINARY_DIR}/fnt2fon.exe CACHE STRING "fnt2fon path")
add_custom_command(TARGET fonts COMMAND ${CMAKE_COMMAND}
	 -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
	 -DFONTDIR="${FONTDIR}"
	 -DBDF2FNT="${BDF2FNT}"
	 -DFNT2FON="${FNT2FON}"
	 -DFONTOUTDIR=${FONTOUTDIR}
	 ARGS -P ${CMAKE_SOURCE_DIR}/fonts/fonts.cmake)
install(DIRECTORY ${FONTOUTDIR} 
	DESTINATION .
	FILES_MATCHING PATTERN "*.fon"
	PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

# core modules
add_subdirectory(lib)
add_subdirectory(blackbox)
add_subdirectory(blackbox/Hooks)

# documentation
add_subdirectory(build/fuzzydoc)
#add_subdirectory(docs)

# plugins
add_subdirectory(3rd_party/iTunesCOMWindowsSDK)
add_subdirectory(plugins/bbKeys)
add_subdirectory(plugins/bbPlugin)
add_subdirectory(plugins/bbLeanBar)
add_subdirectory(plugins/bbFoomp)
add_subdirectory(plugins/bbLeanSkin)
add_subdirectory(plugins/bbSlit)
add_subdirectory(plugins/bbAnalog)
add_subdirectory(plugins/bbIconBox)
add_subdirectory(plugins/bbColor3dc)
add_subdirectory(plugins/bbWorkspaceWheel)
add_subdirectory(plugins/bbPlugin+)
add_subdirectory(plugins/bbLeanBar+)

# NOTE: following plugins are not yet mingw compatible
if (NOT MINGW)
	add_subdirectory(plugins/BBPager)
	add_subdirectory(plugins/BBStyle)
	add_subdirectory(plugins/bbInterface_iTunes)
endif (NOT MINGW)

add_subdirectory(plugins/bbRecycleBin)
add_subdirectory(plugins/bbInterface)

# tools
add_subdirectory(tools/bsetshell)
add_subdirectory(tools/bsetroot)
add_subdirectory(tools/bbstylemaker)
add_subdirectory(tools/bbnote)

# installation
function(install_if_absent)
	foreach(f ${ARGN})
		if(NOT EXISTS \"${CMAKE_INSTALL_PREFIX}/${f}\")
			install(FILES ${f} DESTINATION .)
		endif()
	endforeach()
endfunction()

set(main_RESOURCES
  menu.rc 
  blackbox.rc
  extensions.rc
  plugins.rc
  shellfolders.rc
  stickywindows.ini
)

install_if_absent(${main_RESOURCES})
install(DIRECTORY styles backgrounds DESTINATION .)



###############################################################################
# debug stuff
#message("Debug variable dump:")
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#   message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
