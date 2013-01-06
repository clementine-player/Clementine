cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=gnu++0x")

set(COMMON_SOURCES
  common/convert_UTF.c
  common/dwarf/bytereader.cc
  common/dwarf/dwarf2diehandler.cc
  common/dwarf/dwarf2reader.cc
  common/dwarf_cfi_to_module.cc
  common/dwarf_cu_to_module.cc
  common/dwarf_line_to_module.cc
  common/language.cc
  common/md5.cc
  common/module.cc
  common/stabs_reader.cc
  common/stabs_to_module.cc
  common/string_conversion.cc
)

set(LIBRARY_SOURCES
  client/minidump_file_writer.cc
)

set(DUMP_SYMS_SOURCES
)

if(LINUX)
  add_definitions(-DHAVE_A_OUT_H)

  list(APPEND COMMON_SOURCES
    common/linux/dump_symbols.cc
    common/linux/elf_symbols_to_module.cc
    common/linux/elfutils.cc
    common/linux/file_id.cc
    common/linux/guid_creator.cc
    common/linux/linux_libc_support.cc
    common/linux/memory_mapped_file.cc
    common/linux/safe_readlink.cc
  )

  list(APPEND LIBRARY_SOURCES
    client/linux/crash_generation/crash_generation_client.cc
    client/linux/handler/exception_handler.cc
    client/linux/handler/minidump_descriptor.cc
    client/linux/log/log.cc
    client/linux/minidump_writer/linux_dumper.cc
    client/linux/minidump_writer/linux_ptrace_dumper.cc
    client/linux/minidump_writer/minidump_writer.cc
    client/minidump_file_writer.cc
  )

  list(APPEND DUMP_SYMS_SOURCES
    tools/linux/dump_syms/dump_syms.cc
  )
elseif(APPLE)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/client/apple/Framework)
  add_definitions(-DHAVE_MACH_O_NLIST_H)

  list(APPEND COMMON_SOURCES
    common/mac/arch_utilities.cc
    common/mac/bootstrap_compat.cc
    common/mac/dump_syms.mm
    common/mac/file_id.cc
    common/mac/MachIPC.mm
    common/mac/macho_id.cc
    common/mac/macho_reader.cc
    common/mac/macho_utilities.cc
    common/mac/macho_walker.cc
    common/mac/SimpleStringDictionary.mm
    common/mac/string_utilities.cc
  )

  list(APPEND LIBRARY_SOURCES
    client/mac/crash_generation/crash_generation_client.cc
    client/mac/Framework/Breakpad.mm
    client/mac/Framework/OnDemandServer.mm
    client/mac/handler/breakpad_nlist_64.cc
    client/mac/handler/dynamic_images.cc
    client/mac/handler/exception_handler.cc
    client/mac/handler/minidump_generator.cc
    client/mac/handler/protected_memory_allocator.cc
  )

  list(APPEND DUMP_SYMS_SOURCES
    tools/mac/dump_syms/dump_syms_tool.mm
  )
endif()

add_library(libbreakpad_common STATIC ${COMMON_SOURCES})
add_library(breakpad STATIC ${LIBRARY_SOURCES})
add_executable(dump_syms ${DUMP_SYMS_SOURCES})

target_link_libraries(breakpad libbreakpad_common)
target_link_libraries(dump_syms libbreakpad_common)

if(APPLE)
  target_link_libraries(dump_syms
    /System/Library/Frameworks/Foundation.framework
  )
endif()
