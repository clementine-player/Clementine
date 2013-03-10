if (HAVE_BREAKPAD)
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
