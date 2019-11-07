macro(optional_source TOGGLE)
  parse_arguments(OPTIONAL_SOURCE
    "SOURCES;HEADERS;UI;INCLUDE_DIRECTORIES"
    ""
    ${ARGN}
  )

  if(${TOGGLE})
    list(APPEND SOURCES ${OPTIONAL_SOURCE_SOURCES})
    list(APPEND HEADERS ${OPTIONAL_SOURCE_HEADERS})
    list(APPEND UI ${OPTIONAL_SOURCE_UI})
    include_directories(${OPTIONAL_SOURCE_INCLUDE_DIRECTORIES})
  else(${TOGGLE})
    list(APPEND OTHER_SOURCES ${OPTIONAL_SOURCE_SOURCES})
    list(APPEND OTHER_SOURCES ${OPTIONAL_SOURCE_HEADERS})

    set(_uic_sources)
    qt5_wrap_ui(_uic_sources ${OPTIONAL_SOURCE_UI})
    list(APPEND OTHER_SOURCES ${_uic_sources})
    list(APPEND OTHER_UIC_SOURCES ${_uic_sources})
  endif(${TOGGLE})
endmacro(optional_source)
