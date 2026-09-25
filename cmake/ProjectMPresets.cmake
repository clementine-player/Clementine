# Packs a directory of projectM presets into a single binary Qt resource file
# (see ProjectMVisualisation::InitProjectM). Installing one file is much
# faster than installing ~10,000 small ones, especially on Windows.
#
# add_projectm_presets_rcc(<target> <presets_dir> <output_file>)

function(_projectm_xml_escape out value)
  string(REPLACE "&" "&amp;" value "${value}")
  string(REPLACE "<" "&lt;" value "${value}")
  string(REPLACE ">" "&gt;" value "${value}")
  string(REPLACE "\"" "&quot;" value "${value}")
  set(${out} "${value}" PARENT_SCOPE)
endfunction()

function(add_projectm_presets_rcc target presets_dir output_file)
  get_filename_component(presets_dir "${presets_dir}" ABSOLUTE)

  # The .qrc can't list the presets one by one: some of their names contain
  # [ or ;, which CMake lists can't hold. Instead it has one entry per
  # directory, and rcc adds the files in each one. rcc doesn't keep the paths
  # of subdirectories inside an entry though, so the entries have to be the
  # directories at the bottom of the tree.
  #
  # Escaping the brackets keeps the glob results apart while we look for the
  # directories. Their names never contain a ;.
  file(GLOB_RECURSE entries LIST_DIRECTORIES true RELATIVE "${presets_dir}"
       "${presets_dir}/*")
  string(REPLACE "[" "_LB_" entries "${entries}")
  string(REPLACE "]" "_RB_" entries "${entries}")
  set(dirs "")
  foreach(entry ${entries})
    if (IS_DIRECTORY "${presets_dir}/${entry}")
      list(APPEND dirs "${entry}")
    endif()
  endforeach()
  list(SORT dirs)

  set(qrc "<RCC>\n  <qresource prefix=\"/projectm-presets\">\n")
  foreach(dir "" ${dirs})
    set(has_subdirs FALSE)
    foreach(other ${dirs})
      string(FIND "${other}" "${dir}/" prefix_pos)
      if ("${dir}" STREQUAL "" OR prefix_pos EQUAL 0)
        set(has_subdirs TRUE)
        break()
      endif()
    endforeach()

    if (NOT has_subdirs)
      _projectm_xml_escape(alias "${dir}")
      _projectm_xml_escape(path "${presets_dir}/${dir}")
      string(APPEND qrc "    <file alias=\"${alias}\">${path}</file>\n")
    else()
      file(GLOB lost_presets "${presets_dir}/${dir}/*.milk")
      if (lost_presets)
        message(FATAL_ERROR "projectM presets in ${presets_dir}/${dir} would "
                "be left out, as it also has subdirectories")
      endif()
    endif()
  endforeach()
  string(APPEND qrc "  </qresource>\n</RCC>\n")

  # rcc's --depfile doesn't escape the # in some preset names, so Ninja would
  # repack them on every build. Instead the .qrc carries a hash of the preset
  # list, and only changes (triggering a repack) when a preset is added,
  # removed or renamed.
  string(SHA256 presets_hash "${entries}")

  set(qrc_file "${CMAKE_CURRENT_BINARY_DIR}/projectm-presets.qrc")
  file(WRITE "${qrc_file}.tmp" "<!-- Presets: ${presets_hash} -->\n${qrc}")
  configure_file("${qrc_file}.tmp" "${qrc_file}" COPYONLY)

  add_custom_command(
    OUTPUT "${output_file}"
    COMMAND Qt6::rcc --binary --compress-algo zstd
            -o "${output_file}" "${qrc_file}"
    DEPENDS "${qrc_file}"
    COMMENT "Packing projectM presets into ${output_file}"
    VERBATIM
  )
  add_custom_target(${target} ALL DEPENDS "${output_file}")
endfunction()
