cmake_minimum_required(VERSION 2.8.9)
project(BlackboxZero)
set(CMAKE_VERBOSE_MAKEFILE on)

set(INSTALL_IF_NEW 
  menu.rc 
  blackbox.rc
  extensions.rc
  plugins.rc
  shellfolders.rc
  stickywindows.ini
)

#set (INSTALL_FILES = \
#  -subdir fonts . \
#  -subdir styles lean_blue \
#  -subdir backgrounds .
#)

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

#add_subdirectory(build)
add_subdirectory(lib)
add_subdirectory(blackbox)
#add_subdirectory(blackbox/Hooks)
#add_subdirectory(build/fuzzydoc)
#add_subdirectory(docs)

add_subdirectory(plugins/bbKeys)
add_subdirectory(plugins/bbPlugin)
add_subdirectory(plugins/bbLeanBar)
#add_subdirectory(plugins/bbLeanSkin)
#add_subdirectory(plugins/bbSlit)
#add_subdirectory(plugins/bbAnalog)
#add_subdirectory(plugins/bbIconBox)
#add_subdirectory(plugins/bbColor3dc)
#add_subdirectory(plugins/bbWorkspaceWheel)

add_subdirectory(plugins/bbLeanBar+)
#add_subdirectory(plugins/bbRecycleBin)
#add_subdirectory(plugins/BBPager)
#add_subdirectory(plugins/BBStyle)

#add_subdirectory(tools/bsetshell)
#add_subdirectory(tools/bsetroot)
#add_subdirectory(tools/bbstylemaker)
#add_subdirectory(tools/bbnote)

#add_subdirectory(build/bdf2fon)
#add_subdirectory(fonts)

###############################################################################
# debug stuff
#message("Debug variable dump:")
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#   message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
