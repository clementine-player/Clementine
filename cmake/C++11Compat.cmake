# Hacky stuff to make C++11 features work with old compilers.

if (CMAKE_COMPILER_IS_GNUCC)
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
                  OUTPUT_VARIABLE GCC_VERSION)
  if (GCC_VERSION VERSION_LESS 4.7)
    add_definitions(-Doverride=)
  endif()
endif()
