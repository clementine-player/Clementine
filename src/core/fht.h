// FHT - Fast Hartley Transform Class
//
// Copyright (C) 2004  Melchior FRANZ - mfranz@kde.org
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA
//
// $Id$

#ifndef FHT_H
#define FHT_H

/**
 * Implementation of the Hartley Transform after Bracewell's discrete
 * algorithm. The algorithm is subject to US patent No. 4,646,256 (1987)
 * but was put into public domain by the Board of Trustees of Stanford
 * University in 1994 and is now freely available[1].
 *
 * [1] Computer in Physics, Vol. 9, No. 4, Jul/Aug 1995 pp 373-379
 */
class FHT
{
	int	m_exp2;
	int	m_num;
	float	*m_buf;
	float	*m_tab;
	int	*m_log;

	/**
	 * Create a table of "cas" (cosine and sine) values.
	 * Has only to be done in the constructor and saves from
	 * calculating the same values over and over while transforming.
	 */
	void	makeCasTable();

	/**
	 * Recursive in-place Hartley transform. For internal use only!
	 */
	void	_transform(float *, int, int);

   public:
	/**
	* Prepare transform for data sets with @f$2^n@f$ numbers, whereby @f$n@f$
	* should be at least 3. Values of more than 3 need a trigonometry table.
	* @see makeCasTable()
	*/
	FHT(int);

	~FHT();
	inline int sizeExp() const { return m_exp2; }
	inline int size() const { return m_num; }
	float	*copy(float *, float *);
	float	*clear(float *);
	void	scale(float *, float);

	/**
	 * Exponentially Weighted Moving Average (EWMA) filter.
	 * @param d is the filtered data.
	 * @param s is fresh input.
	 * @param w is the weighting factor.
	 */
	void	ewma(float *d, float *s, float w);

	/**
	 * Logarithmic audio spectrum. Maps semi-logarithmic spectrum
	 * to logarithmic frequency scale, interpolates missing values.
	 * A logarithmic index map is calculated at the first run only.
	 * @param p is the input array.
	 * @param out is the spectrum.
	 */
	void	logSpectrum(float *out, float *p);

	/**
	 * Semi-logarithmic audio spectrum.
	 */
	void	semiLogSpectrum(float *);

	/**
	 * Fourier spectrum.
	 */
	void	spectrum(float *);

	/**
	 * Calculates a mathematically correct FFT power spectrum.
	 * If further scaling is applied later, use power2 instead
	 * and factor the 0.5 in the final scaling factor.
	 * @see FHT::power2()
	 */
	void	power(float *);

	/**
	 * Calculates an FFT power spectrum with doubled values as a
	 * result. The values need to be multiplied by 0.5 to be exact.
	 * Note that you only get @f$2^{n-1}@f$ power values for a data set
	 * of @f$2^n@f$ input values. This is the fastest transform.
	 * @see FHT::power()
	 */
	void	power2(float *);

	/**
	 * Discrete Hartley transform of data sets with 8 values.
	 */
	void	transform8(float *);

	void	transform(float *);
};

#endif
