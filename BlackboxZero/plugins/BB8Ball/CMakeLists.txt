
set(BB8Ball_SOURCES
  BB8Ball.cpp
)

set(BB8Ball_RESOURCES Resource.rc)
#set(blackbox_RESOURCES resource.rc)
#set(CMAKE_RC_COMPILER_INIT windres)
#ENABLE_LANGUAGE(RC)

#readme.txt
#Mortar.ico
#BB8Ball.html

#add_definitions(-DUNICODE)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(BB8Ball SHARED ${BB8Ball_SOURCES})
target_link_libraries(BB8Ball blackbox bblib bbPlugin)
target_link_libraries(BB8Ball version comctl32 gdiplus winmm msimg32 shlwapi comdlg32)

set_property(TARGET BB8Ball PROPERTY FOLDER "plugins/BB8Ball")

#INSTALL_IF_NEW = BB8Ball.rc
#install(FILES changes.txt DESTINATION plugins/BB8Ball)
install(TARGETS BB8Ball RUNTIME DESTINATION plugins/BB8Ball)
#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/BB8Ball/${BB8Ball_RESOURCES}")
#	install(FILES ${BB8Ball_RESOURCES} DESTINATION plugins/BB8Ball)
#endif()

