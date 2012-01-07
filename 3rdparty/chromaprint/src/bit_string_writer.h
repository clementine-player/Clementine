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

#ifndef CHROMAPRINT_BIT_STRING_WRITER_H_
#define CHROMAPRINT_BIT_STRING_WRITER_H_

#include <stdint.h>
#include <vector>
#include <string>

namespace Chromaprint
{
	class Classifier;
	class Image;
	class IntegralImage;

	class BitStringWriter
	{
	public:
		BitStringWriter() : m_buffer(0), m_buffer_size(0)
		{
		}

		void Write(uint32_t x, int bits)
		{
			m_buffer |= (x << m_buffer_size);
			m_buffer_size += bits;
			while (m_buffer_size >= 8) {
				m_value.push_back(m_buffer & 255);
				m_buffer >>= 8;
				m_buffer_size -= 8;
			}
		}

		void Flush()
		{
			while (m_buffer_size > 0) {
				m_value.push_back(m_buffer & 255);
				m_buffer >>= 8;
				m_buffer_size -= 8;
			}
			m_buffer_size = 0;
		}

		std::string value() const
		{
			return m_value;
		}

	private:

		std::string m_value;
		uint32_t m_buffer;
		int m_buffer_size;
	};

};

#endif

