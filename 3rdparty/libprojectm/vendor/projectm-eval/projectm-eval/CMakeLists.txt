
set(BISON_OUTPUT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler.h
    )

set(FLEX_OUTPUT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Scanner.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Scanner.h
    )

if(FLEX_FOUND)
    # Generate lexer code
    add_custom_command(COMMAND ${FLEX_EXECUTABLE}
                       --noline
                       --prefix=prjm_eval_
                       --header-file=Scanner.h
                       -o Scanner.c
                       Scanner.l
                       WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                       OUTPUT ${FLEX_OUTPUT_FILES}
                       DEPENDS Scanner.l
                       )

    add_custom_target(GenerateScanner ALL
                      DEPENDS ${FLEX_OUTPUT_FILES}
                      )
endif()

if(BISON_FOUND)
    # Generate compiler code
    add_custom_command(COMMAND ${BISON_EXECUTABLE}
                       --defines=Compiler.h
                       -Wcounterexamples
                       --no-lines
                       -o Compiler.c
                       Compiler.y
                       WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                       OUTPUT ${BISON_OUTPUT_FILES}
                       DEPENDS Compiler.y
                       )

    add_custom_target(GenerateCompiler ALL
                      DEPENDS ${BISON_OUTPUT_FILES}
                      )
endif()

add_library(projectM_eval STATIC
            ${BISON_OUTPUT_FILES}
            ${FLEX_OUTPUT_FILES}
            CompileContext.c
            CompileContext.h
            Compiler.y
            CompilerFunctions.c
            CompilerFunctions.h
            CompilerTypes.h
            ExpressionTree.c
            ExpressionTree.h
            MemoryBuffer.c
            MemoryBuffer.h
            Scanner.l
            TreeFunctions.c
            TreeFunctions.h
            TreeVariables.c
            TreeVariables.h
            api/projectm-eval.c
            api/projectm-eval.h
            )

target_include_directories(projectM_eval
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/api>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/projectm-eval>
                           )

target_compile_definitions(projectM_eval
    PUBLIC
    PRJM_F_SIZE=${PROJECTM_EVAL_FLOAT_SIZE}
)

set_target_properties(projectM_eval PROPERTIES
                      EXPORT_NAME Eval
                      )

if(NOT NO_MATH_LIB_REQUIRED)
    target_link_libraries(projectM_eval
                          INTERFACE
                          m
                          )
endif()

add_library(projectM::Eval ALIAS projectM_eval)

if(ENABLE_PROJECTM_EVAL_INSTALL)

    install(TARGETS projectM_eval
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            EXPORT projectM-Eval
            )

    install(FILES api/projectm-eval.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/projectm-eval/
            )


    # For use from an installed package (system install, vcpkg, homebrew etc.)
    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/projectM-Eval/projectM-EvalConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY AnyNewerVersion
            )

    configure_package_config_file(projectM-EvalConfig.cmake.in
                                  "${CMAKE_CURRENT_BINARY_DIR}/projectM-Eval/projectM-EvalConfig.cmake"
                                  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/projectM-Eval"
                                  PATH_VARS CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_INCLUDEDIR
                                  )

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/projectM-Eval/projectM-EvalConfigVersion.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/projectM-Eval/projectM-EvalConfig.cmake"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/projectM-Eval"
            COMPONENT Devel
            )

    install(EXPORT projectM-Eval
            FILE projectM-EvalTargets.cmake
            NAMESPACE projectM::
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/projectM-Eval"
            COMPONENT Devel
            )

    # Install pkgconfig file
    if(UNIX)
        configure_file(install-pkgconfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/install-pkgconfig.cmake @ONLY)
        install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/install-pkgconfig.cmake
            COMPONENT Devel
        )
    endif()
endif()
