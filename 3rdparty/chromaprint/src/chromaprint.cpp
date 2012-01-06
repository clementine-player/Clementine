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

#include <vector>
#include <string>
#include <algorithm>
#include <chromaprint.h>
#include "fingerprinter.h"
#include "fingerprint_compressor.h"
#include "fingerprint_decompressor.h"
#include "fingerprinter_configuration.h"
#include "base64.h"

using namespace std;
using namespace Chromaprint;

extern "C" {

struct ChromaprintContextPrivate {
	int algorithm;
	Fingerprinter *fingerprinter;
	vector<int32_t> fingerprint;
};

#define STR(x) #x
#define VERSION_STR(minor, major, patch) \
	STR(major) "." STR(minor) "." STR(patch)

static const char *version_str = VERSION_STR(
	CHROMAPRINT_VERSION_MAJOR,
	CHROMAPRINT_VERSION_MINOR,
	CHROMAPRINT_VERSION_PATCH);

const char *chromaprint_get_version(void)
{
	return version_str;
}

ChromaprintContext *chromaprint_new(int algorithm)
{
	ChromaprintContextPrivate *ctx = new ChromaprintContextPrivate();
	ctx->algorithm = algorithm;
	ctx->fingerprinter = new Fingerprinter(CreateFingerprinterConfiguration(algorithm));
	return (ChromaprintContext *)ctx;
}

void chromaprint_free(ChromaprintContext *c)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *)c;
	delete ctx->fingerprinter;
	delete ctx;
}

int chromaprint_start(ChromaprintContext *c, int sample_rate, int num_channels)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *)c;
	return ctx->fingerprinter->Start(sample_rate, num_channels) ? 1 : 0;
}

int chromaprint_feed(ChromaprintContext *c, void *data, int length)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *)c;
	ctx->fingerprinter->Consume((short *)data, length);
	return 1;
}

int chromaprint_finish(ChromaprintContext *c)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *)c;
	ctx->fingerprint = ctx->fingerprinter->Finish();
	return 1;
}

int chromaprint_get_fingerprint(ChromaprintContext *c, char **data)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *)c;
	string fp = Chromaprint::Base64Encode(Chromaprint::CompressFingerprint(ctx->fingerprint, ctx->algorithm));
	*data = (char *)malloc(fp.size() + 1);
	if (!*data) {
		return 0;
	}
	copy(fp.begin(), fp.end(), *data);
	(*data)[fp.size()] = 0;
	return 1;
}

int chromaprint_get_raw_fingerprint(ChromaprintContext *c, void **data, int *size)
{
	ChromaprintContextPrivate *ctx = (ChromaprintContextPrivate *)c;
	*data = malloc(sizeof(int32_t) * ctx->fingerprint.size());
	if (!*data) {
		return 0;
	}
	*size = ctx->fingerprint.size();
	copy(ctx->fingerprint.begin(), ctx->fingerprint.end(), *((int32_t **)data));
	return 1;
}

int chromaprint_encode_fingerprint(void *fp, int size, int algorithm, void **encoded_fp, int *encoded_size, int base64)
{
	vector<int32_t> uncompressed = vector<int32_t>((int32_t *)fp, (int32_t *)fp + size);
	string compressed = Chromaprint::CompressFingerprint(uncompressed, algorithm);
	if (!base64) {
		*encoded_fp = malloc(compressed.size());
		*encoded_size = compressed.size();	
		copy(compressed.begin(), compressed.end(), (char *)*encoded_fp);
		return 1;
	}
	string encoded = Chromaprint::Base64Encode(compressed);
	*encoded_fp = malloc(encoded.size() + 1);
	*encoded_size = encoded.size();	
	copy(encoded.begin(), encoded.end(), (char *)*encoded_fp);
	((char *)*encoded_fp)[encoded.size()] = 0;
	return 1;
}

int chromaprint_decode_fingerprint(void *encoded_fp, int encoded_size, void **fp, int *size, int *algorithm, int base64)
{
	string encoded = string((char *)encoded_fp, encoded_size);
	string compressed = base64 ? Chromaprint::Base64Decode(encoded) : encoded;
	vector<int32_t> uncompressed = Chromaprint::DecompressFingerprint(compressed, algorithm);
	*fp = malloc(sizeof(int32_t) * uncompressed.size());
	*size = uncompressed.size();	
	copy(uncompressed.begin(), uncompressed.end(), (int32_t *)*fp);
	return 0;
}

void chromaprint_dealloc(void *ptr)
{
	free(ptr);
}

}
