#IF(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")

function(install_if_absent)
	foreach(f ${ARGN})
		if(NOT EXISTS \"${CMAKE_INSTALL_PREFIX}/plugins/bbLeanSkin/${f}\")
			install(FILES ${f} DESTINATION plugins/bbLeanSkin)
		endif()
	endforeach()
endfunction()
function(installdir_if_absent arg1)
	foreach(f ${ARGN})
		if(NOT EXISTS \"${CMAKE_INSTALL_PREFIX}/plugins/bbLeanSkin/${f}\")
			install(DIRECTORY ${f} DESTINATION plugins/bbLeanSkin)
		endif()
	endforeach()
endfunction()


#ifeq "$(PROG)" "dll"
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(bbLeanSkin SHARED bbLeanSkin.cpp)
target_link_libraries(bbLeanSkin blackbox bblib bbPlugin)
target_link_libraries(bbLeanSkin version comctl32)
install(FILES readme.txt DESTINATION plugins/bbLeanSkin)
install(TARGETS bbLeanSkin RUNTIME DESTINATION plugins/bbLeanSkin)
install_if_absent(bbLeanSkin.rc bbLeanSkinMenu.rc exclusions.rc)
installdir_if_absent(buttons)

  #INSTALL_FILES = $(BIN) readme.txt
  #INSTALL_IF_NEW = bbLeanSkin.rc bbLeanSkinMenu.rc exclusions.rc -subdir buttons buttons_11x11.bmp
#endif

#ifeq "$(PROG)" "eng"
#add_library(bbLeanSkinEng SHARED bbLeanSkin.cpp engine/hookctl.cpp engine/subclass.cpp)
#target_link_libraries(bbLeanSkinEng blackbox bblib bbPlugin)
#target_link_libraries(bbLeanSkinEng version comctl32)
  #BIN = bbLeanSkinEng.dll
  #NO_BBLIB = 1
  #INSTALL_FILES = $(BIN)
  #LFLAGS += $(EXTRA_LFLAGS)
#endif

#	ifeq "$(PROG)" "eng32"
#	  BIN = bbLeanSkinEng32.dll
#	  OBJ = hookctl32.obj subclass32.obj BImage32.obj drawico32.obj
#	  NO_BBLIB = 1
#	  IMPLIB = 1
#	  ALWAYS_32 = 1
#	  INSTALL_FILES = $(BIN)
#	  DEFINES += -D BBLEANSKIN_ENG32
#	  LFLAGS += $(EXTRA_LFLAGS)
#	endif
#
#	ifeq "$(PROG)" "run32"
#	  BIN = bbLeanSkinRun32.exe
#	  OBJ = bbLeanSkinRun32.obj
#	  NO_BBLIB = 1
#	  ALWAYS_32 = 1
#	  LIBS = $(call LIBNAME,bbLeanSkinEng32)
#	  INSTALL_FILES = $(BIN)
#	endif
#
#	include $(TOP)/build/makefile.inc
#
#	VPATH += engine
#	vpath %.cpp $(BBAPI)
#	ENG_H = hookinfo.h subclass.h
#
#	bbLeanSkin.obj bbLeanSkinRun32.obj : $(ENG_H)
#	subclass.obj subclass32.obj : $(ENG_H) $(BBAPI)/bImage.cpp
#	hookctl.obj hookctl32.obj : $(ENG_H)
#	readstyle.obj : $(BBAPI)/settings.cpp
#
#	%32.obj: %.cpp
#		$(COMPILE.cpp)
#
## For some reason the gcc 3.4 (rt 3.7) hangs in it's _DllMainCRTStartup
## That's why it is bypassed here by starting with DllMain directly
#	ifeq "$(USING_MINGW)" "1"
#	EXTRA_LFLAGS = -Wl,-e,_DllMain@12
#	endif
#
#  message("f='${f}' arg1='${arg1}' arg2='${arg2}' ARGV2='${ARGV2}'")
#endfunction()
## --------------------------------------------------------------------
#ifndef PROG
## --------------------------------------------------------------------
#
#	include $(TOP)/build/makefile.inc
#
#	all.progs :
#		$(MAKE) $* PROG=dll
#		$(MAKE) $* PROG=eng
#	ifeq "$(BB64)" "1"
#		$(MAKE) $* PROG=eng32
#		$(MAKE) $* PROG=run32
#	endif
#
#	clean.progs :
#		$(MAKE) $* PROG=dll
#		$(MAKE) $* PROG=eng
#		$(MAKE) $* PROG=eng32
#		$(MAKE) $* PROG=run32
#
## --------------------------------------------------------------------
#else
## --------------------------------------------------------------------
#
#
## --------------------------------------------------------------------
#endif
## --------------------------------------------------------------------
#


