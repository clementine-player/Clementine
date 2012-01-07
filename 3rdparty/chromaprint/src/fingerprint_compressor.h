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

#ifndef CHROMAPRINT_FINGERPRINT_COMPRESSOR_H_
#define CHROMAPRINT_FINGERPRINT_COMPRESSOR_H_

#include <stdint.h>
#include <vector>
#include <string>

namespace Chromaprint
{
	class Classifier;
	class Image;
	class IntegralImage;

	class FingerprintCompressor
	{
	public:
		FingerprintCompressor();
		std::string Compress(const std::vector<int32_t> &fingerprint, int algorithm = 0);

	private:

		void WriteNormalBits();
		void WriteExceptionBits();
		void ProcessSubfingerprint(uint32_t);

		std::string m_result;
		std::vector<char> m_bits; 
	};

	inline std::string CompressFingerprint(const std::vector<int32_t> &data, int algorithm = 0)
	{
		FingerprintCompressor compressor;
		return compressor.Compress(data, algorithm);
	}

};

#endif

