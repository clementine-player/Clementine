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

#include <math.h>
#include "filter.h"
#include "filter_utils.h"

using namespace std;
using namespace Chromaprint;

#define COMPARE_FUNC SubtractLog

double Filter::Apply(IntegralImage *image, int x) const
{
	switch (m_type) {
	case 0:
		return Filter0(image, x, m_y, m_width, m_height, COMPARE_FUNC);
	case 1:
		return Filter1(image, x, m_y, m_width, m_height, COMPARE_FUNC);
	case 2:
		return Filter2(image, x, m_y, m_width, m_height, COMPARE_FUNC);
	case 3:
		return Filter3(image, x, m_y, m_width, m_height, COMPARE_FUNC);
	case 4:
		return Filter4(image, x, m_y, m_width, m_height, COMPARE_FUNC);
	case 5:
		return Filter5(image, x, m_y, m_width, m_height, COMPARE_FUNC);
	}
	return 0.0;
}
