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

#include "fingerprint_decompressor.h"
#include "debug.h"
#include "utils.h"

using namespace std;
using namespace Chromaprint;

static const int kMaxNormalValue = 7;
static const int kNormalBits = 3;
static const int kExceptionBits = 5;

FingerprintDecompressor::FingerprintDecompressor()
{
}

void FingerprintDecompressor::UnpackBits()
{
	int i = 0, last_bit = 0, value = 0;
	for (size_t j = 0; j < m_bits.size(); j++) {
		int bit = m_bits[j];
		if (bit == 0) {
			m_result[i] = (i > 0) ? value ^ m_result[i - 1] : value;
			value = 0;
			last_bit = 0;
			i++;
			continue;
		}
		bit += last_bit;
		last_bit = bit;
		value |= 1 << (bit - 1);
	}
}

void FingerprintDecompressor::ReadNormalBits(BitStringReader *reader)
{
	size_t i = 0;
	while (i < m_result.size()) {
		int bit = reader->Read(kNormalBits);
		if (bit == 0) {
			i++;
		}
		m_bits.push_back(bit);
	}
}

void FingerprintDecompressor::ReadExceptionBits(BitStringReader *reader)
{
	for (size_t i = 0; i < m_bits.size(); i++) {
		if (m_bits[i] == kMaxNormalValue) {
			m_bits[i] += reader->Read(kExceptionBits);
		}
	}
}

std::vector<int32_t> FingerprintDecompressor::Decompress(const string &data, int *algorithm)
{
	if (algorithm) {
		*algorithm = data[0];
	}
	int length =
		((unsigned char)(data[1]) << 16) |
		((unsigned char)(data[2]) <<  8) |
		((unsigned char)(data[3])      );

	BitStringReader reader(data);
	reader.Read(8);
	reader.Read(8);
	reader.Read(8);
	reader.Read(8);

	m_result = vector<int32_t>(length, -1);
	reader.Reset();
	ReadNormalBits(&reader);
	reader.Reset();
	ReadExceptionBits(&reader);
	UnpackBits();
	return m_result;
}

