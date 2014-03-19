if(NOT VREEN_INSTALL_HEADERS)
    set(INTERNAL_FLAG "INTERNAL")
endif()
if(VREEN_DEVELOPER_BUILD)
	set(DEVELOPER_FLAG "DEVELOPER")
endif()

add_subdirectory(3rdparty)
add_subdirectory(api)
