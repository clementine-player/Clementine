cmake_minimum_required(VERSION 3.16)
project(airplay-poc CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network DBus Test)

set(THIRDPARTY ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty)
add_subdirectory(${THIRDPARTY}/monocypher monocypher)
add_subdirectory(${THIRDPARTY}/mini-gmp mini-gmp)

add_library(airplay STATIC
  bplist.cpp
  crypto.cpp
  srp.cpp
  tlv8.cpp
)
target_link_libraries(airplay PUBLIC Qt6::Core monocypher mini-gmp)

add_executable(airplay-poc
  audiosource.cpp
  discovery.cpp
  main.cpp
  rtspconnection.cpp
  session.cpp
)
target_link_libraries(airplay-poc airplay Qt6::Network Qt6::DBus)

add_executable(airplay-poc-tests tests.cpp)
target_link_libraries(airplay-poc-tests airplay Qt6::Test)

enable_testing()
add_test(NAME airplay-poc-tests COMMAND airplay-poc-tests)
