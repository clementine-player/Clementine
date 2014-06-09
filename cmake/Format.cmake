add_custom_target(format-diff
  COMMAND python ${CMAKE_SOURCE_DIR}/dist/format.py)
add_custom_target(format
  COMMAND python ${CMAKE_SOURCE_DIR}/dist/format.py -i)
