include_directories(${PROTOBUF_INCLUDE_DIRS})

# Generated next to this file's build directory. Older versions of CMake's
# FindProtobuf get the output path wrong for a .proto outside the target's
# own source directory.
add_library(cast_channel STATIC
  cast_channel.proto
)

protobuf_generate(TARGET cast_channel LANGUAGE cpp)

target_include_directories(cast_channel PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(cast_channel
  protobuf::libprotobuf
)
