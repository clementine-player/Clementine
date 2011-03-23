cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

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
