cmake_minimum_required(VERSION 2.6)

include_directories(${LIBGPOD_INCLUDE_DIRS})
include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../src)

set(SOURCES
  generate_python_docs.cpp
)

set(RESOURCES
  generate_python_docs.qrc
)

qt4_add_resources(QRC ${RESOURCES})

add_executable(generate_python_docs EXCLUDE_FROM_ALL ${SOURCES} ${QRC})
target_link_libraries(generate_python_docs clementine_lib)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/epydoc.css
  ${CMAKE_CURRENT_BINARY_DIR}/epydoc.css
  COPYONLY
)

file(GLOB EPYDOC_PAGES ${CMAKE_CURRENT_SOURCE_DIR}/*.epydoc)
foreach(file ${EPYDOC_PAGES})
  get_filename_component(filename ${file} NAME)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${filename}
    ${CMAKE_CURRENT_BINARY_DIR}/${filename}
    COPYONLY
  )
endforeach(file)

add_custom_target(pythondocs
  generate_python_docs
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
