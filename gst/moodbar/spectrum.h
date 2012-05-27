/* GStreamer moodbar plugin globals
 * Copyright (C) 2006 Joseph Rabinoff <bobqwatson@yahoo.com>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __SPECTRUM_H__
#define __SPECTRUM_H__


/* Since fftwspectrum and fftwunspectrum are supposed to be 
 * opposites, they'll be using the same caps: */

#define SPECTRUM_SIGNAL_CAPS "audio/x-raw-float, " \
			       "rate = (int) [ 1, MAX ], " \
			       "channels = (int) 1, " \
			       "endianness = (int) BYTE_ORDER, " \
			       "width = (int) 32, " \
			       "signed = (boolean) true"

/* audio/x-spectrum-complex-float is an array of complex floats. A
 * complex float is just a pair (r, i) of a real float and an
 * imaginary float, each with the specified width.  The properties
 * are as follows:
 *   rate:  the rate of the original signal
 *   size:  the number of signals processed to make the current buffer
 *   step:  the number of signals advanced after the current buffer
 *   width: the size of the real & imaginary parts of the data
 *   endianness: ditto
 *
 * Each audio/x-spectrum-complex-float buffer represents the Fourier
 * transform of size samples, and hence _must_ have exactly
 * floor(size/2) + 1 complex floats in it; in other words, its
 * buffer size must be (floor(size/2) + 1) * 2 * sizeof(gfloat)
 */

#define SPECTRUM_FREQ_CAPS "audio/x-spectrum-complex-float, " \
			     "rate = (int) [ 1, MAX ], " \
			     "endianness = (int) BYTE_ORDER, " \
			     "width = (int) 32, " \
			     "size = (int) [ 1, MAX ], " \
			     "step = (int) [ 1, MAX ]"


/* Given a band number from a spectrum made from size audio
 * samples at the given rate, return the frequency that band
 * corresponds to.
 */
#define GST_SPECTRUM_BAND_FREQ(band, size, rate) \
  (((gfloat)(band))*((gfloat)(rate))/((gfloat)(size)))

#ifdef __cplusplus
extern "C" {
#endif
  void gstmoodbar_register_static();
#ifdef __cplusplus
}
#endif

#endif  /* __SPECTRUM_H__ */
