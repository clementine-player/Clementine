/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) <2009> Sebastian Dröge <sebastian.droege@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

// Adapted from gstspectrum for Clementine with the following changes:
//   - Uses fftw instead of kiss fft (2x faster).
//   - Hardcoded to 1 channel (use an audioconvert element to do the work
//     instead, simplifies this code a lot).
//   - Send output via a callback instead of GST messages (less overhead).
//   - Removed all properties except interval and band.


#ifndef GST_MOODBAR_FASTSPECTRUM_H_
#define GST_MOODBAR_FASTSPECTRUM_H_

#include <functional>

#include <gst/gst.h>
#include <gst/audio/gstaudiofilter.h>
#include <fftw3.h>

G_BEGIN_DECLS

#define GST_TYPE_FASTSPECTRUM            (gst_fastspectrum_get_type())
#define GST_FASTSPECTRUM(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_FASTSPECTRUM,GstFastSpectrum))
#define GST_IS_FASTSPECTRUM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_FASTSPECTRUM))
#define GST_FASTSPECTRUM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_FASTSPECTRUM,GstFastSpectrumClass))
#define GST_IS_FASTSPECTRUM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_FASTSPECTRUM))

using GstFastSpectrumInputData = void(*)(const guint8* in, double* out,
    guint len, double max_value, guint op, guint nfft);

using OutputCallback = std::function<void(double* magnitudes, int size)>;

struct GstFastSpectrum {
  GstAudioFilter parent;

  /* properties */
  guint64 interval;             /* how many nanoseconds between emits */
  guint64 frames_per_interval;  /* how many frames per interval */
  guint64 frames_todo;
  guint bands;                  /* number of spectrum bands */
  gboolean multi_channel;       /* send separate channel results */

  guint64 num_frames;           /* frame count (1 sample per channel)
                                 * since last emit */
  guint64 num_fft;              /* number of FFTs since last emit */
  GstClockTime message_ts;      /* starttime for next message */

  /* <private> */
  bool channel_data_initialised;
  double* input_ring_buffer;
  double* fft_input;
  fftw_complex* fft_output;
  double* spect_magnitude;
  fftw_plan plan;

  guint input_pos;
  guint64 error_per_interval;
  guint64 accumulated_error;

  GMutex lock;

  GstFastSpectrumInputData input_data;

  OutputCallback output_callback;
};

struct GstFastSpectrumClass {
  GstAudioFilterClass parent_class;
};

GType gst_fastspectrum_get_type (void);

G_END_DECLS

#endif // GST_MOODBAR_FASTSPECTRUM_H_
