add_executable(bsetroot WIN32
	${bsetroot_SOURCES}
	${bsetroot_HEADERS}
	${bsetroot_RESOURCES}
)

#set_target_properties(bsetroot PROPERTIES ENABLE_EXPORTS ON)
target_link_libraries(bsetroot bblib)
target_link_libraries(bsetroot version comctl32)


