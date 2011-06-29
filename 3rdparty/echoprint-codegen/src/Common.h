//
//  echoprint-codegen
//  Copyright 2011 The Echo Nest Corporation. All rights reserved.
//


#ifndef COMMON_H
#define COMMON_H



#include <assert.h>
#ifndef _WIN32
    #include <sys/time.h>
#else
#include "win_funcs.h"
#include <sys/types.h>
/* for STL*/
    #ifdef max
        #undef max
    #endif
    #ifdef min
        #undef min
    #endif
#include <malloc.h>
#endif
#include <float.h>
#include <stdio.h>
#include <stdarg.h>


#ifndef NULL
#define NULL 0
#endif


// Returns the current date in seconds.  The precision is in microseconds.
static inline double now (void) {
    struct timeval tv;
    double now;
    gettimeofday (&tv, NULL);
    now = 1e-6 * tv.tv_usec + tv.tv_sec;
    return now;
}

typedef unsigned int uint;
#define NELEM(array) (sizeof(array) / sizeof(array[0]))

#ifndef _WIN32
#define EN_ARRAY(type,var,size) type var[size]
#else
#define EN_ARRAY(type,var,size) type* var = (type*) _alloca((size)*sizeof(type))
#endif

#endif
