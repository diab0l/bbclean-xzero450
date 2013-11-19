set(bbstylemaker_SOURCES
	main.cpp
	writestyle.cpp
	${CMAKE_SOURCE_DIR}/blackbox/BImage.cpp
	${CMAKE_SOURCE_DIR}/blackbox/Settings.cpp
	#resource.res
	
	#readroot.cpp #included manually in writestyle.cpp
	#rcfile.cpp #included manually in writestyle.cpp
	#utils.cpp #included manually in writestyle.cpp
	#snap.cpp #included manually in bbdialog.cpp
	#bbdialog.cpp # that one from main.cpp
)

#INSTALL_FILES = $(BIN) -as docs/bbstylemaker.txt readme.txt
set(bbstylemaker_RESOURCES
	resource.rc
)

add_definitions(-DBBSETTING_STYLEREADER_ONLY) #@FIXME
#add_definitions(-D__BBCORE__)
add_definitions(-DBBLIB_COMPILING)
add_definitions(-D__BBSM__) #used by Settings.cpp
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
add_executable(bbstylemaker WIN32 ${bbstylemaker_SOURCES}) 
target_link_libraries(bbstylemaker bblib)
target_link_libraries(bbstylemaker version comctl32)

#INSTALL_IF_NEW = bbstylemaker.rc
if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/${bbstylemaker_RESOURCES}")
	install(FILES ${bbstylemaker_RESOURCES} DESTINATION .)
endif()

install(TARGETS bbstylemaker
		RUNTIME DESTINATION .
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib/static)

install(FILES readme.txt bbstylemaker.ico DESTINATION .)

#TODO: where is bbstylemaker.txt?
#install(FILES docs/bbstylemaker.txt DESTINATION docs)
