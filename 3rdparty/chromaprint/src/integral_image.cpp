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

#include "integral_image.h"

using namespace std;
using namespace Chromaprint;

void IntegralImage::Transform()
{
	int num_rows = m_image->NumRows();
	int num_columns = m_image->NumColumns();
	double *current = m_image->Row(0) + 1;
	double *last = m_image->Row(0);
	for (int m = 1; m < num_columns; m++) {
		// First column - add value on top
		*current = current[0] + current[-1];
		++current;
	}
	for (int n = 1; n < num_rows; n++) {
		// First row - add value on left
		*current = current[0] + last[0];
		++current;
		++last;
		// Add values on left, up and up-left
		for (int m = 1; m < num_columns; m++) {
			*current = current[0] + current[-1] + last[0] - last[-1];
			++current;
			++last;
		}
	}
}
