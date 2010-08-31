# add an engine
macro(add_engine engine lib_list src_list moc_list supported)

  # recreate list
  set(lib_list ${lib_list})
  list(GET lib_list 0 name)

  # add a user selectable build option, supported engines enabled by default
  option(ENGINE_${name}_ENABLED "enable engine ${engine}" ${supported})

  # check for all needed libraries
  foreach(lib ${lib_list})
    if(${lib}_NOTFOUND)
      set(ENGINE_${name}_LIB_MISSING TRUE)
    endif(${lib}_NOTFOUND)
  endforeach(lib ${lib_list})

  # check if engine is enabled and needed librares are available
  if(ENGINE_${name}_ENABLED AND NOT ENGINE_${name}_LIB_MISSING)

    # add to list of unsupported engines
    set(supported ${supported})
    if(NOT supported)
      set(ENGINES_UNSUPPORTED "${ENGINES_UNSUPPORTED} ${engine}")
    endif(NOT supported)

    # add define -DHAVE_<engine> so we can clutter the code with #ifdefs
    set("HAVE_${name}" 1 CACHE INTERNAL ${name})

    # add sources and MOC headers
    list(APPEND SOURCES ${src_list})
    list(APPEND HEADERS ${moc_list})

    # add libraries to link against
    foreach(lib ${lib_list})
      set(ENGINE_LIBRARIES ${ENGINE_LIBRARIES} ${${lib}_LIBRARIES} CACHE INTERNAL libraries)
    endforeach(lib ${lib_list})

    # add to list of enabled engines
    set(ENGINES_ENABLED "${ENGINES_ENABLED} ${engine}")

  else(ENGINE_${name}_ENABLED AND NOT ENGINE_${name}_LIB_MISSING)

    # add to list of disabled engines
    set(ENGINES_DISABLED "${ENGINES_DISABLED} ${engine}")
    set("HAVE_${name}" 0 CACHE INTERNAL ${name})

  endif(ENGINE_${name}_ENABLED AND NOT ENGINE_${name}_LIB_MISSING)

endmacro(add_engine engine lib_list src_list moc_list supported)

# print engines to be built
macro(print_engines)
  # show big warning if building unsupported engines
  if(ENGINES_UNSUPPORTED)
    message("")
    message("    *********************************")
    message("    ************ WARNING ************")
    message("    *********************************")
    message("")
    message("    The following engines are NOT supported by clementine developers:")
    message("    ${ENGINES_UNSUPPORTED}")
    message("")
    message("    Don't post any bugs if you use them, fix them yourself!")
    message("")
    pig()
    message("")
  endif(ENGINES_UNSUPPORTED)
  # need at least 1 engine
  if(NOT ENGINES_ENABLED)
    message(FATAL_ERROR "no engine enabled!")
  endif(NOT ENGINES_ENABLED)
endmacro(print_engines)

# print the pig :)
macro(pig)
  file(READ ${CMAKE_SOURCE_DIR}/pig.txt pig)
  message(${pig})
endmacro(pig)
