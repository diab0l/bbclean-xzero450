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

add_subdirectory(lib)
add_subdirectory(blackbox)

#message("Debug variable dump:")
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#   message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

#MAKE_CORE   = $(MAKE) $* -C $1 "$I"
#MAKE_PLUGIN = $(MAKE) $* -C plugins/$1 "$I/plugins/$1"
#MAKE_TOOL   = $(MAKE) $* -C tools/$1 "$I"
#
#include $(TOP)/build/makefile.inc
#
#all.progs clean.progs :
#	#$(call MAKE_CORE,build)
#
#	$(call MAKE_CORE,lib)
#	$(call MAKE_CORE,blackbox)
#	$(call MAKE_CORE,blackbox/Hooks)
#	$(call MAKE_CORE,build/fuzzydoc)
#	$(call MAKE_CORE,docs)
#
#	$(call MAKE_PLUGIN,bbKeys)
#	$(call MAKE_PLUGIN,bbLeanBar)
#	$(call MAKE_PLUGIN,bbLeanSkin)
#	$(call MAKE_PLUGIN,bbSlit)
#	$(call MAKE_PLUGIN,bbAnalog)
#	$(call MAKE_PLUGIN,bbIconBox)
#	$(call MAKE_PLUGIN,bbColor3dc)
#	$(call MAKE_PLUGIN,bbWorkspaceWheel)
#
#	$(call MAKE_PLUGIN,bbLeanBar+)
#	$(call MAKE_PLUGIN,bbRecycleBin)
#	$(call MAKE_PLUGIN,BBPager)
#	$(call MAKE_PLUGIN,BBStyle)
#
#	$(call MAKE_TOOL,bsetshell)
#	$(call MAKE_TOOL,bsetroot)
#	$(call MAKE_TOOL,bbstylemaker)
#	$(call MAKE_TOOL,bbnote)
#
#	$(call MAKE_CORE,build/bdf2fon)
#	$(call MAKE_CORE,fonts)
#
## --------------------------------------------------------------------
