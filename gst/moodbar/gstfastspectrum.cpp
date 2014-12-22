/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 *               <2006,2011> Stefan Kost <ensonic@users.sf.net>
 *               <2007-2009> Sebastian Dröge <sebastian.droege@collabora.co.uk>
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

#include <cstring>
#include <cmath>

#include <QMutex>
#include <QMutexLocker>

#include "gstfastspectrum.h"

GST_DEBUG_CATEGORY_STATIC (gst_fastspectrum_debug);
#define GST_CAT_DEFAULT gst_fastspectrum_debug

/* elementfactory information */
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
# define FORMATS "{ S16LE, S24LE, S32LE, F32LE, F64LE }"
#else
# define FORMATS "{ S16BE, S24BE, S32BE, F32BE, F64BE }"
#endif

#define ALLOWED_CAPS \
  GST_AUDIO_CAPS_MAKE (FORMATS) ", " \
  "layout = (string) interleaved, " \
  "channels = 1"

/* Spectrum properties */
#define DEFAULT_INTERVAL		(GST_SECOND / 10)
#define DEFAULT_BANDS			128

enum {
  PROP_0,
  PROP_INTERVAL,
  PROP_BANDS
};

#define gst_fastspectrum_parent_class parent_class
G_DEFINE_TYPE (GstFastSpectrum, gst_fastspectrum, GST_TYPE_AUDIO_FILTER);

static void gst_fastspectrum_finalize (GObject * object);
static void gst_fastspectrum_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_fastspectrum_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static gboolean gst_fastspectrum_start (GstBaseTransform * trans);
static gboolean gst_fastspectrum_stop (GstBaseTransform * trans);
static GstFlowReturn gst_fastspectrum_transform_ip (GstBaseTransform * trans,
    GstBuffer * in);
static gboolean gst_fastspectrum_setup (GstAudioFilter * base,
    const GstAudioInfo * info);

static void
gst_fastspectrum_class_init (GstFastSpectrumClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstBaseTransformClass *trans_class = GST_BASE_TRANSFORM_CLASS (klass);
  GstAudioFilterClass *filter_class = GST_AUDIO_FILTER_CLASS (klass);
  GstCaps *caps;

  gobject_class->set_property = gst_fastspectrum_set_property;
  gobject_class->get_property = gst_fastspectrum_get_property;
  gobject_class->finalize = gst_fastspectrum_finalize;

  trans_class->start = GST_DEBUG_FUNCPTR (gst_fastspectrum_start);
  trans_class->stop = GST_DEBUG_FUNCPTR (gst_fastspectrum_stop);
  trans_class->transform_ip = GST_DEBUG_FUNCPTR (gst_fastspectrum_transform_ip);
  trans_class->passthrough_on_same_caps = TRUE;

  filter_class->setup = GST_DEBUG_FUNCPTR (gst_fastspectrum_setup);

  g_object_class_install_property (gobject_class, PROP_INTERVAL,
      g_param_spec_uint64 ("interval", "Interval",
          "Interval of time between message posts (in nanoseconds)",
          1, G_MAXUINT64, DEFAULT_INTERVAL,
          GParamFlags(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  g_object_class_install_property (gobject_class, PROP_BANDS,
      g_param_spec_uint ("bands", "Bands", "Number of frequency bands",
          0, G_MAXUINT, DEFAULT_BANDS,
          GParamFlags(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  GST_DEBUG_CATEGORY_INIT (gst_fastspectrum_debug, "spectrum", 0,
      "audio spectrum analyser element");

  gst_element_class_set_static_metadata (element_class, "Spectrum analyzer",
      "Filter/Analyzer/Audio",
      "Run an FFT on the audio signal, output spectrum data",
      "Erik Walthinsen <omega@cse.ogi.edu>, "
      "Stefan Kost <ensonic@users.sf.net>, "
      "Sebastian Dröge <sebastian.droege@collabora.co.uk>");

  caps = gst_caps_from_string (ALLOWED_CAPS);
  gst_audio_filter_class_add_pad_templates (filter_class, caps);
  gst_caps_unref (caps);

  klass->fftw_lock = new QMutex;
}

static void
gst_fastspectrum_init (GstFastSpectrum * spectrum)
{
  spectrum->interval = DEFAULT_INTERVAL;
  spectrum->bands = DEFAULT_BANDS;

  spectrum->channel_data_initialised = false;

  g_mutex_init (&spectrum->lock);
}

static void
gst_fastspectrum_alloc_channel_data (GstFastSpectrum * spectrum)
{
  guint bands = spectrum->bands;
  guint nfft = 2 * bands - 2;

  spectrum->input_ring_buffer = new double[nfft];
  spectrum->fft_input = reinterpret_cast<double*>(
      fftw_malloc(sizeof(double) * nfft));
  spectrum->fft_output =reinterpret_cast<fftw_complex*>(
      fftw_malloc(sizeof(fftw_complex) * (nfft/2+1)));

  spectrum->spect_magnitude = new double[bands]();

  GstFastSpectrumClass* klass = reinterpret_cast<GstFastSpectrumClass*>(
      G_OBJECT_GET_CLASS(spectrum));
  {
    QMutexLocker l(klass->fftw_lock);
    spectrum->plan = fftw_plan_dft_r2c_1d(
        nfft,
        spectrum->fft_input,
        spectrum->fft_output,
        FFTW_ESTIMATE);
  }
  spectrum->channel_data_initialised = true;
}

static void
gst_fastspectrum_free_channel_data (GstFastSpectrum * spectrum)
{
  GstFastSpectrumClass* klass = reinterpret_cast<GstFastSpectrumClass*>(
      G_OBJECT_GET_CLASS(spectrum));
  if (spectrum->channel_data_initialised) {
    {
      QMutexLocker l(klass->fftw_lock);
      fftw_destroy_plan(spectrum->plan);
    }
    fftw_free(spectrum->fft_input);
    fftw_free(spectrum->fft_output);
    delete[] spectrum->input_ring_buffer;
    delete[] spectrum->spect_magnitude;

    spectrum->channel_data_initialised = false;
  }
}

static void
gst_fastspectrum_flush (GstFastSpectrum * spectrum)
{
  spectrum->num_frames = 0;
  spectrum->num_fft = 0;

  spectrum->accumulated_error = 0;
}

static void
gst_fastspectrum_reset_state (GstFastSpectrum * spectrum)
{
  GST_DEBUG_OBJECT (spectrum, "resetting state");

  gst_fastspectrum_free_channel_data (spectrum);
  gst_fastspectrum_flush (spectrum);
}

static void
gst_fastspectrum_finalize (GObject * object)
{
  GstFastSpectrum *spectrum = GST_FASTSPECTRUM (object);

  gst_fastspectrum_reset_state (spectrum);
  g_mutex_clear (&spectrum->lock);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_fastspectrum_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstFastSpectrum *filter = GST_FASTSPECTRUM (object);

  switch (prop_id) {
    case PROP_INTERVAL:{
      guint64 interval = g_value_get_uint64 (value);
      g_mutex_lock (&filter->lock);
      if (filter->interval != interval) {
        filter->interval = interval;
        gst_fastspectrum_reset_state (filter);
      }
      g_mutex_unlock (&filter->lock);
      break;
    }
    case PROP_BANDS:{
      guint bands = g_value_get_uint (value);
      g_mutex_lock (&filter->lock);
      if (filter->bands != bands) {
        filter->bands = bands;
        gst_fastspectrum_reset_state (filter);
      }
      g_mutex_unlock (&filter->lock);
      break;
    }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_fastspectrum_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstFastSpectrum *filter = GST_FASTSPECTRUM (object);

  switch (prop_id) {
    case PROP_INTERVAL:
      g_value_set_uint64 (value, filter->interval);
      break;
    case PROP_BANDS:
      g_value_set_uint (value, filter->bands);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_fastspectrum_start (GstBaseTransform * trans)
{
  GstFastSpectrum *spectrum = GST_FASTSPECTRUM (trans);

  gst_fastspectrum_reset_state (spectrum);

  return TRUE;
}

static gboolean
gst_fastspectrum_stop (GstBaseTransform * trans)
{
  GstFastSpectrum *spectrum = GST_FASTSPECTRUM (trans);

  gst_fastspectrum_reset_state (spectrum);

  return TRUE;
}

/* mixing data readers */

static void
input_data_mixed_float(const guint8* _in, double* out, guint len,
                       double max_value, guint op, guint nfft)
{
  guint j, ip = 0;
  gfloat *in = (gfloat *) _in;

  for (j = 0; j < len; j++) {
    out[op] = in[ip++];
    op = (op + 1) % nfft;
  }
}

static void
input_data_mixed_double (const guint8 * _in, double* out, guint len,
    double max_value, guint op, guint nfft)
{
  guint j, ip = 0;
  gdouble *in = (gdouble *) _in;

  for (j = 0; j < len; j++) {
    out[op] = in[ip++];
    op = (op + 1) % nfft;
  }
}

static void
input_data_mixed_int32_max (const guint8 * _in, double* out, guint len,
    double max_value, guint op, guint nfft)
{
  guint j, ip = 0;
  gint32 *in = (gint32 *) _in;

  for (j = 0; j < len; j++) {
    out[op] = in[ip++] / max_value;
    op = (op + 1) % nfft;
  }
}

static void
input_data_mixed_int24_max (const guint8 * _in, double* out, guint len,
    double max_value, guint op, guint nfft)
{
  guint j;

  for (j = 0; j < len; j++) {
#if G_BYTE_ORDER == G_BIG_ENDIAN
    gint32 value = GST_READ_UINT24_BE (_in);
#else
    gint32 value = GST_READ_UINT24_LE (_in);
#endif
    if (value & 0x00800000)
      value |= 0xff000000;

    out[op] = value / max_value;
    op = (op + 1) % nfft;
    _in += 3;
  }
}

static void
input_data_mixed_int16_max (const guint8 * _in, double * out, guint len,
    double max_value, guint op, guint nfft)
{
  guint j, ip = 0;
  gint16 *in = (gint16 *) _in;

  for (j = 0; j < len; j++) {
    out[op] = in[ip++] / max_value;
    op = (op + 1) % nfft;
  }
}

static gboolean
gst_fastspectrum_setup (GstAudioFilter * base, const GstAudioInfo * info)
{
  GstFastSpectrum *spectrum = GST_FASTSPECTRUM (base);
  GstFastSpectrumInputData input_data = NULL;

  g_mutex_lock (&spectrum->lock);
  switch (GST_AUDIO_INFO_FORMAT (info)) {
    case GST_AUDIO_FORMAT_S16:
      input_data = input_data_mixed_int16_max;
      break;
    case GST_AUDIO_FORMAT_S24:
      input_data = input_data_mixed_int24_max;
      break;
    case GST_AUDIO_FORMAT_S32:
      input_data = input_data_mixed_int32_max;
      break;
    case GST_AUDIO_FORMAT_F32:
      input_data = input_data_mixed_float;
      break;
    case GST_AUDIO_FORMAT_F64:
      input_data = input_data_mixed_double;
      break;
    default:
      g_assert_not_reached ();
      break;
  }
  spectrum->input_data = input_data;

  gst_fastspectrum_reset_state (spectrum);
  g_mutex_unlock (&spectrum->lock);

  return TRUE;
}

static void
gst_fastspectrum_run_fft (GstFastSpectrum * spectrum, guint input_pos)
{
  guint i;
  guint bands = spectrum->bands;
  guint nfft = 2 * bands - 2;

  for (i = 0; i < nfft; i++)
    spectrum->fft_input[i] =
        spectrum->input_ring_buffer[(input_pos + i) % nfft];

  // Should be safe to execute the same plan multiple times in parallel.
  fftw_execute(spectrum->plan);

  gdouble val;
  /* Calculate magnitude in db */
  for (i = 0; i < bands; i++) {
    val = spectrum->fft_output[i][0] * spectrum->fft_output[i][0];
    val += spectrum->fft_output[i][1] * spectrum->fft_output[i][1];
    val /= nfft * nfft;
    spectrum->spect_magnitude[i] += val;
  }
}

static GstFlowReturn
gst_fastspectrum_transform_ip (GstBaseTransform * trans, GstBuffer * buffer)
{
  GstFastSpectrum *spectrum = GST_FASTSPECTRUM (trans);
  guint rate = GST_AUDIO_FILTER_RATE (spectrum);
  guint bps = GST_AUDIO_FILTER_BPS (spectrum);
  guint bpf = GST_AUDIO_FILTER_BPF (spectrum);
  double max_value = (1UL << ((bps << 3) - 1)) - 1;
  guint bands = spectrum->bands;
  guint nfft = 2 * bands - 2;
  guint input_pos;
  GstMapInfo map;
  const guint8 *data;
  gsize size;
  guint fft_todo, msg_todo, block_size;
  gboolean have_full_interval;
  GstFastSpectrumInputData input_data;

  g_mutex_lock (&spectrum->lock);
  gst_buffer_map (buffer, &map, GST_MAP_READ);
  data = map.data;
  size = map.size;

  GST_LOG_OBJECT (spectrum, "input size: %" G_GSIZE_FORMAT " bytes", size);

  if (GST_BUFFER_IS_DISCONT (buffer)) {
    GST_DEBUG_OBJECT (spectrum, "Discontinuity detected -- flushing");
    gst_fastspectrum_flush (spectrum);
  }

  /* If we don't have a FFT context yet (or it was reset due to parameter
   * changes) get one and allocate memory for everything
   */
  if (!spectrum->channel_data_initialised) {
    GST_DEBUG_OBJECT (spectrum, "allocating for bands %u", bands);

    gst_fastspectrum_alloc_channel_data (spectrum);

    /* number of sample frames we process before posting a message
     * interval is in ns */
    spectrum->frames_per_interval =
        gst_util_uint64_scale (spectrum->interval, rate, GST_SECOND);
    spectrum->frames_todo = spectrum->frames_per_interval;
    /* rounding error for frames_per_interval in ns,
     * aggregated it in accumulated_error */
    spectrum->error_per_interval = (spectrum->interval * rate) % GST_SECOND;
    if (spectrum->frames_per_interval == 0)
      spectrum->frames_per_interval = 1;

    GST_INFO_OBJECT (spectrum, "interval %" GST_TIME_FORMAT ", fpi %"
        G_GUINT64_FORMAT ", error %" GST_TIME_FORMAT,
        GST_TIME_ARGS (spectrum->interval), spectrum->frames_per_interval,
        GST_TIME_ARGS (spectrum->error_per_interval));

    spectrum->input_pos = 0;

    gst_fastspectrum_flush (spectrum);
  }

  if (spectrum->num_frames == 0)
    spectrum->message_ts = GST_BUFFER_TIMESTAMP (buffer);

  input_pos = spectrum->input_pos;
  input_data = spectrum->input_data;

  while (size >= bpf) {
    /* run input_data for a chunk of data */
    fft_todo = nfft - (spectrum->num_frames % nfft);
    msg_todo = spectrum->frames_todo - spectrum->num_frames;
    GST_LOG_OBJECT (spectrum,
        "message frames todo: %u, fft frames todo: %u, input frames %"
        G_GSIZE_FORMAT, msg_todo, fft_todo, (size / bpf));
    block_size = msg_todo;
    if (block_size > (size / bpf))
      block_size = (size / bpf);
    if (block_size > fft_todo)
      block_size = fft_todo;

    /* Move the current frames into our ringbuffers */
    input_data(data, spectrum->input_ring_buffer, block_size, max_value, input_pos, nfft);

    data += block_size * bpf;
    size -= block_size * bpf;
    input_pos = (input_pos + block_size) % nfft;
    spectrum->num_frames += block_size;

    have_full_interval = (spectrum->num_frames == spectrum->frames_todo);

    GST_LOG_OBJECT (spectrum,
        "size: %" G_GSIZE_FORMAT ", do-fft = %d, do-message = %d", size,
        (spectrum->num_frames % nfft == 0), have_full_interval);

    /* If we have enough frames for an FFT or we have all frames required for
     * the interval and we haven't run a FFT, then run an FFT */
    if ((spectrum->num_frames % nfft == 0) ||
        (have_full_interval && !spectrum->num_fft)) {
      gst_fastspectrum_run_fft (spectrum, input_pos);
      spectrum->num_fft++;
    }

    /* Do we have the FFTs for one interval? */
    if (have_full_interval) {
      GST_DEBUG_OBJECT (spectrum, "nfft: %u frames: %" G_GUINT64_FORMAT
          " fpi: %" G_GUINT64_FORMAT " error: %" GST_TIME_FORMAT, nfft,
          spectrum->num_frames, spectrum->frames_per_interval,
          GST_TIME_ARGS (spectrum->accumulated_error));

      spectrum->frames_todo = spectrum->frames_per_interval;
      if (spectrum->accumulated_error >= GST_SECOND) {
        spectrum->accumulated_error -= GST_SECOND;
        spectrum->frames_todo++;
      }
      spectrum->accumulated_error += spectrum->error_per_interval;

      if (spectrum->output_callback) {
        // Calculate average
        for (guint i = 0; i < spectrum->bands; i++) {
          spectrum->spect_magnitude[i] /= spectrum->num_fft;
        }

        spectrum->output_callback(spectrum->spect_magnitude, spectrum->bands);

        // Reset spectrum accumulators
        memset(spectrum->spect_magnitude, 0, spectrum->bands * sizeof(double));
      }

      if (GST_CLOCK_TIME_IS_VALID (spectrum->message_ts))
        spectrum->message_ts +=
            gst_util_uint64_scale (spectrum->num_frames, GST_SECOND, rate);

      spectrum->num_frames = 0;
      spectrum->num_fft = 0;
    }
  }

  spectrum->input_pos = input_pos;

  gst_buffer_unmap (buffer, &map);
  g_mutex_unlock (&spectrum->lock);

  g_assert (size == 0);

  return GST_FLOW_OK;
}
