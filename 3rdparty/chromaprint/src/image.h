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

#ifndef CHROMAPRINT_IMAGE_H_
#define CHROMAPRINT_IMAGE_H_

#include <vector>
#include <algorithm>
#include <assert.h>

namespace Chromaprint
{

	class Image
	{
	public:
		explicit Image(int columns) : m_columns(columns)
		{
		}

		Image(int columns, int rows) : m_columns(columns), m_data(columns * rows)
		{
		}

		template<class Iterator>
		Image(int columns, Iterator first, Iterator last) : m_columns(columns), m_data(first, last)
		{
		}

		int NumColumns() const { return m_columns; }
		int NumRows() const { return m_data.size() / m_columns; }

		void AddRow(const std::vector<double> &row)
		{
			m_data.resize(m_data.size() + m_columns);
			std::copy(row.begin(), row.end(), m_data.end() - m_columns);
		}

		double *Row(int i)
		{
			assert(0 <= i && i < NumRows());
			return &m_data[m_columns * i];
		}

		double *operator[](int i)
		{
			return Row(i);
		}

	private:
		int m_columns;
		std::vector<double> m_data;
	};

};

#endif
