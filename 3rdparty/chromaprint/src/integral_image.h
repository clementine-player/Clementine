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

#ifndef CHROMAPRINT_INTEGRAL_IMAGE_H_
#define CHROMAPRINT_INTEGRAL_IMAGE_H_

#include "image.h"

namespace Chromaprint
{

	/**
	 * Image transformation that allows us to quickly calculate the sum of
	 * values in a rectangular area.
	 *
	 * http://en.wikipedia.org/wiki/Summed_area_table
	 */
	class IntegralImage
	{
	public:
		/**
		 * Construct the integral image. Note that will modify the original
		 * image in-place, so it will not be usable afterwards.
		 */
		IntegralImage(Image *image) : m_image(image)
		{
			Transform();
		}

		//! Number of columns in the image
		int NumColumns() const { return m_image->NumColumns(); }

		//! Number of rows in the image
		int NumRows() const { return m_image->NumRows(); }

		double *Row(int i)
		{
			return m_image->Row(i);
		}
	
		double *operator[](int i)
		{
			return m_image->Row(i);
		}
	
		double Area(int x1, int y1, int x2, int y2)
		{
			double area = (*m_image)[x2][y2];
			if (x1 > 0) {
				area -= (*m_image)[x1-1][y2];
				if (y1 > 0) {
					area += (*m_image)[x1-1][y1-1];
				}
			}
			if (y1 > 0) {
				area -= (*m_image)[x2][y1-1];
			}
			//std::cout << "Area("<<x1<<","<<y1<<","<<x2<<","<<y2<<") = "<<area<<"\n";
			return area;
		}

	private:
		void Transform();

		Image *m_image;
	};

};

#endif
