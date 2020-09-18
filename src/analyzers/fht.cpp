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

#include "fht.h"

#include <cmath>

FHT::FHT(int n) : num_((n < 3) ? 0 : 1 << n), exp2_((n < 3) ? -1 : n) {
  if (n > 3) {
    buf_vector_.resize(num_);
    tab_vector_.resize(num_ * 2);
    makeCasTable();
  }
}

FHT::~FHT() {}

int FHT::sizeExp() const { return exp2_; }
int FHT::size() const { return num_; }

float* FHT::buf_() { return buf_vector_.data(); }
float* FHT::tab_() { return tab_vector_.data(); }
int* FHT::log_() { return log_vector_.data(); }

void FHT::makeCasTable(void) {
  float* costab = tab_();
  float* sintab = tab_() + num_ / 2 + 1;

  for (int ul = 0; ul < num_; ul++) {
    float d = M_PI * ul / (num_ / 2);
    *costab = *sintab = cos(d);

    costab += 2;
    sintab += 2;
    if (sintab > tab_() + num_ * 2) sintab = tab_() + 1;
  }
}

void FHT::scale(float* p, float d) {
  for (int i = 0; i < (num_ / 2); i++) *p++ *= d;
}

void FHT::ewma(float* d, float* s, float w) {
  for (int i = 0; i < (num_ / 2); i++, d++, s++) *d = *d * w + *s * (1 - w);
}

void FHT::logSpectrum(float* out, float* p) {
  int n = num_ / 2, i, j, k, *r;
  if (log_vector_.size() < n) {
    log_vector_.resize(n);
    float f = n / log10(static_cast<double>(n));
    for (i = 0, r = log_(); i < n; i++, r++) {
      j = static_cast<int>(rint(log10(i + 1.0) * f));
      *r = j >= n ? n - 1 : j;
    }
  }
  semiLogSpectrum(p);
  *out++ = *p = *p / 100;
  for (k = i = 1, r = log_(); i < n; i++) {
    j = *r++;
    if (i == j) {
      *out++ = p[i];
    } else {
      float base = p[k - 1];
      float step = (p[j] - base) / (j - (k - 1));
      for (float corr = 0; k <= j; k++, corr += step) *out++ = base + corr;
    }
  }
}

void FHT::semiLogSpectrum(float* p) {
  power2(p);
  for (int i = 0; i < (num_ / 2); i++, p++) {
    float e = 10.0 * log10(sqrt(*p / 2));
    *p = e < 0 ? 0 : e;
  }
}

void FHT::spectrum(float* p) {
  power2(p);
  for (int i = 0; i < (num_ / 2); i++, p++)
    *p = static_cast<float>(sqrt(*p / 2));
}

void FHT::power(float* p) {
  power2(p);
  for (int i = 0; i < (num_ / 2); i++) *p++ /= 2;
}

void FHT::power2(float* p) {
  _transform(p, num_, 0);

  *p = static_cast<float>(2 * pow(*p, 2));
  p++;

  float* q = p + num_ - 2;
  for (int i = 1; i < (num_ / 2); i++) {
    *p = static_cast<float>(pow(*p, 2) + pow(*q, 2));
    p++;
    q--;
  }
}

void FHT::transform(float* p) {
  if (num_ == 8)
    transform8(p);
  else
    _transform(p, num_, 0);
}

void FHT::transform8(float* p) {
  float a, b, c, d, e, f, g, h, b_f2, d_h2;
  float a_c_eg, a_ce_g, ac_e_g, aceg, b_df_h, bdfh;

  a = *p++, b = *p++, c = *p++, d = *p++;
  e = *p++, f = *p++, g = *p++, h = *p;
  b_f2 = (b - f) * M_SQRT2;
  d_h2 = (d - h) * M_SQRT2;

  a_c_eg = a - c - e + g;
  a_ce_g = a - c + e - g;
  ac_e_g = a + c - e - g;
  aceg = a + c + e + g;

  b_df_h = b - d + f - h;
  bdfh = b + d + f + h;

  *p = a_c_eg - d_h2;
  *--p = a_ce_g - b_df_h;
  *--p = ac_e_g - b_f2;
  *--p = aceg - bdfh;
  *--p = a_c_eg + d_h2;
  *--p = a_ce_g + b_df_h;
  *--p = ac_e_g + b_f2;
  *--p = aceg + bdfh;
}

void FHT::_transform(float* p, int n, int k) {
  if (n == 8) {
    transform8(p + k);
    return;
  }

  int i, j, ndiv2 = n / 2;
  float a, *t1, *t2, *t3, *t4, *ptab, *pp;

  for (i = 0, t1 = buf_(), t2 = buf_() + ndiv2, pp = &p[k]; i < ndiv2; i++)
    *t1++ = *pp++, *t2++ = *pp++;

  std::copy(buf_(), buf_() + n, p + k);

  _transform(p, ndiv2, k);
  _transform(p, ndiv2, k + ndiv2);

  j = num_ / ndiv2 - 1;
  t1 = buf_();
  t2 = t1 + ndiv2;
  t3 = p + k + ndiv2;
  ptab = tab_();
  pp = p + k;

  a = *ptab++ * *t3++;
  a += *ptab * *pp;
  ptab += j;

  *t1++ = *pp + a;
  *t2++ = *pp++ - a;

  for (i = 1, t4 = p + k + n; i < ndiv2; i++, ptab += j) {
    a = *ptab++ * *t3++;
    a += *ptab * *--t4;

    *t1++ = *pp + a;
    *t2++ = *pp++ - a;
  }

  std::copy(buf_(), buf_() + n, p + k);
}
