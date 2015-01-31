/*-
 * Copyright (c) 2001-2003 Allan Saddi <allan@saddi.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ALLAN SADDI AND HIS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL ALLAN SADDI OR HIS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: sha256.h 348 2003-02-23 22:12:06Z asaddi $
 */
//
/////////// EXAMPLE /////////////////////////////////
//
// SHA256Context sha256;
// SHA256Init (&sha256);
//
// uint8_t* pBuffer = new uint8_t[SHA_BUFFER_SIZE + 7];
// // Ensure it is on a 64-bit boundary. 
// INTPTR offs;
// if ((offs = reinterpret_cast<INTPTR>(pBuffer) & 7L))
//    pBuffer += 8 - offs;
//
// unsigned int len;
//
// ifstream inFile("test.txt", ios::binary);
//
// for (;;)
// {
//    inFile.read( reinterpret_cast<char*>(pBuffer), SHA_BUFFER_SIZE );
//    len = inFile.gcount();
//
//    if ( len == 0)
//      break;
//
//    SHA256Update (&sha256, pBuffer, len);
// }
//
// uint8_t hash[SHA256_HASH_SIZE];
// SHA256Final (&sha256, hash);
//
// cout << "Hash: ";
// for (int i = 0; i < SHA256_HASH_SIZE; ++i)
//    printf ("%02x", hash[i]);
// cout << endl;


#ifndef _SHA256_H
#define _SHA256_H

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

/* Define to 1 if you have the <inttypes.h> header file. */
#ifndef WIN32
#define HAVE_INTTYPES_H 1
#endif

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#ifndef WIN32
#define HAVE_STDINT_H 1
#endif

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strerror' function. */
#ifndef WIN32
#define HAVE_STRERROR 1
#endif

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#ifndef WIN32
#define HAVE_SYS_TYPES_H 1
#endif

/* Define to 1 if you have the <unistd.h> header file. */
#ifndef WIN32
#define HAVE_UNISTD_H 1
#endif

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define as `__inline' if that's what the C compiler calls it, or to nothing
   if it is not supported. */
#ifdef WIN32
#define inline __inline
#endif

/* Define to `unsigned' if <sys/types.h> does not define. */
/* #undef size_t */

#ifdef WIN32
#define uint64_t  unsigned __int64
#define uint32_t  unsigned int 
#define uint8_t   unsigned char
#endif // WIN32

#ifdef WIN32
#define INTPTR intptr_t
#else
#define INTPTR long
#endif

#define SHA_BUFFER_SIZE 65536

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------


#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif

#define SHA256_HASH_SIZE 32

/* Hash size in 32-bit words */
#define SHA256_HASH_WORDS 8

struct _SHA256Context {
  uint64_t totalLength;
  uint32_t hash[SHA256_HASH_WORDS];
  uint32_t bufferLength;
  union {
    uint32_t words[16];
    uint8_t bytes[64];
  } buffer;
#ifdef RUNTIME_ENDIAN
  int littleEndian;
#endif /* RUNTIME_ENDIAN */
};

typedef struct _SHA256Context SHA256Context;

#ifdef __cplusplus
extern "C" {
#endif

void SHA256Init (SHA256Context *sc);
void SHA256Update (SHA256Context *sc, const void *data, uint32_t len);
void SHA256Final (SHA256Context *sc, uint8_t hash[SHA256_HASH_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* !_SHA256_H */
