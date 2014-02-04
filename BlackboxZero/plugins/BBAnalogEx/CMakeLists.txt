set(BBAnalogEx_SOURCES
    bbanalogex.cpp
)

#set(BBAnalogEx_RESOURCES Resource.rc) #FIXME missing afx?.h

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(BBAnalogEx SHARED ${BBAnalogEx_SOURCES} ${BBAnalogEx_RESOURCES})
target_link_libraries(BBAnalogEx blackbox bblib bbPlugin)
target_link_libraries(BBAnalogEx version comctl32 shlwapi comdlg32 winmm gdiplus msimg32)

set_property(TARGET BBAnalogEx PROPERTY FOLDER "plugins/BBAnalogEx")

install(FILES readme.txt DESTINATION plugins/BBAnalogEx)
install(TARGETS BBAnalogEx RUNTIME DESTINATION plugins/BBAnalogEx)
#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/BBAnalogEx/${BBAnalogEx_RESOURCES}")
#	install(FILES ${BBAnalogEx_RESOURCES} DESTINATION plugins/BBAnalogEx)
#endif()

