/*
 * Chromaprint -- Audio fingerprinting toolkit
 * Copyright (C) 2010  Lukas Lalinsky <lalinsky@gmail.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <algorithm>
#include <assert.h>
#include "base64.h"
#include "bit_string_writer.h"
#include "debug.h"

using namespace std;
using namespace Chromaprint;

static const char kBase64Chars[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static const char kBase64CharsReversed[128] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 52,
	53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
	25, 0, 0, 0, 0, 63, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
	38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0, 0, 0, 0, 0
};

string Chromaprint::Base64Encode(const string &orig)
{
	int size = orig.size();
	int encoded_size = (size * 4 + 2) / 3;
	string encoded(encoded_size, '\x00');
	const unsigned char *src = (unsigned char *)orig.data();
	string::iterator dest = encoded.begin();
	while (size > 0) {
		*dest++ = kBase64Chars[(src[0] >> 2)];
		*dest++ = kBase64Chars[((src[0] << 4) | (--size ? (src[1] >> 4) : 0)) & 63];
		if (size) {
			*dest++ = kBase64Chars[((src[1] << 2) | (--size ? (src[2] >> 6) : 0)) & 63];
			if (size) {
				*dest++ = kBase64Chars[src[2] & 63];
				--size;
			}
		}
		src += 3;
	}
	return encoded;
}

string Chromaprint::Base64Decode(const string &encoded)
{
	string str((3 * encoded.size()) / 4, '\x00');
	const unsigned char *src = (const unsigned char *)encoded.data();
	int size = encoded.size();
	string::iterator dest = str.begin();
	while (size > 0) {
		int b0 = kBase64CharsReversed[*src++];
		if (--size) {
			int b1 = kBase64CharsReversed[*src++];
			int r = (b0 << 2) | (b1 >> 4);
			assert(dest != str.end());
			*dest++ = r;
			if (--size) {
				int b2 = kBase64CharsReversed[*src++];
				r = ((b1 << 4) & 255) | (b2 >> 2);
				assert(dest != str.end());
				*dest++ = r;
				if (--size) {
					int b3 = kBase64CharsReversed[*src++];
					r = ((b2 << 6) & 255) | b3;
					assert(dest != str.end());
					*dest++ = r;
					--size;
				}
			}
		}
	}
	return str;
}

