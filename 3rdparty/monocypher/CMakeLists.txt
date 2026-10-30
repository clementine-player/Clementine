add_library(monocypher STATIC
  monocypher.c
)

target_include_directories(monocypher PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
