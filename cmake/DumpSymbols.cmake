if (HAVE_BREAKPAD)
  if (NOT CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    message(SEND_ERROR "You must set CMAKE_BUILD_TYPE to RelWithDebInfo when using Breakpad")
  endif()

  add_custom_target(breakpad_symbols
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND python
            ../dist/dump_all_symbols.py
            --symbols_directory breakpad_symbols
            --dump_syms_binary "$<TARGET_FILE:dump_syms>"
            --crashreporting_hostname "${CRASHREPORTING_HOSTNAME}"
            "$<TARGET_FILE:clementine>"
    COMMENT "Dumping and uploading breakpad symbols to ${CRASHREPORTING_HOSTNAME}"
  )

  add_dependencies(breakpad_symbols clementine dump_syms)
endif()
