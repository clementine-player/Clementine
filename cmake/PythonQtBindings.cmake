add_custom_target(pythonqt-bindings)

macro(add_pythonqt_bindings)
  parse_arguments(ADD_PYTHONQT_BINDINGS
    "NAME;HEADER;TYPESYSTEM"
    ""
    ${ARGN}
  )

  set(allheaders_filename ${CMAKE_CURRENT_SOURCE_DIR}/${ADD_PYTHONQT_BINDINGS_HEADER})
  set(typesystem_filename ${CMAKE_CURRENT_SOURCE_DIR}/${ADD_PYTHONQT_BINDINGS_TYPESYSTEM})
  set(target_name "pythonqt-bindings-${ADD_PYTHONQT_BINDINGS_NAME}")

  add_custom_target(${target_name}
    pythonqt-generator
      --output-directory=${CMAKE_CURRENT_SOURCE_DIR}
      --include-paths=${CMAKE_CURRENT_SOURCE_DIR}:${CMAKE_CURRENT_BINARY_DIR}:${QT_HEADERS_DIR}:${QT_QTCORE_INCLUDE_DIR}:${QT_QTGUI_INCLUDE_DIR}:${QT_QTNETWORK_INCLUDE_DIR}
      ${allheaders_filename}
      ${typesystem_filename}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty/pythonqt/generator
  )
  add_dependencies(pythonqt-bindings ${target_name})
endmacro(add_pythonqt_bindings)
