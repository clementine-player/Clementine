add_library(projectM_ns-eel2 STATIC
            ns-eel.c
            ns-eel.h
            )

target_compile_definitions(projectM_ns-eel2
                           PUBLIC
                           EEL_F_SIZE=${PROJECTM_EVAL_FLOAT_SIZE}
                           )

target_include_directories(projectM_ns-eel2
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
                           $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/projectm-eel>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/projectm-eval/ns-eel2>
                           )

target_link_libraries(projectM_ns-eel2
                      PUBLIC
                      projectM::Eval
                      )

set_target_properties(projectM_ns-eel2 PROPERTIES
                      EXPORT_NAME ns-eel2
                      )

add_library(projectM::ns-eel2 ALIAS projectM_ns-eel2)

if(ENABLE_PROJECTM_EVAL_INSTALL)

    install(TARGETS projectM_ns-eel2
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            EXPORT projectM-EvalMilkdrop
            )

    install(FILES ns-eel.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/projectm-eval/ns-eel2
            )

    # For use from an installed package (system install, vcpkg, homebrew etc.)
    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/projectM-EvalMilkdrop/projectM-EvalMilkdropConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY AnyNewerVersion
            )

    configure_package_config_file(projectM-EvalMilkdropConfig.cmake.in
                                  "${CMAKE_CURRENT_BINARY_DIR}/projectM-EvalMilkdrop/projectM-EvalMilkdropConfig.cmake"
                                  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/projectM-EvalMilkdrop"
                                  PATH_VARS CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_INCLUDEDIR
                                  )

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/projectM-EvalMilkdrop/projectM-EvalMilkdropConfigVersion.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/projectM-EvalMilkdrop/projectM-EvalMilkdropConfig.cmake"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/projectM-EvalMilkdrop"
            COMPONENT Devel
            )

    install(EXPORT projectM-EvalMilkdrop
            FILE projectM-EvalMilkdropTargets.cmake
            NAMESPACE projectM::
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/projectM-EvalMilkdrop"
            COMPONENT Devel
            )

endif()