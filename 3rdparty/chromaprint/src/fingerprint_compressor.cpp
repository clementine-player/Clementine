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
#include "fingerprint_compressor.h"
#include "bit_string_writer.h"
#include "debug.h"
#include "utils.h"

using namespace std;
using namespace Chromaprint;

static const int kMaxNormalValue = 7;
static const int kNormalBits = 3;
static const int kExceptionBits = 5;

FingerprintCompressor::FingerprintCompressor()
{
}

void FingerprintCompressor::ProcessSubfingerprint(uint32_t x)
{
	int bit = 1, last_bit = 0;
	while (x != 0) {
		if ((x & 1) != 0) {
			m_bits.push_back(bit - last_bit);
			last_bit = bit;
		}
		x >>= 1;
		bit++;
	}
	m_bits.push_back(0);
}

void FingerprintCompressor::WriteNormalBits()
{
	BitStringWriter writer;
	for (size_t i = 0; i < m_bits.size(); i++) {
		writer.Write(min(int(m_bits[i]), kMaxNormalValue), kNormalBits);
	}
	writer.Flush();
	m_result += writer.value();
}

void FingerprintCompressor::WriteExceptionBits()
{
	BitStringWriter writer;
	for (size_t i = 0; i < m_bits.size(); i++) {
		if (m_bits[i] >= kMaxNormalValue) {
			writer.Write(int(m_bits[i]) - kMaxNormalValue, kExceptionBits);
		}
	}
	writer.Flush();
	m_result += writer.value();
}

std::string FingerprintCompressor::Compress(const vector<int32_t> &data, int algorithm)
{
	if (data.size() > 0) {
		ProcessSubfingerprint(data[0]);
		for (size_t i = 1; i < data.size(); i++) {
			ProcessSubfingerprint(data[i] ^ data[i - 1]);
		}
	}
	int length = data.size();
	m_result.resize(4);
	m_result[0] = algorithm & 255;
	m_result[1] = (length >> 16) & 255;
	m_result[2] = (length >>  8) & 255;
	m_result[3] = (length      ) & 255;
	WriteNormalBits();
	WriteExceptionBits();
	return m_result;
}

