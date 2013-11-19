
set(BBStyle_SOURCES
	BBStyle.cpp
#   resource.res
)

#INSTALL_FILES = $(BIN) BBStyle.html
#INSTALL_IF_NEW = resource.rc

include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
#add_executable(bbnote WIN32 ${BBStyle_SOURCES}) 
add_library(BBStyle SHARED ${BBStyle_SOURCES})
target_link_libraries(BBStyle bblib blackbox)
target_link_libraries(BBStyle version comctl32 shlwapi)



