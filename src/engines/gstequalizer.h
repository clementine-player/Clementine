// PCM time-domain equalizer
// (c) 2002 Felipe Rivera <liebremx at users sourceforge net>
// (c) 2004 Mark Kretschmann <markey@web.de>
// License: GPL V2

#ifndef AMAROK_GST_EQUALIZER_H
#define AMAROK_GST_EQUALIZER_H

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>

G_BEGIN_DECLS

#define BAND_NUM 10
#define EQ_MAX_BANDS 10
#define EQ_CHANNELS 2

#define GST_TYPE_EQUALIZER \
  (gst_equalizer_get_type())
#define GST_EQUALIZER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_EQUALIZER,GstEqualizer))
#define GST_EQUALIZER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_EQUALIZER,GstEqualizerClass))
#define GST_IS_EQUALIZER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_EQUALIZER))
#define GST_IS_EQUALIZER_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_EQUALIZER))

typedef struct _GstEqualizer GstEqualizer;
typedef struct _GstEqualizerClass GstEqualizerClass;

// Floating point
typedef struct
{
    float beta;
    float alpha;
    float gamma;
} sIIRCoefficients;

/* Coefficient history for the IIR filter */
typedef struct
{
    float x[3]; /* x[n], x[n-1], x[n-2] */
    float y[3]; /* y[n], y[n-1], y[n-2] */
} sXYData;


struct _GstEqualizer
{
    // Do not remove
    GstBaseTransform element;

    GstPad *srcpad;
    GstPad *sinkpad;

    int samplerate;
    int channels;

    // Properties
    bool active;
    // Gain for each band
    // values should be between -0.2 and 1.0
    float gain[EQ_MAX_BANDS][EQ_CHANNELS] __attribute__((aligned));
    // Volume gain
    // values should be between 0.0 and 1.0
    float preamp[EQ_CHANNELS] __attribute__((aligned));

    // Coefficients
    sIIRCoefficients* iir_cf;

    sXYData data_history[EQ_MAX_BANDS][EQ_CHANNELS] __attribute__((aligned));
};

struct _GstEqualizerClass
{
    GstBaseTransformClass parent_class;

    /* signals */
};

void gst_equalizer_set_property( GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec );
void gst_equalizer_get_property( GObject * object, guint prop_id, GValue * value, GParamSpec * pspec );
void set_filters( GstEqualizer* obj );
GType gst_equalizer_get_type( void );
GstEqualizer* gst_equalizer_new();


G_END_DECLS


#endif /* AMAROK_GST_EQUALIZER_H */
