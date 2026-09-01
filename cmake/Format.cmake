add_custom_target(format-diff
  COMMAND python3 ${CMAKE_SOURCE_DIR}/dist/format.py)
add_custom_target(format
  COMMAND python3 ${CMAKE_SOURCE_DIR}/dist/format.py -i)
