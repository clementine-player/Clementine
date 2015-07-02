/* This file is part of Clementine.
   Copyright 2004, Melchior FRANZ <mfranz@kde.org>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Original Author:  Melchior FRANZ  <mfranz@kde.org>  2004
*/

#ifndef ANALYZERS_FHT_H_
#define ANALYZERS_FHT_H_

/**
 * Implementation of the Hartley Transform after Bracewell's discrete
 * algorithm. The algorithm is subject to US patent No. 4,646,256 (1987)
 * but was put into public domain by the Board of Trustees of Stanford
 * University in 1994 and is now freely available[1].
 *
 * [1] Computer in Physics, Vol. 9, No. 4, Jul/Aug 1995 pp 373-379
 */
class FHT {
  int exp2_;
  int num_;
  float* buf_;
  float* tab_;
  int* log_;

  /**
   * Create a table of "cas" (cosine and sine) values.
   * Has only to be done in the constructor and saves from
   * calculating the same values over and over while transforming.
   */
  void makeCasTable();

  /**
   * Recursive in-place Hartley transform. For internal use only!
   */
  void _transform(float*, int, int);

 public:
  /**
  * Prepare transform for data sets with @f$2^n@f$ numbers, whereby @f$n@f$
  * should be at least 3. Values of more than 3 need a trigonometry table.
  * @see makeCasTable()
  */
  explicit FHT(int);

  ~FHT();
  inline int sizeExp() const { return exp2_; }
  inline int size() const { return num_; }
  float* copy(float*, float*);
  float* clear(float*);
  void scale(float*, float);

  /**
   * Exponentially Weighted Moving Average (EWMA) filter.
   * @param d is the filtered data.
   * @param s is fresh input.
   * @param w is the weighting factor.
   */
  void ewma(float* d, float* s, float w);

  /**
   * Logarithmic audio spectrum. Maps semi-logarithmic spectrum
   * to logarithmic frequency scale, interpolates missing values.
   * A logarithmic index map is calculated at the first run only.
   * @param p is the input array.
   * @param out is the spectrum.
   */
  void logSpectrum(float* out, float* p);

  /**
   * Semi-logarithmic audio spectrum.
   */
  void semiLogSpectrum(float*);

  /**
   * Fourier spectrum.
   */
  void spectrum(float*);

  /**
   * Calculates a mathematically correct FFT power spectrum.
   * If further scaling is applied later, use power2 instead
   * and factor the 0.5 in the final scaling factor.
   * @see FHT::power2()
   */
  void power(float*);

  /**
   * Calculates an FFT power spectrum with doubled values as a
   * result. The values need to be multiplied by 0.5 to be exact.
   * Note that you only get @f$2^{n-1}@f$ power values for a data set
   * of @f$2^n@f$ input values. This is the fastest transform.
   * @see FHT::power()
   */
  void power2(float*);

  /**
   * Discrete Hartley transform of data sets with 8 values.
   */
  void transform8(float*);

  void transform(float*);
};

#endif  // ANALYZERS_FHT_H_
