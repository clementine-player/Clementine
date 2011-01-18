find_path(PYQT_SIP_DIR Qt/Qtmod.sip
  PATHS /System/Library/Frameworks/Python.framework/Versions/2.6
  PATH_SUFFIXES share/sip/PyQt4
)

find_program(SIP_BINARY "sip"
  PATHS /System/Library/Frameworks/Python.framework/Versions/2.6/bin)

macro(add_sip_binding outputvar source)
  # Work out what the SIP flags should be for PyQt4.  These would normally be
  # obtained from PyQt4.pyqtconfig.Configuration().pyqt_sip_flags, but we can't
  # call that when cross-compiling.
  set(PYQT_SIP_FLAGS
    "-x" "VendorID"
    "-x" "PyQt_NoPrintRangeBug"
    "-t" "Qt_4_7_1"
  )

  if(WIN32)
    list(APPEND PYQT_SIP_FLAGS "-t" "WS_WIN")
  elseif(APPLE)
    list(APPEND PYQT_SIP_FLAGS "-t" "WS_MACX")
  else(WIN32)
    list(APPEND PYQT_SIP_FLAGS "-t" "WS_X11")
  endif(WIN32)

  get_filename_component(source_directory ${source} PATH)
  get_filename_component(source_basename ${source} NAME)
  get_filename_component(source_module ${source} NAME_WE)

  set(outputs
    "${CMAKE_CURRENT_BINARY_DIR}/sip${source_module}cmodule.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/sipAPI${source_module}.h"
    ${ARGN}
  )

  # Find any included files
  execute_process(
    COMMAND "awk" "/^%Include/ {ORS=\";\"; print \"${source_directory}/\" $2}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${source}"
    OUTPUT_VARIABLE included_files
  )
  foreach(included_file ${included_files} ${source})
    # Sip creates 1 file per class... so we have to figure out what classes
    # it will generate
    execute_process(
      COMMAND "awk" "/^\\s*(class|namespace|struct) +([A-Za-z0-9]+)/ {ORS=\";\"; print $2}"
              "${CMAKE_CURRENT_SOURCE_DIR}/${included_file}"
      OUTPUT_VARIABLE classes
    )
    foreach(class ${classes})
      list(APPEND outputs "${CMAKE_CURRENT_BINARY_DIR}/sip${source_module}${class}.cpp")
    endforeach(class)
  endforeach(included_file)

  add_custom_command(
    OUTPUT ${outputs}
    COMMAND ${SIP_BINARY} ${PYQT_SIP_FLAGS}
      "-I${PYQT_SIP_DIR}"
      "-c" "${CMAKE_CURRENT_BINARY_DIR}"
      "${CMAKE_CURRENT_SOURCE_DIR}/${source}"
    DEPENDS ${included_files} "${source}"
  )

  list(APPEND ${outputvar} ${outputs})
endmacro(add_sip_binding)
