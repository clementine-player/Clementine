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

#ifndef CHROMAPRINT_BIT_STRING_READER_H_
#define CHROMAPRINT_BIT_STRING_READER_H_

#include <stdint.h>
#include <string>
#include "debug.h"

namespace Chromaprint
{
	class BitStringReader
	{
	public:
		BitStringReader(const std::string &input) : m_value(input), m_buffer(0), m_buffer_size(0)
		{
			m_value_iter = m_value.begin();
		}

		uint32_t Read(int bits)
		{
			if (m_buffer_size < bits) {
				if (m_value_iter != m_value.end()) {
					m_buffer |= (unsigned char)(*m_value_iter++) << m_buffer_size;
					m_buffer_size += 8;
				}
			}
			uint32_t result = m_buffer & ((1 << bits) - 1);
			m_buffer >>= bits;
			m_buffer_size -= bits;
			return result;
		}

		void Reset()
		{
			m_buffer = 0;
			m_buffer_size = 0;
		}

	private:

		std::string m_value;
		std::string::iterator m_value_iter;
		uint32_t m_buffer;
		int m_buffer_size;
	};

};

#endif

