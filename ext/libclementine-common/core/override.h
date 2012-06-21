#ifndef OVERRIDE_H
#define OVERRIDE_H

// Defines the OVERRIDE macro as C++11's override control keyword if
// it is available.

#ifndef __has_extension
  #define __has_extension(x) 0
#endif

#if __has_extension(cxx_override_control)  // Clang feature checking macro.
#  define OVERRIDE override
#else
#  define OVERRIDE
#endif

#endif  // OVERRIDE_H
