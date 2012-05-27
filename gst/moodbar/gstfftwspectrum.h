/* GStreamer FFTW-based signal-to-spectrum converter
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


#ifndef __GST_FFTWSPECTRUM_H__
#define __GST_FFTWSPECTRUM_H__

#include <gst/gst.h>
#include <fftw3.h>

G_BEGIN_DECLS

/* #defines don't like whitespacey bits */
#define GST_TYPE_FFTWSPECTRUM \
  (gst_fftwspectrum_get_type())
#define GST_FFTWSPECTRUM(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_FFTWSPECTRUM,GstFFTWSpectrum))
#define GST_FFTWSPECTRUM_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_FFTWSPECTRUM,GstFFTWSpectrumClass))

typedef struct _GstFFTWSpectrum      GstFFTWSpectrum;
typedef struct _GstFFTWSpectrumClass GstFFTWSpectrumClass;

struct _GstFFTWSpectrum
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  /* Stream data */
  gint rate, size, step;

  /* Actual queued (incoming) stream */
  gfloat       *samples;
  gint          numsamples;
  GstClockTime  timestamp;  /* Timestamp of the first sample */
  guint64       offset;     /* Offset of the first sample */

  /* State data for fftw */
  float      *fftw_in;
  float      *fftw_out;
  fftwf_plan  fftw_plan;

  /* Properties */
  gint32   def_size, def_step;
  gboolean hi_q;
};

struct _GstFFTWSpectrumClass 
{
  GstElementClass parent_class;
};

GType gst_fftwspectrum_get_type (void);

G_END_DECLS

#endif /* __GST_FFTWSPECTRUM_H__ */
