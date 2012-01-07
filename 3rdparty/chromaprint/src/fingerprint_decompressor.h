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

#ifndef CHROMAPRINT_FINGERPRINT_DECOMPRESSOR_H_
#define CHROMAPRINT_FINGERPRINT_DECOMPRESSOR_H_

#include <stdint.h>
#include <vector>
#include <string>
#include "bit_string_reader.h"

namespace Chromaprint
{
	class FingerprintDecompressor
	{
	public:
		FingerprintDecompressor();
		std::vector<int32_t> Decompress(const std::string &fingerprint, int *algorithm = 0);

	private:

		void ReadNormalBits(BitStringReader *reader);
		void ReadExceptionBits(BitStringReader *reader);
		void UnpackBits();

		std::vector<int32_t> m_result;
		std::vector<char> m_bits;
	};

	inline std::vector<int32_t> DecompressFingerprint(const std::string &data, int *algorithm = 0)
	{
		FingerprintDecompressor decompressor;
		return decompressor.Decompress(data, algorithm);
	}

};

#endif

