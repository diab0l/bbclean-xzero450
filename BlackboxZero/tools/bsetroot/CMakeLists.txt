project(bsetroot)
#TODO: image_tiny.obj bsetroot.htm

add_subdirectory(CXIMAGE)

# bsetroot
set(bsetroot_SOURCES
	bsetroot.cpp
	image_cx.cpp
	${CMAKE_SOURCE_DIR}/blackbox/BImage.cpp
)
set(bsetroot_RESOURCES
	bsetroot.rc
)

add_executable(bsetroot WIN32 ${bsetroot_SOURCES})
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(bsetroot cximage zlib jpeg png)
target_link_libraries(bsetroot bblib blackbox)
target_link_libraries(bsetroot version comctl32 )

if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/${bsetroot_RESOURCES}")
	install(FILES ${bsetroot_RESOURCES} DESTINATION .)
endif()

# bsetbg
set(bsetbg_SOURCES
	bsetbg.cpp
)
set(bsetbg_RESOURCES
	bsbg-rsc.rc
)

add_executable(bsetbg WIN32 ${bsetbg_SOURCES}) 
target_link_libraries(bsetbg bblib)
target_link_libraries(bsetbg version comctl32)

if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/${bsetbg_RESOURCES}")
	install(FILES ${bsetbg_RESOURCES} DESTINATION .)
endif()

# common install
install(TARGETS bsetroot bsetbg
		RUNTIME DESTINATION .
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib/static)


