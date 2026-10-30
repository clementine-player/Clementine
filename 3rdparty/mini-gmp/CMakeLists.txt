add_library(mini-gmp STATIC
  mini-gmp.c
)

target_include_directories(mini-gmp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
