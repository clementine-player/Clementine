// PCM time-domain equalizer
// (c) 2002 Felipe Rivera <liebremx at users sourceforge net>
// (c) 2004 Mark Kretschmann <markey@web.de>
// (c) 2006 Jakub Stachowski <qbast@go2.pl>
// License: GPL V2


#include "gstequalizer.h"
#include "iir_cf.h"         // IIR filter coefficients

#include <cmath>
#include <cstring>
#include <vector>

#include <QtDebug>


GST_DEBUG_CATEGORY_STATIC ( gst_equalizer_debug );
#define GST_CAT_DEFAULT gst_equalizer_debug

/* signals and args */
enum {
  LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_ACTIVE,
  ARG_PREAMP,
  ARG_GAIN
};

static GstElementDetails gst_equalizer_details = {
  const_cast<char*>("Equalizer"),
  const_cast<char*>("Filter/Effect/Audio"),
  const_cast<char*>("Parametric Equalizer"),
  const_cast<char*>("Mark Kretschmann <markey@web.de>") };

GstStaticPadTemplate sink_template =
    GST_STATIC_PAD_TEMPLATE ( (gchar*) "sink",
                              GST_PAD_SINK,
                              GST_PAD_ALWAYS,
                              GST_STATIC_CAPS (
                                  "audio/x-raw-int, " \
                                  "rate = (int) [ 1, MAX ], " \
                                  "channels = (int) [ 1, MAX ], " \
                                  "endianness = (int) { LITTLE_ENDIAN, BIG_ENDIAN }, " \
                                  "width = (int) 16, " \
                                  "depth = (int) 16, " \
                                  "signed = (bool) TRUE "
                                  )
                              );

GstStaticPadTemplate src_template =
    GST_STATIC_PAD_TEMPLATE (  (gchar*) "src",
                               GST_PAD_SRC,
                               GST_PAD_ALWAYS,
                               GST_STATIC_CAPS (
                                   "audio/x-raw-int, " \
                                   "rate = (int) [ 1, MAX ], " \
                                   "channels = (int) [ 1, MAX ], " \
                                   "endianness = (int) { LITTLE_ENDIAN, BIG_ENDIAN }, " \
                                   "width = (int) 16, " \
                                   "depth = (int) 16, " \
                                   "signed = (bool) TRUE"
                                   )
                               );

// static guint gst_equalizer_signals[ LAST_SIGNAL ] = { 0 };

#define _do_init(bla) \
GST_DEBUG_CATEGORY_INIT (gst_equalizer_debug, "equalizer", 0, "equalizer element");

GST_BOILERPLATE_FULL ( GstEqualizer, gst_equalizer, GstBaseTransform, GST_TYPE_BASE_TRANSFORM, _do_init );

static gboolean gst_equalizer_set_caps (GstBaseTransform * base, GstCaps * incaps, GstCaps * outcaps);
static GstFlowReturn gst_equalizer_transform_ip (GstBaseTransform * base, GstBuffer * outbuf);

/////////////////////////////////////////////////////////////////////////////////////
// INIT
/////////////////////////////////////////////////////////////////////////////////////

void gst_equalizer_base_init(gpointer g_class) {
  GstElementClass * gstelement_class = GST_ELEMENT_CLASS ( g_class );
  gst_element_class_add_pad_template (gstelement_class,
                                      gst_static_pad_template_get (&src_template));
  gst_element_class_add_pad_template (gstelement_class,
                                      gst_static_pad_template_get (&sink_template));
  gst_element_class_set_details ( gstelement_class, &gst_equalizer_details );
}


void gst_equalizer_class_init(GstEqualizerClass* klass) {
  GObjectClass* gobject_class;
  gobject_class = G_OBJECT_CLASS( klass );

  gobject_class->set_property = gst_equalizer_set_property;
  gobject_class->get_property = gst_equalizer_get_property;

  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_ACTIVE, g_param_spec_boolean ("active", "active", "active", false, (GParamFlags)G_PARAM_READWRITE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_PREAMP, g_param_spec_int ("preamp", "preamp", "preamp", 0, 100, 0, (GParamFlags)G_PARAM_READWRITE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_GAIN, g_param_spec_pointer ("gain", "gain", "gain", (GParamFlags)G_PARAM_WRITABLE));
  GST_BASE_TRANSFORM_CLASS (klass)->transform_ip =
      GST_DEBUG_FUNCPTR (gst_equalizer_transform_ip);
  GST_BASE_TRANSFORM_CLASS (klass)->set_caps =
      GST_DEBUG_FUNCPTR (gst_equalizer_set_caps);
}


void gst_equalizer_init(GstEqualizer* obj, GstEqualizerClass* klass) {
  // Properties
  obj->active = false;
}


/////////////////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
/////////////////////////////////////////////////////////////////////////////////////

static gboolean gst_equalizer_set_caps (GstBaseTransform* base, GstCaps* incaps,
                                        GstCaps* outcaps) {
  GstEqualizer* obj = GST_EQUALIZER (base);
  GstStructure* structure = gst_caps_get_structure (incaps, 0);

  /* Since we're an audio filter, we want to handle raw audio
    * and from that audio type, we need to get the samplerate and
    * number of channels. */

  gst_structure_get_int (structure, "rate", &obj->samplerate);
  gst_structure_get_int (structure, "channels", &obj->channels);
  // Load the correct filter table according to the sampling rate
  set_filters( obj );
  /* Zero the history arrays */
  memset(obj->data_history, 0, sizeof(sXYData) * EQ_MAX_BANDS * EQ_CHANNELS);
  qDebug() << "Samplerate " << obj->samplerate << ", channels: " << obj->channels;
  return TRUE;
}



void gst_equalizer_set_property(GObject* object, guint prop_id,
                                const GValue * value, GParamSpec* pspec) {
  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail ( GST_IS_EQUALIZER ( object ) );

  GstEqualizer* obj = GST_EQUALIZER ( object );
  std::vector<int>* gains;

  switch (prop_id) {
  case ARG_ACTIVE:
    obj->active = g_value_get_boolean (value);
    break;

  case ARG_PREAMP:
    for ( int chan = 0; chan < EQ_CHANNELS; chan++ )
      obj->preamp[chan] = (float)g_value_get_int(value) * 0.01;
    break;

  case ARG_GAIN:
    gains = (std::vector<int>*) g_value_get_pointer(value);
    for ( int band = 0; band < BAND_NUM; band++ )
      for ( int chan = 0; chan < EQ_CHANNELS; chan++ )
        obj->gain[band][chan] = (float)(*gains)[band] * 0.012 - 0.2;
    break;

        default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID ( object, prop_id, pspec );
    break;
  }
}


void gst_equalizer_get_property(GObject* object, guint prop_id, GValue* value,
                                GParamSpec* pspec) {
  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail ( GST_IS_EQUALIZER ( object ) );

  GstEqualizer* obj = GST_EQUALIZER ( object );

  switch (prop_id) {
  case ARG_ACTIVE:
    g_value_set_boolean (value, obj->active);
    break;

  case ARG_PREAMP:
    g_value_set_int(value, obj->preamp[0] * 100);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID ( object, prop_id, pspec );
    break;
  }
}


void set_filters(GstEqualizer* obj) {
  Q_UNUSED(iir_cforiginal10_44100);
  Q_UNUSED(iir_cforiginal10_48000);
  Q_UNUSED(iir_cf15_44100);
  Q_UNUSED(iir_cf15_48000);
  Q_UNUSED(iir_cf25_44100);
  Q_UNUSED(iir_cf25_48000);
  Q_UNUSED(iir_cf31_44100);
  Q_UNUSED(iir_cf31_48000);

  switch(obj->samplerate) {
  case 11025:
    obj->iir_cf = iir_cf10_11k_11025;
    break;

  case 22050:
    obj->iir_cf = iir_cf10_22k_22050;
    break;

  case 48000:
    obj->iir_cf = iir_cf10_48000;
    break;

  default:
    obj->iir_cf = iir_cf10_44100;
    break;
  }
}

static GstFlowReturn gst_equalizer_transform_ip(GstBaseTransform* base,
                                                GstBuffer * outbuf) {
  GstEqualizer* obj = GST_EQUALIZER ( base );

  if ( !obj->active ) return GST_FLOW_OK;

  gint16 *data = (gint16*)GST_BUFFER_DATA ( outbuf );
  gint length = GST_BUFFER_SIZE( outbuf );

  /* Indexes for the history arrays
     * These have to be kept between calls to this function
     * hence they are static */
  static gint i = 0, j = 2, k = 1;

  gint index, band, channel;
  gint halflength;
  float out[EQ_CHANNELS], pcm[EQ_CHANNELS];
  float tempfloat;

  /**
     * IIR filter equation is
     * y[n] = 2 * (alpha*(x[n]-x[n-2]) + gamma*y[n-1] - beta*y[n-2])
     *
     * NOTE: The 2 factor was introduced in the coefficients to save
     * 			a multiplication
     *
     * This algorithm cascades two filters to get nice filtering
     * at the expense of extra CPU cycles
     */
  /* 16bit, 2 bytes per sample, so divide by two the length of
     * the buffer (length is in bytes)
     */
  halflength = (length >> 1);
  for (index = 0; index < halflength; index+=2) {
    /* For each channel */
    for (channel = 0; channel < obj->channels; channel++) {
      pcm[channel] = data[index+channel];
      /* Preamp gain */
      pcm[channel] *= obj->preamp[channel];

      out[channel] = 0.;
      /* For each band */
      for (band = 0; band < BAND_NUM; band++) {
        /* Store Xi(n) */
        obj->data_history[band][channel].x[i] = pcm[channel];
        /* Calculate and store Yi(n) */
        obj->data_history[band][channel].y[i] =
            (
                /* = alpha * [x(n)-x(n-2)] */
                obj->iir_cf[band].alpha * ( obj->data_history[band][channel].x[i]
                                            -  obj->data_history[band][channel].x[k])
                /* + gamma * y(n-1) */
                + obj->iir_cf[band].gamma * obj->data_history[band][channel].y[j]
                /* - beta * y(n-2) */
                - obj->iir_cf[band].beta * obj->data_history[band][channel].y[k]
                );
        /*
                 * The multiplication by 2.0 was 'moved' into the coefficients to save
                 * CPU cycles here */
        /* Apply the gain  */
        out[channel] +=  obj->data_history[band][channel].y[i]*obj->gain[band][channel]; // * 2.0;
      } /* For each band */

      /* Volume stuff
               Scale down original PCM sample and add it to the filters
               output. This substitutes the multiplication by 0.25
               Go back to use the floating point multiplication before the
               conversion to give more dynamic range
               */
      out[channel] += pcm[channel]*0.25;

      /* Round and convert to integer and limit the output */
      tempfloat = out[channel] < -32768.0 ? -32768.0 : out[channel];
      tempfloat = tempfloat > 32767.0 ? 32767.0 : tempfloat;
      data[index+channel] = (gint)rintf(tempfloat);

    } /* For each channel */

    i++; j++; k++;

    /* Wrap around the indexes */
    if (i == 3) i = 0;
    else if (j == 3) j = 0;
    else k = 0;
  }/* For each pair of samples */

  return GST_FLOW_OK;
}



GstEqualizer* gst_equalizer_new() {
  GstEqualizer* obj = GST_EQUALIZER ( g_object_new ( GST_TYPE_EQUALIZER, NULL ) );
  gst_object_set_name( (GstObject*) obj, "Equalizer" );

  return obj;
}

