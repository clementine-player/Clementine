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

#ifndef CHROMAPRINT_CLASSIFIER_H_
#define CHROMAPRINT_CLASSIFIER_H_

#include <ostream>
#include "quantizer.h"
#include "filter.h"
#include "integral_image.h"

namespace Chromaprint
{

	class Classifier
	{
	public:
		Classifier(const Filter &filter = Filter(), const Quantizer &quantizer = Quantizer())
			: m_filter(filter), m_quantizer(quantizer)
		{
		}

		int Classify(IntegralImage *image, int offset) const
		{
			double value = m_filter.Apply(image, offset);
			return m_quantizer.Quantize(value);
		}

		const Filter &filter() const { return m_filter; }
		const Quantizer &quantizer() const { return m_quantizer; }

	private:
		Filter m_filter;
		Quantizer m_quantizer;
	};

	inline std::ostream &operator<<(std::ostream &stream, const Classifier &q)
	{
		stream << "Classifier(" << q.filter() << ", " << q.quantizer() << ")";
		return stream;
	}
};

#endif
