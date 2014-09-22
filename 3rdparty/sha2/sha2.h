/*
 * FILE:	sha2.h
 * AUTHOR:	Aaron D. Gifford - http://www.aarongifford.com/
 * 
 * Copyright (c) 2000-2001, Aaron D. Gifford
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
 * 3. Neither the name of the copyright holder nor the names of contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTOR(S) ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTOR(S) BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: sha2.h,v 1.1 2001/11/08 00:02:01 adg Exp adg $
 */

#ifndef __CLEMENTINE_SHA2_H__
#define __CLEMENTINE_SHA2_H__

/*
 * Import u_intXX_t size_t type definitions from system headers.  You
 * may need to change this, or define these things yourself in this
 * file.
 */
#include <sys/types.h>

namespace clementine_sha2 {

/*** SHA-256/384/512 Various Length Definitions ***********************/
static const int SHA256_BLOCK_LENGTH = 64;
static const int SHA256_DIGEST_LENGTH = 32;
static const int SHA256_DIGEST_STRING_LENGTH = (SHA256_DIGEST_LENGTH * 2 + 1);


/*** SHA-256/384/512 Context Structures *******************************/
/* NOTE: If your architecture does not define either u_intXX_t types or
 * uintXX_t (from inttypes.h), you may need to define things by hand
 * for your system:
 */
#ifdef __MINGW32__
typedef unsigned char u_int8_t;		/* 1-byte  (8-bits)  */
typedef unsigned int u_int32_t;		/* 4-bytes (32-bits) */
typedef unsigned long long u_int64_t;	/* 8-bytes (64-bits) */
#endif

typedef struct _SHA256_CTX {
	u_int32_t	state[8];
	u_int64_t	bitcount;
	u_int8_t	buffer[SHA256_BLOCK_LENGTH];
} SHA256_CTX;


void SHA256_Init(SHA256_CTX *);
void SHA256_Update(SHA256_CTX*, const u_int8_t*, size_t);
void SHA256_Final(u_int8_t[SHA256_DIGEST_LENGTH], SHA256_CTX*);
char* SHA256_End(SHA256_CTX*, char[SHA256_DIGEST_STRING_LENGTH]);
char* SHA256_Data(const u_int8_t*, size_t, char[SHA256_DIGEST_STRING_LENGTH]);

}  // namespace clementine_sha2

#endif /* __CLEMENTINE_SHA2_H__ */
