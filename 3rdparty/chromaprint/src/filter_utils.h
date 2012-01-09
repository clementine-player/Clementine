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

#ifndef CHROMAPRINT_FILTER_UTILS_H_
#define CHROMAPRINT_FILTER_UTILS_H_

#include <math.h>
#include "integral_image.h"
#include "utils.h"

namespace Chromaprint
{

	inline double Subtract(double a, double b)
	{
		return a - b;
	}

	inline double SubtractLog(double a, double b)
	{
		double r = log(1.0 + a) - log(1.0 + b);
		assert(!IsNaN(r));
		return r;
	}

	// oooooooooooooooo
	// oooooooooooooooo
	// oooooooooooooooo
	// oooooooooooooooo
	template<class Comparator>
	double Filter0(IntegralImage *image, int x, int y, int w, int h, Comparator cmp)
	{
		double a = image->Area(x, y, x + w - 1, y + h - 1);
		double b = 0;
		return cmp(a, b);
	}

	// ................
	// ................
	// oooooooooooooooo
	// oooooooooooooooo
	template<class Comparator>
	double Filter1(IntegralImage *image, int x, int y, int w, int h, Comparator cmp)
	{
		int h_2 = h / 2;

		double a = image->Area(x, y + h_2, x + w - 1, y + h - 1);
		double b = image->Area(x, y, x + w - 1, y + h_2 - 1);

		return cmp(a, b);
	}

	// .......ooooooooo
	// .......ooooooooo
	// .......ooooooooo
	// .......ooooooooo
	template<class Comparator>
	double Filter2(IntegralImage *image, int x, int y, int w, int h, Comparator cmp)
	{
		int w_2 = w / 2;

		double a = image->Area(x + w_2, y, x + w - 1, y + h - 1);
		double b = image->Area(x, y, x + w_2 - 1, y + h - 1);

		return cmp(a, b);
	}

	// .......ooooooooo
	// .......ooooooooo
	// ooooooo.........
	// ooooooo.........
	template<class Comparator>
	double Filter3(IntegralImage *image, int x, int y, int w, int h, Comparator cmp)
	{
		int w_2 = w / 2;
		int h_2 = h / 2;

		double a = image->Area(x,       y + h_2, x + w_2 - 1, y + h - 1) +
				   image->Area(x + w_2, y,       x + w - 1,   y + h_2 - 1);
		double b = image->Area(x,       y,       x + w_2 -1,  y + h_2 - 1) +
				   image->Area(x + w_2, y + h_2, x + w - 1,   y + h - 1);

		return cmp(a, b);
	}

	// ................
	// oooooooooooooooo
	// ................
	template<class Comparator>
	double Filter4(IntegralImage *image, int x, int y, int w, int h, Comparator cmp)
	{
		int h_3 = h / 3;

		double a = image->Area(x, y + h_3,     x + w - 1, y + 2 * h_3 - 1);
		double b = image->Area(x, y,           x + w - 1, y + h_3 - 1) +
				   image->Area(x, y + 2 * h_3, x + w - 1, y + h - 1);

		return cmp(a, b);
	}

	// .....oooooo.....
	// .....oooooo.....
	// .....oooooo.....
	// .....oooooo.....
	template<class Comparator>
	double Filter5(IntegralImage *image, int x, int y, int w, int h, Comparator cmp)
	{
		int w_3 = w / 3;

		double a = image->Area(x + w_3,     y, x + 2 * w_3 - 1, y + h - 1);
		double b = image->Area(x,           y, x + w_3 - 1,     y + h - 1) +
				   image->Area(x + 2 * w_3, y, x + w - 1,       y + h - 1);

		return cmp(a, b);
	}

};

#endif
