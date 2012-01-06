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

#ifndef CHROMAPRINT_FFT_LIB_H_
#define CHROMAPRINT_FFT_LIB_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WITH_AVFFT
#include "fft_lib_avfft.h"
#endif

#ifdef WITH_FFTW3
#include "fft_lib_fftw3.h"
#endif

#ifdef WITH_VDSP
#include "fft_lib_vdsp.h"
#endif

#endif
