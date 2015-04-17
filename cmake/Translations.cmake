cmake_minimum_required(VERSION 2.6)

set (XGETTEXT_OPTIONS --qt --keyword=tr:1,2c --keyword=tr --flag=tr:1:pass-c-format --flag=tr:1:pass-qt-format
    --keyword=trUtf8 --flag=tr:1:pass-c-format --flag=tr:1:pass-qt-format
    --keyword=translate:2,3c --keyword=translate:2 --flag=translate:2:pass-c-format --flag=translate:2:pass-qt-format
    --keyword=QT_TR_NOOP --flag=QT_TR_NOOP:1:pass-c-format --flag=QT_TR_NOOP:1:pass-qt-format
    --keyword=QT_TRANSLATE_NOOP:2 --flag=QT_TRANSLATE_NOOP:2:pass-c-format --flag=QT_TRANSLATE_NOOP:2:pass-qt-format
    --keyword=_ --flag=_:1:pass-c-format --flag=_:1:pass-qt-format
    --keyword=N_ --flag=N_:1:pass-c-format --flag=N_:1:pass-qt-format
    --from-code=utf-8)

macro(add_pot outfiles header pot)
  # Make relative filenames for all source files
  set(add_pot_sources)
  foreach(_filename ${ARGN})
    get_filename_component(_absolute_filename ${_filename} ABSOLUTE)
    file(RELATIVE_PATH _relative_filename ${CMAKE_CURRENT_SOURCE_DIR} ${_absolute_filename})
    list(APPEND add_pot_sources ${_relative_filename})
  endforeach(_filename)

  # Generate the .pot
  add_custom_command(
    OUTPUT ${pot}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE}
        ${XGETTEXT_OPTIONS} -s -C --omit-header
        --output=${CMAKE_CURRENT_BINARY_DIR}/pot.temp
        ${add_pot_sources}
    COMMAND cat ${header} ${CMAKE_CURRENT_BINARY_DIR}/pot.temp > ${pot}
    DEPENDS ${add_pot_sources} ${header}
  )

  list(APPEND ${outfiles} ${pot})
endmacro(add_pot)

# Syntax is:
#   add_po(sources_var po_prefix
#     LANGUAGES language1 language2 ...
#     DIRECTORY dir
#     
macro(add_po outfiles po_prefix)
  parse_arguments(ADD_PO
    "LANGUAGES;DIRECTORY"
    ""
    ${ARGN}
  )

  foreach (_lang ${ADD_PO_LANGUAGES})
    set(_po_filename "${_lang}.po")
    set(_po_filepath "${CMAKE_CURRENT_SOURCE_DIR}/${ADD_PO_DIRECTORY}/${_po_filename}")
    set(_qm_filename "clementine_${_lang}.qm")
    set(_qm_filepath "${CMAKE_CURRENT_BINARY_DIR}/${ADD_PO_DIRECTORY}/${_qm_filename}")

    # Convert the .po files to .qm files
    add_custom_command(
      OUTPUT ${_qm_filepath}
      COMMAND ${QT_LCONVERT_EXECUTABLE} ARGS ${_po_filepath} -o ${_qm_filepath} -of qm
      DEPENDS ${_po_filepath} ${_po_filepath}
    )

    list(APPEND ${outfiles} ${_qm_filepath})
  endforeach (_lang)

  # Generate a qrc file for the translations
  set(_qrc ${CMAKE_CURRENT_BINARY_DIR}/${ADD_PO_DIRECTORY}/translations.qrc)
  file(WRITE ${_qrc} "<RCC><qresource prefix=\"/${ADD_PO_DIRECTORY}\">")
  foreach(_lang ${ADD_PO_LANGUAGES})
    file(APPEND ${_qrc} "<file>${po_prefix}${_lang}.qm</file>")
  endforeach(_lang)
  file(APPEND ${_qrc} "</qresource></RCC>")
  qt5_add_resources(${outfiles} ${_qrc})
endmacro(add_po)
