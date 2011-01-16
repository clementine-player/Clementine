function(install_script_files scriptname)
  if(APPLE)
    foreach(file ${ARGN})
      configure_file(
        "${file}"
        "${CMAKE_BINARY_DIR}/clementine.app/Contents/Resources/scripts/${scriptname}/${file}"
        COPY_ONLY
      )
    endforeach(file)
  else(APPLE)
    install(FILES ${ARGN} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/clementine/scripts/${scriptname}/)
  endif(APPLE)
endfunction(install_script_files)

add_subdirectory(digitallyimported-radio)
