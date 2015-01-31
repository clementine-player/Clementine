macro(lastfm_add_test test_class)
    include_directories(${QT_INCLUDES} "${PROJECT_SOURCE_DIR}/src" ${CMAKE_CURRENT_BINARY_DIR})

    set(LASTFM_TEST_CLASS ${test_class})
    configure_file(main.cpp.in Test${LASTFM_TEST_CLASS}.cpp)
    configure_file(Test${LASTFM_TEST_CLASS}.h Test${LASTFM_TEST_CLASS}.h)
    add_executable(${LASTFM_TEST_CLASS}Test Test${LASTFM_TEST_CLASS}.cpp)

    qt5_use_modules(${LASTFM_TEST_CLASS}Test Core Test Xml Network)

    target_link_libraries(${LASTFM_TEST_CLASS}Test
        ${LASTFM_LIB_TARGET_NAME}
        ${QT_QTTEST_LIBRARY}
        ${QT_QTCORE_LIBRARY}
    )

    add_test(NAME ${LASTFM_TEST_CLASS}Test COMMAND ${LASTFM_TEST_CLASS}Test)
endmacro()
