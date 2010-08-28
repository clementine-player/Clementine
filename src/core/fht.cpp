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

#include <math.h>
#include <string.h>
#include "fht.h"


FHT::FHT(int n) :
    m_buf(0),
    m_tab(0),
    m_log(0)
{
    if (n < 3) {
        m_num = 0;
        m_exp2 = -1;
        return;
    }
    m_exp2 = n;
    m_num = 1 << n;
    if (n > 3) {
        m_buf = new float[m_num];
        m_tab = new float[m_num * 2];
        makeCasTable();
    }
}


FHT::~FHT()
{
    delete[] m_buf;
    delete[] m_tab;
    delete[] m_log;
}


void FHT::makeCasTable(void)
{
    float d, *costab, *sintab;
    int ul, ndiv2 = m_num / 2;

    for (costab = m_tab, sintab = m_tab + m_num / 2 + 1, ul = 0; ul < m_num; ul++) {
        d = M_PI * ul / ndiv2;
        *costab = *sintab = cos(d);

        costab += 2, sintab += 2;
        if (sintab > m_tab + m_num * 2)
            sintab = m_tab + 1;
    }
}


float* FHT::copy(float *d, float *s)
{
    return (float *)memcpy(d, s, m_num * sizeof(float));
}


float* FHT::clear(float *d)
{
    return (float *)memset(d, 0, m_num * sizeof(float));
}


void FHT::scale(float *p, float d)
{
    for (int i = 0; i < (m_num / 2); i++)
        *p++ *= d;
}


void FHT::ewma(float *d, float *s, float w)
{
    for (int i = 0; i < (m_num / 2); i++, d++, s++)
        *d = *d * w + *s * (1 - w);
}


void FHT::logSpectrum(float *out, float *p)
{
    int n = m_num / 2, i, j, k, *r;
    if (!m_log) {
        m_log = new int[n];
        float f = n / log10((double)n);
        for (i = 0, r = m_log; i < n; i++, r++) {
            j = int(rint(log10(i + 1.0) * f));
            *r = j >= n ? n - 1 : j;
        }
    }
    semiLogSpectrum(p);
    *out++ = *p = *p / 100;
    for (k = i = 1, r = m_log; i < n; i++) {
        j = *r++;
        if (i == j)
            *out++ = p[i];
        else {
            float base = p[k - 1];
            float step = (p[j] - base) / (j - (k - 1));
            for (float corr = 0; k <= j; k++, corr += step)
                *out++ = base + corr;
        }
    }
}


void FHT::semiLogSpectrum(float *p)
{
    float e;
    power2(p);
    for (int i = 0; i < (m_num / 2); i++, p++) {
        e = 10.0 * log10(sqrt(*p * .5));
        *p = e < 0 ? 0 : e;
    }
}


void FHT::spectrum(float *p)
{
    power2(p);
    for (int i = 0; i < (m_num / 2); i++, p++)
        *p = (float)sqrt(*p * .5);
}


void FHT::power(float *p)
{
    power2(p);
    for (int i = 0; i < (m_num / 2); i++)
        *p++ *= .5;
}


void FHT::power2(float *p)
{
    int i;
    float *q;
    _transform(p, m_num, 0);

    *p = (*p * *p), *p += *p, p++;

    for (i = 1, q = p + m_num - 2; i < (m_num / 2); i++, --q)
        *p = (*p * *p) + (*q * *q), p++;
}


void FHT::transform(float *p)
{
    if (m_num == 8)
        transform8(p);
    else
        _transform(p, m_num, 0);
}


void FHT::transform8(float *p)
{
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


void FHT::_transform(float *p, int n, int k)
{
    if (n == 8) {
        transform8(p + k);
        return;
    }

    int i, j, ndiv2 = n / 2;
    float a, *t1, *t2, *t3, *t4, *ptab, *pp;

    for (i = 0, t1 = m_buf, t2 = m_buf + ndiv2, pp = &p[k]; i < ndiv2; i++)
        *t1++ = *pp++, *t2++ = *pp++;

    memcpy(p + k, m_buf, sizeof(float) * n);

    _transform(p, ndiv2, k);
    _transform(p, ndiv2, k + ndiv2);

    j = m_num / ndiv2 - 1;
    t1 = m_buf;
    t2 = t1 + ndiv2;
    t3 = p + k + ndiv2;
    ptab = m_tab;
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
    memcpy(p + k, m_buf, sizeof(float) * n);
}

