include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/parser)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/parser/rpp)

add_definitions(-DRXX_ALLOCATOR_INIT_0)

set(SOURCES
  abstractmetabuilder.cpp
  abstractmetalang.cpp
  asttoxml.cpp
  customtypes.cpp
  fileout.cpp
  generator.cpp
  generatorset.cpp
  generatorsetqtscript.cpp
  main.cpp
  metajava.cpp
  metaqtscriptbuilder.cpp
  metaqtscript.cpp
  parser/ast.cpp
  parser/binder.cpp
  parser/class_compiler.cpp
  parser/codemodel.cpp
  parser/codemodel_finder.cpp
  parser/compiler_utils.cpp
  parser/control.cpp
  parser/declarator_compiler.cpp
  parser/default_visitor.cpp
  parser/dumptree.cpp
  parser/lexer.cpp
  parser/list.cpp
  parser/name_compiler.cpp
  parser/parser.cpp
  parser/rpp/builtin-macros.cpp
  parser/rpp/preprocessor.cpp
  parser/smallobject.cpp
  parser/tokens.cpp
  parser/type_compiler.cpp
  parser/visitor.cpp
  prigenerator.cpp
  reporthandler.cpp
  setupgenerator.cpp
  shellgenerator.cpp
  shellheadergenerator.cpp
  shellimplgenerator.cpp
  typeparser.cpp
  typesystem.cpp
)

set(HEADERS
  fileout.h
  generator.h
  generatorset.h
  generatorsetqtscript.h
  prigenerator.h
  setupgenerator.h
  shellgenerator.h
  shellheadergenerator.h
  shellimplgenerator.h
)

set(RESOURCES
  generator.qrc
)

qt4_wrap_cpp(MOC ${HEADERS})
qt4_add_resources(QRC ${RESOURCES})

add_executable(pythonqt-generator
  EXCLUDE_FROM_ALL
  ${SOURCES}
  ${MOC}
  ${QRC}
)

target_link_libraries(pythonqt-generator
  ${QT_LIBRARIES}
)

