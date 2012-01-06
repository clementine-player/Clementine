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

#ifndef CHROMAPRINT_QUANTIZER_H_
#define CHROMAPRINT_QUANTIZER_H_

#include <assert.h>
#include <ostream>

namespace Chromaprint
{

	class Quantizer
	{
	public:
		Quantizer(double t0 = 0.0, double t1 = 0.0, double t2 = 0.0)
			: m_t0(t0), m_t1(t1), m_t2(t2)
		{
			assert(t0 <= t1 && t1 <= t2);
		}

		int Quantize(double value) const
		{
			if (value < m_t1) {
				if (value < m_t0) {
					return 0;
				}
				return 1;
			}
			else {
				if (value < m_t2) {
					return 2;
				}
				return 3;
			}
		}

		double t0() const { return m_t0; }
		void set_t0(double t) { m_t0 = t; }

		double t1() const { return m_t1; }
		void set_t1(double t) { m_t1 = t; }

		double t2() const { return m_t2; }
		void set_t2(double t) { m_t2 = t; }

	private:
		double m_t0, m_t1, m_t2;
	};

	inline std::ostream &operator<<(std::ostream &stream, const Quantizer &q)
	{
		stream << "Quantizer(" << q.t0() << ", " << q.t1() << ", " << q.t2() << ")";
		return stream;
	}

};

#endif
