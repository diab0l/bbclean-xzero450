#INSTALL_FILES = $(BIN) readme.txt
#INSTALL_IF_NEW = bbKeys.rc

set(bbKeys_RESOURCES bbKeys.rc)
set(bbKeys_SOURCES
	bbKeys.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
add_library(bbKeys SHARED ${bbKeys_SOURCES})
target_link_libraries(bbKeys blackbox)


