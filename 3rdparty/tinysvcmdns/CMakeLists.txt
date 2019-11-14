set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
add_definitions(-D_GNU_SOURCE)

set(TINYSVCMDNS-SOURCES
  mdns.c
  mdnsd.c
)

add_library(tinysvcmdns STATIC
  ${TINYSVCMDNS-SOURCES}
)

target_link_libraries(tinysvcmdns
  ws2_32
)
