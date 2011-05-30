set(SPMEDIAKEY-SOURCES
  SPMediaKeyTap.m
  SPInvocationGrabbing/NSObject+SPInvocationGrabbing.m
)

set(SPMEDIAKEY-HEADERS
  SPMediaKeyTap.h
  SPInvocationGrabbing/NSObject+SPInvocationGrabbing.h
)

ADD_LIBRARY(SPMediaKeyTap STATIC
  ${SPMEDIAKEY-SOURCES}
)
