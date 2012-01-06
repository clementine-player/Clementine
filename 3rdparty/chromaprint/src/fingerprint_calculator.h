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

#ifndef CHROMAPRINT_FINGERPRINT_CALCULATOR_H_
#define CHROMAPRINT_FINGERPRINT_CALCULATOR_H_

#include <stdint.h>
#include <vector>

namespace Chromaprint
{
	class Classifier;
	class Image;
	class IntegralImage;

	class FingerprintCalculator
	{
	public:
		FingerprintCalculator(const Classifier *classifiers, int num_classifiers);

		std::vector<int32_t> Calculate(Image *image);

		int32_t CalculateSubfingerprint(IntegralImage *image, int offset);

	private:
		const Classifier *m_classifiers;
		int m_num_classifiers;
		int m_max_filter_width;
	};

};

#endif

