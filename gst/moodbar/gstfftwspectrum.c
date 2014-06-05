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

/**
 * SECTION:element-fftwspectrum
 *
 * <refsect2>
 * <title>Example launch line</title>
 * <para>
 * <programlisting>
 * gst-launch audiotestsrc ! audioconvert ! fftwspectrum ! fftwunspectrum ! audioconvert ! alsasink
 * </programlisting>
 * </para>
 * </refsect2>
 */

/* This is a simple plugin to take an audio signal and return its
 * Fourier transform, using fftw3.  It takes a specified number N of
 * samples and returns the first N/2+1 (complex) Fourier transform
 * values (the other half of the values being the complex conjugates
 * of the first).  The modulus of these values correspond to the
 * strength of the signal in their various bands, and the phase gives
 * information about the phase of the signal.  The step by which the
 * transform increments is also variable, so it can return redundant
 * data (to reduce artifacts when converting back into a signal).
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <fftw3.h>
#include <string.h>
#include <math.h>

#include "gstfftwspectrum.h"
#include "spectrum.h"

GST_DEBUG_CATEGORY (gst_fftwspectrum_debug);
#define GST_CAT_DEFAULT gst_fftwspectrum_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

/* The size and step arguments are actually only default values
 * used to fixate the size and step properties of the source cap.
 */
enum
{
  ARG_0,
  ARG_DEF_SIZE,
  ARG_DEF_STEP,
  ARG_HIQUALITY
};

#define DEF_SIZE_DEFAULT      1024
#define DEF_STEP_DEFAULT      512
#define HIQUALITY_DEFAULT     TRUE

static GstStaticPadTemplate sink_factory 
  = GST_STATIC_PAD_TEMPLATE ("sink",
			     GST_PAD_SINK,
			     GST_PAD_ALWAYS,
			     GST_STATIC_CAPS 
			       ( SPECTRUM_SIGNAL_CAPS )
			     );

/* See spectrum.h for a definition of the frequency caps */
static GstStaticPadTemplate src_factory 
  = GST_STATIC_PAD_TEMPLATE ("src",
			     GST_PAD_SRC,
			     GST_PAD_ALWAYS,
			     GST_STATIC_CAPS
			       ( SPECTRUM_FREQ_CAPS )
			     );

GST_BOILERPLATE (GstFFTWSpectrum, gst_fftwspectrum, GstElement,
    GST_TYPE_ELEMENT);

static void gst_fftwspectrum_set_property (GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec);
static void gst_fftwspectrum_get_property (GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec);

static gboolean gst_fftwspectrum_set_sink_caps (GstPad *pad, GstCaps *caps);
static gboolean gst_fftwspectrum_set_src_caps (GstPad *pad, GstCaps *caps);
static void     gst_fftwspectrum_fixatecaps (GstPad *pad, GstCaps *caps);
static GstCaps *gst_fftwspectrum_getcaps (GstPad *pad);

static GstFlowReturn gst_fftwspectrum_chain (GstPad *pad, GstBuffer *buf);
static GstStateChangeReturn gst_fftwspectrum_change_state (GstElement *element,
    GstStateChange transition);


#define OUTPUT_SIZE(conv) (((conv)->size/2+1)*sizeof(fftw_complex))


/***************************************************************/
/* GObject boilerplate stuff                                   */
/***************************************************************/


static void
gst_fftwspectrum_base_init (gpointer gclass)
{
  static GstElementDetails element_details = 
    {
      "FFTW-based Fourier transform",
      "Filter/Converter/Spectrum",
      "Convert a raw audio stream into a frequency spectrum",
      "Joe Rabinoff <bobqwatson@yahoo.com>"
    };
  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_factory));
  gst_element_class_set_details (element_class, &element_details);
}

/* initialize the plugin's class */
static void
gst_fftwspectrum_class_init (GstFFTWSpectrumClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_fftwspectrum_set_property;
  gobject_class->get_property = gst_fftwspectrum_get_property;

  g_object_class_install_property (gobject_class, ARG_DEF_SIZE,
      g_param_spec_int ("def-size", "Default Size", 
	  "Apply a Fourier transform to this many samples at a time (default value)",
	  1, G_MAXINT32, DEF_SIZE_DEFAULT, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, ARG_DEF_STEP,
      g_param_spec_int ("def-step", "Default Step", 
	  "Advance the stream this many samples each time (default value)",
	  1, G_MAXINT32, DEF_STEP_DEFAULT, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, ARG_HIQUALITY,
      g_param_spec_boolean ("hiquality", "High Quality", 
	  "Use a more time-consuming, higher quality algorithm chooser",
	  HIQUALITY_DEFAULT, G_PARAM_READWRITE));

  gstelement_class->change_state 
    = GST_DEBUG_FUNCPTR (gst_fftwspectrum_change_state);
}

/* initialize the new element
 * instantiate pads and add them to element
 * set functions
 * initialize structure
 */
static void
gst_fftwspectrum_init (GstFFTWSpectrum * conv,
		       GstFFTWSpectrumClass * gclass)
{
  GstElementClass *klass = GST_ELEMENT_GET_CLASS (conv);

  conv->sinkpad =
      gst_pad_new_from_template 
          (gst_element_class_get_pad_template (klass, "sink"), "sink");
  gst_pad_set_setcaps_function (conv->sinkpad, 
      GST_DEBUG_FUNCPTR (gst_fftwspectrum_set_sink_caps));
  gst_pad_set_getcaps_function (conv->sinkpad,
      GST_DEBUG_FUNCPTR (gst_fftwspectrum_getcaps));
  gst_pad_set_chain_function (conv->sinkpad, 
      GST_DEBUG_FUNCPTR (gst_fftwspectrum_chain));

  conv->srcpad =
      gst_pad_new_from_template 
          (gst_element_class_get_pad_template (klass, "src"), "src");
  gst_pad_set_setcaps_function (conv->srcpad, 
      GST_DEBUG_FUNCPTR (gst_fftwspectrum_set_src_caps));
  gst_pad_set_getcaps_function (conv->srcpad,
      GST_DEBUG_FUNCPTR (gst_fftwspectrum_getcaps));
  gst_pad_set_fixatecaps_function (conv->srcpad,
      GST_DEBUG_FUNCPTR (gst_fftwspectrum_fixatecaps));


  gst_element_add_pad (GST_ELEMENT (conv), conv->sinkpad);
  gst_element_add_pad (GST_ELEMENT (conv), conv->srcpad);

  /* These are set once the (source) capabilities are determined */
  conv->rate = 0;
  conv->size = 0;
  conv->step = 0;
  
  /* These are set when we change to READY */
  conv->fftw_in   = NULL;
  conv->fftw_out  = NULL;
  conv->fftw_plan = NULL;

  /* These are set when we start receiving data */
  conv->samples    = NULL;
  conv->numsamples = 0;
  conv->timestamp  = 0;
  conv->offset     = 0;

  /* Properties */
  conv->def_size = DEF_SIZE_DEFAULT;
  conv->def_step = DEF_STEP_DEFAULT;
  conv->hi_q     = HIQUALITY_DEFAULT;
}

static void
gst_fftwspectrum_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstFFTWSpectrum *conv = GST_FFTWSPECTRUM (object);

  switch (prop_id) 
    {
    case ARG_DEF_SIZE:
      conv->def_size = g_value_get_int (value);
      break;
    case ARG_DEF_STEP:
      conv->def_step = g_value_get_int (value);
      break;
    case ARG_HIQUALITY:
      conv->hi_q = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gst_fftwspectrum_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstFFTWSpectrum *conv = GST_FFTWSPECTRUM (object);

  switch (prop_id) 
    {
    case ARG_DEF_SIZE:
      g_value_set_int (value, conv->def_size);
      break;
    case ARG_DEF_STEP:
      g_value_set_int (value, conv->def_step);
      break;
    case ARG_HIQUALITY:
      g_value_set_boolean (value, conv->hi_q);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


/* Allocate and deallocate fftw state data */
static void
free_fftw_data (GstFFTWSpectrum *conv)
{
  if(conv->fftw_plan != NULL)
    fftw_destroy_plan (conv->fftw_plan);
  if(conv->fftw_in != NULL)
    fftw_free (conv->fftw_in);
  if(conv->fftw_out != NULL)
    fftw_free (conv->fftw_out);

  conv->fftw_in   = NULL;
  conv->fftw_out  = NULL;
  conv->fftw_plan = NULL;
}

static void
alloc_fftw_data (GstFFTWSpectrum *conv)
{
  free_fftw_data (conv);

  GST_DEBUG ("Allocating data for size = %d and step = %d",
	     conv->size, conv->step);

  conv->fftw_in  = (double *) fftw_malloc (sizeof(double) * conv->size);
  conv->fftw_out = (double *) fftw_malloc (OUTPUT_SIZE (conv));
  
  /* We use the simplest real-to-complex algorithm, which takes n real
   * inputs and returns floor(n/2) + 1 complex outputs (the other n/2
   * outputs are the hermetian conjugates).  This should be optimal for
   * implementing filters.
   */

  static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
  g_static_mutex_lock(&mutex);
  conv->fftw_plan 
    = fftw_plan_dft_r2c_1d(conv->size, conv->fftw_in,
          (fftw_complex *) conv->fftw_out,
			    conv->hi_q ? FFTW_MEASURE : FFTW_ESTIMATE);
  g_static_mutex_unlock(&mutex);
}


/***************************************************************/
/* Capabilities negotiation                                    */
/***************************************************************/

/* The input and output capabilities are only related by the "rate"
 * parameter, which is propagated so that an audio signal can be
 * reconstructed eventually.  This module does no rate conversion.
 * 
 * The way I understand it, there are two times when caps negotiation
 * takes place: (1) when a sink pad receives either its first buffer,
 * or a buffer with a new caps type, and (2) when a source pad request
 * a buffer from something downstream, and the returned allocated
 * buffer has different caps from the ones already negotiated.  In the
 * first case, _set_sink_caps is called, and in the second, _set_src_caps
 * is called.
 *   When (1) occurs, we remember the rate (the only variable parameter
 * in the source) and set the source caps.  Then _set_src_caps is called.
 * In _set_src_caps, we check that the rate hasn't changed, and figure out
 * or remember appropriate size and step attributes.  If _set_src_caps is
 * called from _set_sink_caps, this completes our setting up our internal
 * configuration; if it is called from (2), we reconfigure just the source
 * part of the internal configuration.
 */

static gboolean
gst_fftwspectrum_set_sink_caps (GstPad * pad, GstCaps * caps)
{
  GstFFTWSpectrum *conv;
  GstCaps *srccaps, *newsrccaps;
  GstStructure *newstruct;
  gint rate;
  gboolean res;

  conv = GST_FFTWSPECTRUM (gst_pad_get_parent (pad));

  srccaps = gst_pad_get_allowed_caps (conv->srcpad);
  newsrccaps = gst_caps_copy_nth (srccaps, 0);
  gst_caps_unref (srccaps);

  newstruct = gst_caps_get_structure (caps, 0);
  if (!gst_structure_get_int (newstruct, "rate", &rate))
    {
      gst_caps_unref (newsrccaps);
      gst_object_unref (conv);
      return FALSE;
    }

  /* Fixate the source caps with the given rate */
  gst_caps_set_simple (newsrccaps, "rate", G_TYPE_INT, rate, NULL);
  gst_pad_fixate_caps (conv->srcpad, newsrccaps);
  conv->rate = rate;
  res = gst_pad_set_caps (conv->srcpad, newsrccaps);
  if (!res)
    conv->rate = 0;
  
  gst_caps_unref (newsrccaps);
  gst_object_unref (conv);

  return res;
}

static gboolean
gst_fftwspectrum_set_src_caps (GstPad * pad, GstCaps * caps)
{
  GstFFTWSpectrum *conv;
  gboolean res = FALSE;
  GstStructure *newstruct;
  gint rate;

  conv = GST_FFTWSPECTRUM (gst_pad_get_parent (pad));

  newstruct = gst_caps_get_structure (caps, 0);
  if (!gst_structure_get_int (newstruct, "rate", &rate))
    goto out;

  /* Assume caps negotiation has already taken place */
  if (rate == conv->rate)
    {
      gint size, step;

      if (!gst_structure_get_int (newstruct, "size", &size))
	goto out;
      if (!gst_structure_get_int (newstruct, "step", &step))
	goto out;

      if (conv->size != size  ||  conv->step != step)
	{
	  conv->size = size;
	  conv->step = step;
	  
	  /* Re-allocate the fftw data */
	  if (GST_STATE (GST_ELEMENT (conv)) >= GST_STATE_READY)
	    alloc_fftw_data (conv);
	}

      res = TRUE;
    }

 out:
  gst_object_unref (conv);

  return res;
}


/* The only thing that can constrain the caps is the rate. */ 
static GstCaps *
gst_fftwspectrum_getcaps (GstPad *pad)
{
  GstFFTWSpectrum *conv;
  GstCaps *tmplcaps;

  conv = GST_FFTWSPECTRUM (gst_pad_get_parent (pad));
  tmplcaps = gst_caps_copy (gst_pad_get_pad_template_caps (pad));
  
  if(conv->rate != 0)
    {
      /* Assumes the template caps are simple */
      gst_caps_set_simple (tmplcaps, "rate", G_TYPE_INT, conv->rate, NULL);
    }

  gst_object_unref (conv);  

  return tmplcaps;
}


/* This is called when the source pad needs to choose its capabilities
 * when it has a choice and nobody's forcing its hand.  In this case
 * we take our hint from the def_size and def_step properties.
 */
static void
gst_fftwspectrum_fixatecaps (GstPad *pad, GstCaps *caps)
{
  GstFFTWSpectrum *conv;
  GstStructure *s;
  const GValue *val;

  conv = GST_FFTWSPECTRUM (gst_pad_get_parent (pad));
  s = gst_caps_get_structure (caps, 0);

  val = gst_structure_get_value (s, "size");
  if (val == NULL)
    gst_caps_set_simple (caps, "size", G_TYPE_INT, conv->def_size, NULL);
  else if (G_VALUE_TYPE (val) == GST_TYPE_INT_RANGE)
    {
      gint sizemin, sizemax;
      sizemin = gst_value_get_int_range_min (val);
      sizemax = gst_value_get_int_range_max (val);
      gst_caps_set_simple (caps, "size", G_TYPE_INT, 
			   CLAMP (conv->def_size, sizemin, sizemax), NULL);
    }
  /* else it should be already fixed */
  
  val = gst_structure_get_value (s, "step");
  if (val == NULL)
    gst_caps_set_simple (caps, "step", G_TYPE_INT, conv->def_step, NULL);
  else if (G_VALUE_TYPE (val) == GST_TYPE_INT_RANGE)
    {
      gint stepmin, stepmax;
      stepmin = gst_value_get_int_range_min (val);
      stepmax = gst_value_get_int_range_max (val);
      gst_caps_set_simple (caps, "step", G_TYPE_INT, 
			   CLAMP (conv->def_step, stepmin, stepmax), NULL);
    }
  /* else it should be already fixed */

  /* Assume rate is already fixed (if not it'll be fixed by default) */

  gst_object_unref (conv);  
}


/***************************************************************/
/* Actual conversion                                           */
/***************************************************************/


static GstStateChangeReturn 
gst_fftwspectrum_change_state (GstElement * element,
			       GstStateChange transition)
{
  GstFFTWSpectrum *conv = GST_FFTWSPECTRUM (element);
  GstStateChangeReturn res;

  switch (transition)
    {
    case GST_STATE_CHANGE_NULL_TO_READY:
      alloc_fftw_data (conv);
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      conv->samples    = (gdouble *) g_malloc (sizeof(gdouble));
      conv->numsamples = 0;
      conv->timestamp  = 0;
      conv->offset     = 0;
      break;
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
      break;
    default:
      break;
    }

  res = parent_class->change_state (element, transition);

  switch (transition) 
    {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
      break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:      
      g_free(conv->samples);
      conv->samples    = NULL;
      conv->numsamples = 0;
      conv->timestamp  = 0;
      conv->offset     = 0;
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
      free_fftw_data (conv);
      break;
    default:
      break;
    }

  return res;
}


/* Adds the samples contained in buf to the end of conv->samples,
 * updating conv->numsamples.
 */
static void 
push_samples (GstFFTWSpectrum *conv, GstBuffer *buf)
{
  gint newsamples = GST_BUFFER_SIZE (buf) / sizeof (gdouble);
  gint oldsamples = conv->numsamples;

  conv->numsamples += newsamples;
  conv->samples = g_realloc (conv->samples, conv->numsamples * sizeof (gdouble));
  memcpy (&conv->samples[oldsamples], GST_BUFFER_DATA (buf), 
    newsamples * sizeof (gdouble));
  
  /* GST_LOG ("Added %d samples", newsamples); */
}

/* This basically does the opposite of push_samples, but takes samples
 * off the front.
 */
static void
shift_samples (GstFFTWSpectrum *conv, gint toshift)
{
  gdouble *oldsamples = conv->samples;

  conv->numsamples -= toshift;
  conv->samples = g_malloc (MAX (conv->numsamples, 1) * sizeof (double));
  memcpy (conv->samples, &oldsamples[toshift], 
    conv->numsamples * sizeof (gdouble));
  g_free (oldsamples);

  /* Fix the timestamp and offset */
  conv->timestamp 
    += gst_util_uint64_scale_int (GST_SECOND, toshift, conv->rate);
  conv->offset += toshift;

  /* GST_LOG ("Disposed of %d samples (time: %" GST_TIME_FORMAT " offset: %llu)",
     toshift, GST_TIME_ARGS(conv->timestamp), conv->offset); */
}


/* This function queues samples until there are at least
 * max (conv->size, conv->step) samples to process.  We
 * then process samples in chunks of conv->size and increment
 * by conv->step.
 */
static GstFlowReturn
gst_fftwspectrum_chain (GstPad * pad, GstBuffer * buf)
{
  GstFFTWSpectrum *conv;
  GstBuffer *outbuf;
  GstFlowReturn res = GST_FLOW_OK;

  conv = GST_FFTWSPECTRUM (gst_pad_get_parent (pad));

  push_samples (conv, buf);
  gst_buffer_unref (buf);

  while (conv->numsamples >= MAX (conv->size, conv->step))
    {
      res = gst_pad_alloc_buffer_and_set_caps 
	(conv->srcpad, conv->offset, OUTPUT_SIZE (conv), 
	 GST_PAD_CAPS(conv->srcpad), &outbuf);
      if (res != GST_FLOW_OK)
	break;
      
      GST_BUFFER_SIZE       (outbuf) = OUTPUT_SIZE (conv);
      GST_BUFFER_OFFSET     (outbuf) = conv->offset;
      GST_BUFFER_OFFSET_END (outbuf) = conv->offset + conv->step;
      GST_BUFFER_TIMESTAMP  (outbuf) = conv->timestamp;
      GST_BUFFER_DURATION   (outbuf) 
	= gst_util_uint64_scale_int (GST_SECOND, conv->step, conv->rate);
      
      /* Do the Fourier transform */
      memcpy (conv->fftw_in, conv->samples, conv->size * sizeof (double));
      fftw_execute (conv->fftw_plan);
      { /* Normalize */
	gint i;
	gfloat root = sqrtf (conv->size);
	for (i = 0; i < 2*(conv->size/2+1); ++i)
	  conv->fftw_out[i] /= root;
      }
      memcpy (GST_BUFFER_DATA (outbuf), conv->fftw_out, OUTPUT_SIZE (conv));
      
      res = gst_pad_push (conv->srcpad, outbuf);
      
      shift_samples (conv, conv->step);

      if (res != GST_FLOW_OK)
	break;
    }

  gst_object_unref (conv);

  return res;
}

