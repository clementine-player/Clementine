cmake_minimum_required(VERSION 2.6)

set(CLEMENTINE-LANGUAGES
  cs
  da
  de
  el
  en_GB
  es
  fi
  fr
  gl
  it
  kk
  nb
  pl
  pt
  pt_BR
  ro
  ru
  sk
  sv
  tr
)


# Translations
set (XGETTEXT_OPTIONS --qt --keyword=tr --flag=tr:1:pass-c-format --flag=tr:1:pass-qt-format
    --keyword=trUtf8 --flag=tr:1:pass-c-format --flag=tr:1:pass-qt-format
    --keyword=translate:2 --flag=translate:2:pass-c-format --flag=translate:2:pass-qt-format
    --keyword=QT_TR_NOOP --flag=QT_TR_NOOP:1:pass-c-format --flag=QT_TR_NOOP:1:pass-qt-format
    --keyword=QT_TRANSLATE_NOOP:2 --flag=QT_TRANSLATE_NOOP:2:pass-c-format --flag=QT_TRANSLATE_NOOP:2:pass-qt-format
    --keyword=_ --flag=_:1:pass-c-format --flag=_:1:pass-qt-format
    --keyword=N_ --flag=N_:1:pass-c-format --flag=N_:1:pass-qt-format
    --from-code=utf-8)

# Generate the .pot
set (CLEMENTINE-POT "${CMAKE_CURRENT_SOURCE_DIR}/translations.pot")
add_custom_target(pot ALL
  COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} 
      ${XGETTEXT_OPTIONS} -C --omit-header --no-location
      --directory=${CMAKE_CURRENT_SOURCE_DIR}
      --output=${CMAKE_CURRENT_BINARY_DIR}/translations.pot
      ${CLEMENTINE-TRANSLATION-SOURCE}
  COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/header ${CMAKE_CURRENT_BINARY_DIR}/translations.pot > ${CLEMENTINE-POT}
  DEPENDS ${CLEMENTINE-TRANSLATION-DEPS}
)

# The pot must be built last - CMake's depenency scanner doesn't work well
# when the deps are in another dir
add_dependencies(pot
  clementine_analyzers
  clementine_core
  clementine_engines
  clementine_library
  clementine_playlist
  clementine_radio
  clementine_transcoder
  clementine_ui
  clementine_widgets
)

add_custom_target(po_all ALL)

# Merge the .pot into .po files
foreach (_lang ${CLEMENTINE-LANGUAGES})
  set(_po ${CMAKE_CURRENT_SOURCE_DIR}/${_lang}.po)
  add_custom_target("po_${_lang}"
    COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet -U --no-location --no-fuzzy-matching --backup=off
        ${_po} ${CLEMENTINE-POT}
    DEPENDS ${_po})
  add_dependencies("po_${_lang}" pot)
  add_dependencies(po_all "po_${_lang}")
endforeach (_lang)

# Convert the .po files to .qm files
foreach (_lang ${CLEMENTINE-LANGUAGES})
  set(_po_filename "${_lang}.po")
  set(_po_filepath "${CMAKE_CURRENT_SOURCE_DIR}/${_po_filename}")
  set(_qm_filename "clementine_${_lang}.qm")
  set(_qm_filepath "${CMAKE_CURRENT_BINARY_DIR}/${_qm_filename}")
  
  add_custom_command(OUTPUT ${_qm_filepath}
    COMMAND ${QT_LCONVERT_EXECUTABLE} ARGS ${_po_filepath} -o ${_qm_filepath} -of qm
    MAIN_DEPENDENCY ${_po_filepath}
  )
  set(CLEMENTINE-QM-FILES ${CLEMENTINE-QM-FILES} ${_qm_filepath})
endforeach (_lang)

# Generate a qrc file for the translations
set(CLEMENTINE-QM-RESOURCE ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc)
file(WRITE ${CLEMENTINE-QM-RESOURCE} "<RCC><qresource prefix=\"/translations\">")
foreach(QM-FILE ${CLEMENTINE-QM-FILES})
  file(RELATIVE_PATH QM-RELATIVE-PATH ${CMAKE_CURRENT_BINARY_DIR} ${QM-FILE})
  file(APPEND ${CLEMENTINE-QM-RESOURCE} "<file>" ${QM-RELATIVE-PATH} "</file>")
endforeach(QM-FILE)
file(APPEND ${CLEMENTINE-QM-RESOURCE} "</qresource></RCC>")
qt4_add_resources(SOURCES ${CLEMENTINE-QM-RESOURCE})

add_library(clementine_translations
  ${SOURCES}
)
