#ifndef OVERRIDE_H
#define OVERRIDE_H

// Defines the OVERRIDE macro as C++11's override control keyword if
// it is available.

#ifndef __has_extension
  #define __has_extension(x) 0
#endif

#if __has_extension(cxx_override_control)  // Clang feature checking macro.
#  define OVERRIDE override
#elif defined(__GNUC__)  // Clang also defines this.
#  include <features.h>
#  if __GNUC_PREREQ(4,7)
#    define OVERRIDE override
#  else
#    define OVERRIDE
#  endif
#endif

#endif  // OVERRIDE_H
