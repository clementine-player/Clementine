add_library(monocypher STATIC
  monocypher.c
  monocypher-ed25519.c
)

target_include_directories(monocypher PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
