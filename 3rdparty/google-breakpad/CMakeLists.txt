cmake_minimum_required(VERSION 3.0.0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (LINUX)
  set(SOURCES
      client/linux/crash_generation/crash_generation_client.cc
      client/linux/handler/exception_handler.cc
      client/linux/minidump_writer/linux_dumper.cc
      client/linux/minidump_writer/minidump_writer.cc
      client/minidump_file_writer.cc
      common/convert_UTF.c
      common/md5.c
      common/string_conversion.cc
      common/linux/file_id.cc
      common/linux/guid_creator.cc
  )

  ADD_LIBRARY(breakpad STATIC
      ${SOURCES}
  )
endif (LINUX)

if (APPLE)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/tools/mac/dump_syms/build/Release/dump_syms
                     COMMAND xcodebuild -target dump_syms -configuration Release -sdk macosx10.5 ARCHS=i386
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools/mac/dump_syms)

  add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/client/mac/build/Release/Breakpad.Framework
                     COMMAND xcodebuild -target Breakpad -configuration Release -sdk macosx10.5 ARCHS=i386
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/client/mac)

  add_custom_target(breakpad
                    DEPENDS tools/mac/dump_syms/build/Release/dump_syms
                            client/mac/build/Release/Breakpad.Framework)
endif (APPLE)
