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

#include "fingerprint_calculator.h"
#include "classifier.h"
#include "debug.h"
#include "utils.h"

using namespace std;
using namespace Chromaprint;

FingerprintCalculator::FingerprintCalculator(const Classifier *classifiers, int num_classifiers)
	: m_classifiers(classifiers), m_num_classifiers(num_classifiers)
{
	m_max_filter_width = 0;
	for (int i = 0; i < num_classifiers; i++) {
		m_max_filter_width = max(m_max_filter_width, classifiers[i].filter().width());
	}
	assert(m_max_filter_width > 0);
}


vector<int32_t> FingerprintCalculator::Calculate(Image *image)
{
	int length = image->NumRows() - m_max_filter_width + 1;
	if (length <= 0) {
		DEBUG() << "Chromaprint::FingerprintCalculator::Calculate() -- Not "
				<< "enough data. Image has " << image->NumRows() << " rows, "
				<< "needs at least " << m_max_filter_width << " rows.\n";
		return vector<int32_t>();
	}
	IntegralImage integral_image(image);
	vector<int32_t> fingerprint(length);
	for (int i = 0; i < length; i++) {
		fingerprint[i] = CalculateSubfingerprint(&integral_image, i);
	}
	return fingerprint;
}

int32_t FingerprintCalculator::CalculateSubfingerprint(IntegralImage *image, int offset)
{
	uint32_t bits = 0;
	for (int i = 0; i < m_num_classifiers; i++) {
	//for (int i = m_num_classifiers - 1; i >= 0; i--) {
		bits = (bits << 2) | GrayCode(m_classifiers[i].Classify(image, offset));
		//bits = (bits << 2) | m_classifiers[i].Classify(image, offset);
	}
	return UnsignedToSigned(bits);
}

