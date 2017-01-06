include(CompilerUtils)
include(QtBundleUtils)
include(MocUtils)

set(CMAKE_AUTOMOC true)

macro(LIST_CONTAINS var value)
set(${var})
foreach(value2 ${ARGN})
    if(${value} STREQUAL ${value2})
	set(${var} TRUE)
    endif(${value} STREQUAL ${value2})
endforeach(value2)
endmacro(LIST_CONTAINS)

macro(LIST_FILTER list regex output)
    unset(${output})
    foreach(item ${list})
	string(REGEX MATCH ${regex} item2 ${item})
	if(item MATCHES ${regex})
            list(APPEND ${output} ${item})
	endif()
    endforeach()
endmacro()

macro(PARSE_ARGUMENTS prefix arg_names option_names)
    set(DEFAULT_ARGS)
    foreach(arg_name ${arg_names})
	set(${prefix}_${arg_name})
    endforeach(arg_name)
    foreach(option ${option_names})
	set(${prefix}_${option} FALSE)
    endforeach(option)

    set(current_arg_name DEFAULT_ARGS)
    set(current_arg_list)
    foreach(arg ${ARGN})
	list_contains(is_arg_name ${arg} ${arg_names})
	if(is_arg_name)
	    set(${prefix}_${current_arg_name} ${current_arg_list})
	    set(current_arg_name ${arg})
	    set(current_arg_list)
	else(is_arg_name)
	    list_contains(is_option ${arg} ${option_names})
	    if(is_option)
		set(${prefix}_${arg} TRUE)
	    else(is_option)
		set(current_arg_list ${current_arg_list} ${arg})
	    endif(is_option)
	endif(is_arg_name)
    endforeach(arg)
    SET(${prefix}_${current_arg_name} ${current_arg_list})
endmacro(PARSE_ARGUMENTS)

macro(qt_use_modules target)
    if(USE_QT5)
	find_package(Qt5Core QUIET)
    endif()
    if(Qt5Core_DIR)
	add_definitions("-DQT_DISABLE_DEPRECATED_BEFORE=0")
	qt5_use_modules(${target} ${ARGN})
    else()
	foreach(_module ${ARGN})
	    list(APPEND _modules Qt${_module})
	endforeach()
	find_package(Qt4 COMPONENTS ${_modules} QUIET)
        include(UseQt4)
	target_link_libraries(${target} ${QT_LIBRARIES})
        set(${target}_libraries ${QT_LIBRARIES})
    endif()
endmacro()

macro(UPDATE_COMPILER_FLAGS target)
    parse_arguments(FLAGS
        ""
        "DEVELOPER;CXX11"
        ${ARGN}
    )

    if(FLAGS_DEVELOPER)
        if(MSVC)
            update_cxx_compiler_flag(${target} "/W3" W3)
            update_cxx_compiler_flag(${target} "/WX" WX)
        else()
            update_cxx_compiler_flag(${target} "-Wall" WALL)
            update_cxx_compiler_flag(${target} "-Wextra" WEXTRA)
            update_cxx_compiler_flag(${target} "-Wnon-virtual-dtor" WDTOR)
            update_cxx_compiler_flag(${target} "-Werror" WERROR)
			#update_cxx_compiler_flag(${target} "-Wdocumentation" WERROR)
        endif()
    endif()

    if(FLAGS_CXX11)
        update_cxx_compiler_flag(${target} "-std=c++0x" CXX_11)
        #update_cxx_compiler_flag(${target} "-stdlib=libc++" STD_LIBCXX)
        #add check for c++11 support
    endif()

    get_target_property(${target}_TYPE ${target} TYPE)
    if(${target}_TYPE STREQUAL "STATIC_LIBRARY" AND NOT WIN32)
        update_cxx_compiler_flag(${target} "-fpic" PIC)
    elseif(${target}_TYPE STREQUAL "SHARED_LIBRARY")
        update_cxx_compiler_flag(${target} "-fvisibility=hidden" HIDDEN_VISIBILITY)
    endif()
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS "${COMPILER_FLAGS}")
endmacro()

function(__GET_SOURCES name)
    list(LENGTH ARGV _len)
    if(_len GREATER 1)
        list(GET ARGV 1 sourceDir)
    endif()
    if(NOT DEFINED sourceDir)
        set(sourceDir ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
    #Search for source and headers in source directory
    file(GLOB_RECURSE HDR "${sourceDir}/*.h")
    file(GLOB_RECURSE CXX "${sourceDir}/*.cpp")
    file(GLOB_RECURSE CC "${sourceDir}/*.c")
    file(GLOB_RECURSE FORMS "${sourceDir}/*.ui")
    file(GLOB_RECURSE QRC "${sourceDir}/*.qrc")
    if(APPLE)
        file(GLOB_RECURSE MM "${sourceDir}/*.mm")
    endif()

    list(APPEND sources
        ${CXX}
        ${CC}
        ${MM}
        ${HDR}
	${FORMS}
	${QRC}
    )
    set(${name} ${sources} PARENT_SCOPE)
endfunction()

function(__CHECK_SOURCE_FILES name)
    list_filter("${ARGV}" ".*\\\\.h" HDR)
    list_filter("${ARGV}" ".*\\\\.cpp" CXX)
    list_filter("${ARGV}" ".*\\\\.cc" CC)
    list_filter("${ARGV}" ".*\\\\.mm" MM)
    list_filter("${ARGV}" ".*\\\\.ui" FORMS)
    list_filter("${ARGV}" ".*\\\\.qrc" QRC)

    if(USE_QT5)
	find_package(Qt5Core QUIET)
	find_package(Qt5Widgets QUIET)
    else()
	find_package(Qt4 COMPONENTS QtCore QUIET REQUIRED)
    endif()

    if(Qt5Core_DIR)
	qt5_add_resources(${name}_QRC ${QRC})
	qt5_wrap_ui(${name}_HDR ${FORMS})
    else()
	qt4_add_resources(${name}_QRC ${QRC})
	qt4_wrap_ui(${name}_HDR ${FORMS})
    endif()

    set(__sources "")
    list(APPEND _extra_sources
	${CXX}
	${CC}
	${MM}
	${HDR}
	${FORMS}
	${${name}_QRC}
	${${name}_HDR}
    )
    set(${name} ${_extra_sources} PARENT_SCOPE)
endfunction()

macro(ADD_SIMPLE_LIBRARY target)
    parse_arguments(LIBRARY
	"LIBRARIES;INCLUDES;DEFINES;VERSION;SOVERSION;DEFINE_SYMBOL;SOURCE_DIR;SOURCE_FILES;INCLUDE_DIR;PKGCONFIG_TEMPLATE;QT"
        "STATIC;INTERNAL;DEVELOPER;CXX11;NO_FRAMEWORK"
        ${ARGN}
    )

    set(FRAMEWORK FALSE)
    if(APPLE AND NOT LIBRARY_NO_FRAMEWORK)
        set(FRAMEWORK TRUE)
    endif()

    if(LIBRARY_STATIC)
        set(type STATIC)
        set(FRAMEWORK FALSE)
    else()
        set(type SHARED)
    endif()

    if(LIBRARY_DEVELOPER)
        list(APPEND opts DEVELOPER)
    endif()
    if(LIBRARY_CXX11)
        list(APPEND opts CXX11)
    endif()

    if(NOT LIBRARY_SOURCE_DIR)
        set(LIBRARY_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    if(NOT LIBRARY_SOURCE_DIR STREQUAL "NONE")
        __get_sources(LIBRARY_SOURCES ${LIBRARY_SOURCE_DIR})
    endif()

    set(LIBRARY_EXTRA_SOURCES "")
    __check_source_files(LIBRARY_EXTRA_SOURCES ${LIBRARY_SOURCE_FILES})

    # This project will generate library
    add_library(${target} ${type} ${LIBRARY_SOURCES} ${LIBRARY_EXTRA_SOURCES})
    foreach(_define ${LIBRARY_DEFINES})
        add_definitions(-D${_define})
    endforeach()

    include_directories(${CMAKE_CURRENT_BINARY_DIR}
        ${LIBRARY_SOURCE_DIR}
	${LIBRARY_INCLUDES}
    )
    update_compiler_flags(${target} ${opts})
    qt_use_modules(${target} ${LIBRARY_QT})
    set_target_properties(${target} PROPERTIES
        VERSION ${LIBRARY_VERSION}
        SOVERSION ${LIBRARY_SOVERSION}
        DEFINE_SYMBOL ${LIBRARY_DEFINE_SYMBOL}
        FRAMEWORK ${FRAMEWORK}
    )

    target_link_libraries(${target}
	${QT_LIBRARIES}
        ${LIBRARY_LIBRARIES}
    )

    #TODO add framework creation ability
    if(LIBRARY_INCLUDE_DIR)
        set(INCNAME ${LIBRARY_INCLUDE_DIR})
    else()
        set(INCNAME ${target})
    endif()

    file(GLOB_RECURSE PUBLIC_HEADERS "${LIBRARY_SOURCE_DIR}/*[^p].h")
    file(GLOB_RECURSE PRIVATE_HEADERS "${LIBRARY_SOURCE_DIR}/*_p.h")

    generate_include_headers("include/${INCNAME}" ${PUBLIC_HEADERS})
    generate_include_headers("include/${INCNAME}/${LIBRARY_VERSION}/${INCNAME}/private/" ${PRIVATE_HEADERS})
    if(FRAMEWORK)
	set_source_files_properties(${PUBLIC_HEADERS}
	    PROPERTIES MACOSX_PACKAGE_LOCATION Headers)
	set_source_files_properties(${PRIVATE_HEADERS}
	    PROPERTIES MACOSX_PACKAGE_LOCATION Headers/${LIBRARY_VERSION}/${INCNAME}/private/)
    endif()

    if(NOT LIBRARY_INTERNAL)
        if(NOT FRAMEWORK)
            install(FILES ${PUBLIC_HEADERS} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/${INCNAME}")
            install(FILES ${PRIVATE_HEADERS} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/${INCNAME}/${LIBRARY_VERSION}/${INCNAME}/private/")
        endif()
        if(LIBRARY_PKGCONFIG_TEMPLATE)
            string(TOUPPER ${target} _target)
            if(FRAMEWORK)
                set(${_target}_PKG_LIBS "-L${LIB_DESTINATION} -f${target}")
                set(${_target}_PKG_INCDIR "${LIB_DESTINATION}/${target}.framework/Contents/Headers")
            else()
                set(${_target}_PKG_LIBS "-L${LIB_DESTINATION} -l${target}")
                set(${_target}_PKG_INCDIR "${CMAKE_INSTALL_PREFIX}/include/${INCNAME}")
            endif()
            add_pkgconfig_file(${LIBRARY_PKGCONFIG_TEMPLATE})
        endif()
    endif()
    if(type STREQUAL "SHARED" OR NOT LIBRARY_INTERNAL)
        install(TARGETS ${target}
            RUNTIME DESTINATION ${RLIBDIR}
            LIBRARY DESTINATION ${LIBDIR}
            ARCHIVE DESTINATION ${LIBDIR}
            FRAMEWORK DESTINATION ${LIBDIR}
        )
    endif()
    string(TOLOWER ${type} _type)
    message(STATUS "Added ${_type} library ${target}")
endmacro()

macro(ADD_SIMPLE_EXECUTABLE target)
    parse_arguments(EXECUTABLE
	"LIBRARIES;INCLUDES;DEFINES;SOURCE_DIR;SOURCE_FILES;QT"
        "INTERNAL;GUI;CXX11"
        ${ARGN}
    )

    if(EXECUTABLE_GUI)
        if(APPLE)
            set(type MACOSX_BUNDLE)
        else()
            set(type WIN32)
        endif()
    else()
        set(type "")
    endif()

    if(NOT EXECUTABLE_SOURCE_DIR)
	set(EXECUTABLE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    set(EXECUTABLE_EXTRA_SOURCES "")
    __get_sources(EXECUTABLE_EXTRA_SOURCES ${EXECUTABLE_SOURCE_DIR})
    __check_source_files(SOURCES "${EXECUTABLE_EXTRA_SOURCES};${EXECUTABLE_SOURCE_FILES}")

    # This project will generate library
    add_executable(${target} ${type} ${SOURCES})
    foreach(_define ${EXECUTABLE_DEFINES})
        add_definitions(-D${_define})
    endforeach()

    include_directories(${CMAKE_CURRENT_BINARY_DIR}
        .
        ${EXECUTABLE_INCLUDES}
    )

    if(EXECUTABLE_CXX11)
        list(APPEND opts CXX11)
    endif()

    update_compiler_flags(${target} ${opts})
    qt_use_modules(${target} ${EXECUTABLE_QT})

    target_link_libraries(${target}
        ${EXECUTABLE_LIBRARIES}
	${QT_LIBRARIES}
    )

    if(NOT EXECUTABLE_INTERNAL)
        install(TARGETS ${target}
            RUNTIME DESTINATION ${BINDIR}
            BUNDLE DESTINATION .
        )
    endif()
    message(STATUS "Added executable: ${target}")
endmacro()

macro(ADD_QML_DIR _qmldir)
    parse_arguments(QMLDIR
        "URI;VERSION;IMPORTS_DIR"
        ""
        ${ARGN}
    )
    if(NOT QMLDIR_IMPORTS_DIR)
        set(QMLDIR_IMPORTS_DIR "${QT_IMPORTS_DIR}")
    endif()

    string(REPLACE "." "/" _URI ${QMLDIR_URI})
    message(STATUS "Added qmldir: ${_qmldir} with uri ${QMLDIR_URI}")
    set(QML_DIR_DESTINATION "${QMLDIR_IMPORTS_DIR}/${_URI}")
    deploy_folder("${_qmldir}"
        DESTINATION "${QML_DIR_DESTINATION}"
        DESCRIPTION "qmldir with uri ${QMLDIR_URI}")
endmacro()

macro(ADD_QML_MODULE target)
    parse_arguments(MODULE
	"LIBRARIES;INCLUDES;DEFINES;URI;QML_DIR;VERSION;SOURCE_DIR;SOURCE_FILES;IMPORTS_DIR;PLUGIN_DIR;QT"
        "CXX11"
        ${ARGN}
    )

    if(NOT MODULE_IMPORTS_DIR)
	set(MODULE_IMPORTS_DIR "${QT_IMPORTS_DIR}")
    endif()
    if(MODULE_QML_DIR)
        add_qml_dir("${MODULE_QML_DIR}"
            URI "${MODULE_URI}"
            VERSION "${MODULE_VERSION}"
            IMPORTS_DIR "${MODULE_IMPORTS_DIR}"
        )
    endif()

    __get_sources(SOURCES ${MODULE_SOURCE_DIR})
    set(MODULE_EXTRA_SOURCES "")

    __check_source_files(MODULE_EXTRA_SOURCES ${MODULE_SOURCE_FILES})
    list(APPEND SOURCES ${MODULE_EXTRA_SOURCES})
    # This project will generate library
    add_library(${target} SHARED ${SOURCES})
    foreach(_define ${MODULE_DEFINES})
        add_definitions(-D${_define})
    endforeach()

    include_directories(${CMAKE_CURRENT_BINARY_DIR}
        ${MODULE_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${MODULE_INCLUDES}
    )

    qt_use_modules(${target} ${MODULE_QT})
    target_link_libraries(${target}
        ${${target}_libraries}
        ${MODULE_LIBRARIES}
    )

    if(MODULE_CXX11)
        list(APPEND opts CXX11)
    endif()
    update_compiler_flags(${target} ${opts})
    message(STATUS "Added qml module: ${target} with uri ${MODULE_URI}")
    string(REPLACE "." "/" _URI ${MODULE_URI})
    install(TARGETS ${target} DESTINATION "${MODULE_IMPORTS_DIR}/${_URI}/${MODULE_PLUGIN_DIR}")
endmacro()

macro(ADD_SIMPLE_QT_TEST target)
    parse_arguments(TEST
	"LIBRARIES;RESOURCES;SOURCES;QT"
        "CXX11"
        ${ARGN}
    )

    if(TEST_SOURCES)
        set(${target}_SRC ${TEST_SOURCES})
    else()
        set(${target}_SRC ${target}.cpp)
    endif()
    list(APPEND TEST_QT Test)

    list(APPEND ${target}_SRC ${TEST_RESOURCES})
    __check_source_files(${target}_SRC ${${target}_SRC})
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    add_executable(${target} ${${target}_SRC})
    qt_use_modules(${target} ${TEST_QT})
    target_link_libraries(${target} ${TEST_LIBRARIES} ${QT_LIBRARIES})
    if(TEST_CXX11)
        list(APPEND opts CXX11)
    endif()
    update_compiler_flags(${target} ${opts})
    add_test(NAME ${target} COMMAND ${target})
    message(STATUS "Added simple test: ${target}")
endmacro()

macro(APPEND_TARGET_LOCATION target list)
    get_target_property(${target}_LOCATION ${target} LOCATION)
    list(APPEND ${list} ${${target}_LOCATION})
endmacro()

macro(CHECK_DIRECTORY_EXIST directory exists)
    if(EXISTS ${directory})
        set(_exists FOUND)
    else()
        set(_exists NOT_FOUND)
    endif()
    set(exists ${_exists})
endmacro()

macro(CHECK_QML_MODULE name exists)
    check_directory_exist("${QT_IMPORTS_DIR}/${name}" _exists)
    message(STATUS "Checking qml module ${name} - ${_exists}")
    set(${exists} ${_exists})
endmacro()

macro(ADD_PUBLIC_HEADER header dir)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${header}
        ${CMAKE_CURRENT_BINARY_DIR}/include/${dir}/${header} COPYONLY)
    list(APPEND PUBLIC_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/include/${dir}/${header})
endmacro()

macro(GENERATE_PUBLIC_HEADER header dir name)
    add_public_header(${header})
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/include/${dir}/${name}
        "#include \"${name}\"\n"
    )
    list(APPEND PUBLIC_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/include/${name})
endmacro()

macro(GENERATE_INCLUDE_HEADERS _dir)
    include_directories(${PROJECT_BINARY_DIR})
    foreach(header ${ARGN})
        get_filename_component(_basename ${header} NAME_WE)
        get_filename_component(_abs_FILE ${header} ABSOLUTE)
        #message(STATUS "${PROJECT_BINARY_DIR}/${_dir}/${_basename}.h")

        if(NOT EXISTS "${PROJECT_BINARY_DIR}/${_dir}/${_basename}.h" )
            file(WRITE "${PROJECT_BINARY_DIR}/${_dir}/${_basename}.h"
            "#include \"${_abs_FILE}\"\n"
                )
        endif()
    endforeach()
endmacro()

macro(ADD_CUSTOM_DIRECTORY sourceDir)
    parse_arguments(DIR
        "DESCRIPTION"
        ""
        ${ARGN}
    )

    string(RANDOM tail)
    get_filename_component(_basename ${sourceDir} NAME_WE)
    file(GLOB_RECURSE _files "${sourceDir}/*")
    add_custom_target(dir_${_basename}_${tail} ALL
        SOURCES ${_files}
    )
    if(DIR_DESCRIPTION)
	source_group(${DIR_DESCRIPTION} FILES ${_files})
    endif()
endmacro()

macro(DEPLOY_FOLDER sourceDir)
    parse_arguments(FOLDER
        "DESCRIPTION;DESTINATION"
        ""
        ${ARGN}
    )

    get_filename_component(_basename ${sourceDir} NAME_WE)
    get_filename_component(_destname ${FOLDER_DESTINATION} NAME_WE)
    file(GLOB_RECURSE _files "${sourceDir}/*")
    message(STATUS "deploy folder: ${sourceDir}")
    add_custom_target(qml_${_destname} ALL
        SOURCES ${_files}
    )
    file(GLOB _files "${sourceDir}/*")
    foreach(_file ${_files})
        if(IS_DIRECTORY ${_file})
	    install(DIRECTORY ${_file} DESTINATION  ${FOLDER_DESTINATION})
	    get_filename_component(_name ${_file} NAME_WE)
	else()
	    install(FILES ${_file} DESTINATION ${FOLDER_DESTINATION})
        endif()
    endforeach()
    if(FOLDER_DESCRIPTION)
	source_group(${FOLDER_DESCRIPTION} FILES ${_files})
    endif()
endmacro()

macro(ENABLE_QML_DEBUG_SUPPORT target)

endmacro()

macro(ADD_PKGCONFIG_FILE file)
    #add pkgconfig file
    get_filename_component(_name ${file} NAME_WE)
    set(PKGCONFIG_DESTINATION ${CMAKE_INSTALL_PREFIX}/${LIBDIR}/pkgconfig)
    configure_file(${file}
        ${CMAKE_CURRENT_BINARY_DIR}/${_name}.pc
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_name}.pc
        DESTINATION ${PKGCONFIG_DESTINATION}
    )
endmacro()

macro(FIND_MODULE module)
    parse_arguments(MODULE
        "PREFIX;LIBRARY_HINTS;HEADERS_DIR;INCLUDE_HINTS;GLOBAL_HEADER"
        "NO_PKGCONFIG;NO_MACOSX_FRAMEWORK"
        ${ARGN}
    )
    string(TOUPPER ${module} _name)
    if(MODULE_PREFIX)
        set(_name "${MODULE_PREFIX}_${_name}")
    endif()

    #try to use pkgconfig
    if(NOT MODULE_NO_PKGCONFIG)
        find_package(PkgConfig)
        pkg_check_modules(${_name} ${module})
        if(${_name}_FOUND)
            list(GET ${_name}_INCLUDE_DIRS 0 ${_name}_INCLUDE_DIR)
            list(GET ${_name}_LIBRARIES 0 ${_name}_LIBRARIES)
        endif()
        #message("${_name} + ${${_name}_LIBRARIES} AND ${${_name}_INCLUDE_DIR}")
    endif()
    #try to find macosx framework
    if(APPLE AND NOT MODULE_NO_MACOSX_FRAMEWORK AND NOT ${_name}_FOUND)
        message(STATUS "Try to find MacosX framework ${module}.framework")
        find_library(${_name}_LIBRARIES
                NAMES ${module}
                HINTS ${MODULE_LIBRARY_HINTS}
        )
        if(${_name}_LIBRARIES)
            set(${_name}_FOUND true)
            list(APPEND ${MODULE_PREFIX}_LIBRARIES ${${_name}_LIBRARIES})
            set(${_name}_INCLUDE_DIR "${${MODULE_PREFIX}_LIBRARIES}/Headers/")
            list(APPEND ${MODULE_PREFIX}_INCLUDES
                    ${${_name}_INCLUDE_DIR}
            )
        endif()
    endif()
    #try to use simple find
    if(NOT ${_name}_FOUND)
        message(STATUS "Try to find ${module}")
        include(FindLibraryWithDebug)
        find_path(${_name}_INCLUDE_DIR ${MODULE_GLOBAL_HEADER}
            HINTS ${MODULE_INCLUDE_HINTS}
            PATH_SUFFIXES ${MODULE_HEADERS_DIR}
        )
        find_library_with_debug(${_name}_LIBRARIES
            WIN32_DEBUG_POSTFIX d
            NAMES ${module}
            HINTS ${MODULE_LIBRARY_HINTS}
            )
        #message("${MODULE_HEADERS_DIR} ${MODULE_INCLUDE_HINTS} ${QT_INCLUDE_DIR}")
    endif()

    #include(FindPackageHandleStandardArgs)
    #find_package_handle_standard_args(${_name} DEFAULT_MSG  ${${_name}_LIBRARIES} ${${_name}_INCLUDE_DIR})

    if(${_name}_LIBRARIES AND ${_name}_INCLUDE_DIR)
        message(STATUS "Found ${module}: ${${_name}_LIBRARIES}")
        set(${_name}_FOUND true)
        list(APPEND ${MODULE_PREFIX}_LIBRARIES
                ${${_name}_LIBRARIES}
        )
        list(APPEND ${MODULE_PREFIX}_INCLUDES
                ${${_name}_INCLUDE_DIR}
        )
        #message("${${_name}_LIBRARIES} \n ${${_name}_INCLUDE_DIR}")
    else(${_name}_LIBRARIES AND ${_name}_INCLUDE_DIR)
        message(STATUS "Could NOT find ${module}")
    endif()
    mark_as_advanced(${${_name}_INCLUDE_DIR} ${${_name}_LIBRARIES})
endmacro()

macro(FIND_QT_MODULE module)
    parse_arguments(MODULE
        "HEADERS_DIR;GLOBAL_HEADER"
        ""
        ${ARGN}
    )
    find_module(${module}
        PREFIX QT
        HEADERS_DIR ${MODULE_HEADERS_DIR}
        GLOBAL_HEADER ${MODULE_GLOBAL_HEADER}
        LIBRARY_HINTS ${QT_LIBRARY_DIR}
    )
endmacro()
