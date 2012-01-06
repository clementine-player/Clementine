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

#ifndef CHROMAPRINT_FILTER_H_
#define CHROMAPRINT_FILTER_H_

#include <ostream>
#include "integral_image.h"

namespace Chromaprint
{

	class Filter
	{
	public:
		Filter(int type = 0, int y = 0, int height = 0, int width = 0)
			: m_type(type), m_y(y), m_height(height), m_width(width)
		{}

		double Apply(IntegralImage *image, int offset) const;

		int type() const { return m_type; }
		void set_type(int type) { m_type = type; }

		int y() const { return m_y; }
		void set_y(int y) { m_y = y; }

		int height() const { return m_height; }
		void set_height(int height) { m_height = height; }

		int width() const { return m_width; }
		void set_width(int width) { m_width = width; }

	private:
		int m_type;
		int m_y;
		int m_height;
		int m_width;
	};

	inline std::ostream &operator<<(std::ostream &stream, const Filter &f)
	{
		stream << "Filter(" << f.type() << ", " << f.y() << ", "
			<< f.height() << ", " << f.width() << ")";
		return stream;
	}
	
};

#endif
