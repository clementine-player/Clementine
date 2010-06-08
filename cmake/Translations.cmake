cmake_minimum_required(VERSION 2.6)

set (XGETTEXT_OPTIONS --qt --keyword=tr --flag=tr:1:pass-c-format --flag=tr:1:pass-qt-format
    --keyword=trUtf8 --flag=tr:1:pass-c-format --flag=tr:1:pass-qt-format
    --keyword=translate:2 --flag=translate:2:pass-c-format --flag=translate:2:pass-qt-format
    --keyword=QT_TR_NOOP --flag=QT_TR_NOOP:1:pass-c-format --flag=QT_TR_NOOP:1:pass-qt-format
    --keyword=QT_TRANSLATE_NOOP:2 --flag=QT_TRANSLATE_NOOP:2:pass-c-format --flag=QT_TRANSLATE_NOOP:2:pass-qt-format
    --keyword=_ --flag=_:1:pass-c-format --flag=_:1:pass-qt-format
    --keyword=N_ --flag=N_:1:pass-c-format --flag=N_:1:pass-qt-format
    --from-code=utf-8)

macro(add_pot header pot)
  # Generate the .pot
  add_custom_target(pot ALL
    COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE}
        ${XGETTEXT_OPTIONS} -C --omit-header --no-location
        --directory=${CMAKE_CURRENT_SOURCE_DIR}
        --output=${CMAKE_CURRENT_BINARY_DIR}/pot.temp
        ${ARGN}
    COMMAND cat ${header} ${CMAKE_CURRENT_BINARY_DIR}/pot.temp > ${pot}
    DEPENDS ${ARGN}
  )
endmacro(add_pot)

# Syntax is:
#   add_po(sources_var po_prefix
#     POT potfile
#     LANGUAGES language1 language2 ...
#     DIRECTORY dir
#     
macro(add_po outfiles po_prefix)
  parse_arguments(ADD_PO
    "POT;LANGUAGES;DIRECTORY"
    ""
    ${ARGN}
  )

  # Merge the .pot into .po files
  foreach (_lang ${ADD_PO_LANGUAGES})
    set(_po ${CMAKE_CURRENT_SOURCE_DIR}/${ADD_PO_DIRECTORY}/${_lang}.po)
    add_custom_target("po_${_lang}" ALL
      COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet -U --no-location --no-fuzzy-matching --backup=off
          ${_po} ${ADD_PO_POT}
      DEPENDS ${_po})
    add_dependencies("po_${_lang}" pot)
  endforeach (_lang)

  # Convert the .po files to .qm files
  foreach (_lang ${ADD_PO_LANGUAGES})
    set(_po_filename "${_lang}.po")
    set(_po_filepath "${CMAKE_CURRENT_SOURCE_DIR}/${ADD_PO_DIRECTORY}/${_po_filename}")
    set(_qm_filename "clementine_${_lang}.qm")
    set(_qm_filepath "${CMAKE_CURRENT_BINARY_DIR}/${ADD_PO_DIRECTORY}/${_qm_filename}")

    add_custom_command(
      OUTPUT ${_qm_filepath}
      COMMAND ${QT_LCONVERT_EXECUTABLE} ARGS ${_po_filepath} -o ${_qm_filepath} -of qm
      MAIN_DEPENDENCY ${_po_filepath}
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
  qt4_add_resources(${outfiles} ${_qrc})
endmacro(add_po)
