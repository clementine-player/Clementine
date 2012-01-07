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

#ifndef CHROMAPRINT_COMBINED_BUFFER_H_
#define CHROMAPRINT_COMBINED_BUFFER_H_

#include <math.h>
#include <assert.h>
#include <algorithm>

namespace Chromaprint
{

	template<class T>
	class CombinedBuffer;

	template<class T>
	class _CombinedBufferIterator
	{
	public:
		typedef std::input_iterator_tag iterator_category;
		typedef T value_type;
		typedef int difference_type;
		typedef T* pointer;
		typedef T& reference;

		_CombinedBufferIterator(CombinedBuffer<T> *buffer = 0, int pos = 0)
			: m_buffer(buffer)
		{
			pos += buffer->Offset();
			if (pos < buffer->BufferSize(0)) {
				m_ptr = buffer->Buffer(0) + pos;
				m_ptr_end = buffer->Buffer(0) + buffer->BufferSize(0);
			}
			else {
				pos -= buffer->BufferSize(0);
				m_ptr = buffer->Buffer(1) + pos;
				m_ptr_end = buffer->Buffer(1) + buffer->BufferSize(1);
			}
		}

		_CombinedBufferIterator<T> &operator=(const _CombinedBufferIterator<T> &rhs)
		{
			m_buffer = rhs.m_buffer;
			m_ptr = rhs.m_ptr;
			m_ptr_end = rhs.m_pre_end;
			return *this;
		}

		bool operator==(const _CombinedBufferIterator<T> &rhs) const
		{
			return (m_ptr == rhs.m_ptr) && (m_buffer == rhs.m_buffer);
		}

		bool operator!=(const _CombinedBufferIterator<T> &rhs) const
		{
			return !(operator==(rhs));
		}

		void operator++()
		{
			++m_ptr;
			if (m_ptr >= m_ptr_end) {
				if (m_ptr_end == m_buffer->Buffer(0) + m_buffer->BufferSize(0)) {
					m_ptr = m_buffer->Buffer(1);
					m_ptr_end = m_buffer->Buffer(1) + m_buffer->BufferSize(1);
				}
			}
		}
	
		void operator++(int)
		{
			++(*this);
		}

		short &operator*()
		{
			assert(m_ptr);
			return *m_ptr;
		}

	private:
		CombinedBuffer<T> *m_buffer;
		T *m_ptr_end;
		T *m_ptr;
	};

	template<class T>
	class CombinedBuffer
	{
	public:
		typedef _CombinedBufferIterator<T> Iterator;

		CombinedBuffer(T *buffer1, int size1, T *buffer2, int size2)
			: m_offset(0)
		{
			m_buffer[0] = buffer1;
			m_buffer[1] = buffer2;
			m_buffer[2] = 0;
			m_size[0] = size1;
			m_size[1] = size2;
			m_size[2] = -1;
		}

		int Size()
		{
			return m_size[0] + m_size[1] - m_offset;
		}

		int Shift(int shift)
		{
			m_offset += shift;
			return m_offset;
		}

		int Offset() const
		{
			return m_offset;
		}

		Iterator Begin()
		{
			return Iterator(this, 0);
		}

		Iterator End()
		{
			return Iterator(this, Size());
		}

		T &operator[](int i)
		{
			i += m_offset;
			if (i < m_size[0]) {
				return m_buffer[0][i];
			}
			i -= m_size[0];
			return m_buffer[1][i];
		}

		T *Buffer(int i)
		{
			return m_buffer[i];
		}

		int BufferSize(int i)
		{
			return m_size[i];
		}

	private:
		T *m_buffer[3];
		int m_size[3];
		int m_offset;
	};

};

#endif
