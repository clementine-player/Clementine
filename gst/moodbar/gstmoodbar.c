/* GStreamer spectrum analysis toy
 * Copyright (C) 2006 Joseph Rabinoff <bobqwatson@yahoo.com>
 * Some code copyright (C) 2005 Gav Wood
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
 * SECTION:element-moodbar
 *
 * <refsect2>
 * <title>Example launch line</title>
 * <para>
 * <programlisting>
 * gst-launch filesrc location=test.mp3 ! mad ! audioconvert ! fftwspectrum ! moodbar height=50 ! pngenc ! filesink location=test.png
 * </programlisting>
 * </para>
 * </refsect2>
 */

/* This plugin is based on the Moodbar code in Amarok version 1.4.0a,
 * written by Gav Wood.  The algorithm is basically the same as the
 * one applied there, and the normalizing code below is taken directly
 * from Gav Wood's Exscalibar package.
 */

/* This plugin takes a frequency-domain stream, does some simple 
 * analysis, and returns a string of (unsigned char) rgb triples
 * that represent the magnitude of various sections of the stream.
 * Since we have to perform some normalization, we queue up all
 * of our analysis until we get an EOS event, at which point we 
 * normalize and do the output.  If a max-width is specified, the
 * output is scaled down to the desired width if necessary.
 */

/* More precisely, the analysis performed is as follows:
 *  (1) the spectrum is broken into 24 parts, called "bark bands"
 *      (Gav's terminology), as given in bark_bands below
 *  (2) we compute the size of the first 8 bark bands and store
 *      that as the "red" component; similarly for blue and green
 *  (3) after receiving an EOS, we normalize all of the analysis
 *      done in (1) and (2) and return a stream of rgb triples
 *      (application/x-raw-rgb)
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <string.h>
#include <math.h>

#include "gstmoodbar.h"
#include "spectrum.h"

GST_DEBUG_CATEGORY (gst_moodbar_debug);
#define GST_CAT_DEFAULT gst_moodbar_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  ARG_0,
  ARG_HEIGHT,
  ARG_MAX_WIDTH
};

static GstStaticPadTemplate sink_factory 
  = GST_STATIC_PAD_TEMPLATE ("sink",
			     GST_PAD_SINK,
			     GST_PAD_ALWAYS,
			     GST_STATIC_CAPS 
			       ( SPECTRUM_FREQ_CAPS )
			     );

static GstStaticPadTemplate src_factory 
  = GST_STATIC_PAD_TEMPLATE ("src",
			     GST_PAD_SRC,
			     GST_PAD_ALWAYS,
			     GST_STATIC_CAPS
			       ( "video/x-raw-rgb, " 
				   "bpp = (int) 24, "
				   "depth = (int) 24, "
				   "height = (int) [ 1, MAX ], "
				   "width = (int) [ 1, MAX ], "
				   "framerate = (fraction) 0/1"
			       )
			     );

GST_BOILERPLATE (GstMoodbar, gst_moodbar, GstElement,
    GST_TYPE_ELEMENT);

static void gst_moodbar_set_property (GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec);
static void gst_moodbar_get_property (GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec);

static gboolean gst_moodbar_set_sink_caps (GstPad *pad, GstCaps *caps);
static gboolean gst_moodbar_sink_event (GstPad *pad, GstEvent *event);

static GstFlowReturn gst_moodbar_chain (GstPad *pad, GstBuffer *buf);
static GstStateChangeReturn gst_moodbar_change_state (GstElement *element,
    GstStateChange transition);

static void gst_moodbar_finish (GstMoodbar *mood);

/* This is a failsafe so we don't eat up all of a computer's memory
 * if we hit an endless stream. */
#define MAX_TRIPLES (1024*1024*4)  

#define NUMFREQS(mood) ((mood)->size/2+1)

/* Allocate mood->r, mood->g, and mood->b in chunks of this many */
#define FRAME_CHUNK 1000

/* Default height of the output image */
#define HEIGHT_DEFAULT 1

/* Default max-width of the output image, or 0 for no rescaling */
#define MAX_WIDTH_DEFAULT 0

/* We use this table to break up the incoming spectrum into segments */
static const guint bark_bands[24] 
  = { 100,  200,  300,  400,  510,  630,  770,   920, 
      1080, 1270, 1480, 1720, 2000, 2320, 2700,  3150, 
      3700, 4400, 5300, 6400, 7700, 9500, 12000, 15500 };


/***************************************************************/
/* GObject boilerplate stuff                                   */
/***************************************************************/


static void
gst_moodbar_base_init (gpointer gclass)
{
  static GstElementDetails element_details = 
    {
      "Moodbar analyzer",
      "Filter/Converter/Moodbar",
      "Convert a spectrum into a stream of (uchar) rgb triples representing its \"mood\"",
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
gst_moodbar_class_init (GstMoodbarClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_moodbar_set_property;
  gobject_class->get_property = gst_moodbar_get_property;

  g_object_class_install_property (gobject_class, ARG_HEIGHT,
      g_param_spec_int ("height", "Image height", 
	  "The height of the resulting raw image",
	  1, G_MAXINT32, HEIGHT_DEFAULT, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, ARG_MAX_WIDTH,
      g_param_spec_int ("max-width", "Image maximum width", 
	  "The maximum width of the resulting raw image, or 0 for no rescaling",
	  0, G_MAXINT32, MAX_WIDTH_DEFAULT, G_PARAM_READWRITE));

  gstelement_class->change_state 
    = GST_DEBUG_FUNCPTR (gst_moodbar_change_state);
}

/* initialize the new element
 * instantiate pads and add them to element
 * set functions
 * initialize structure
 */
static void
gst_moodbar_init (GstMoodbar *mood, GstMoodbarClass *gclass)
{
  GstElementClass *klass = GST_ELEMENT_GET_CLASS (mood);

  mood->sinkpad =
      gst_pad_new_from_template 
          (gst_element_class_get_pad_template (klass, "sink"), "sink");
  gst_pad_set_setcaps_function (mood->sinkpad, 
				GST_DEBUG_FUNCPTR (gst_moodbar_set_sink_caps));
  gst_pad_set_event_function (mood->sinkpad,
			      GST_DEBUG_FUNCPTR (gst_moodbar_sink_event));
  gst_pad_set_chain_function (mood->sinkpad, 
			      GST_DEBUG_FUNCPTR (gst_moodbar_chain));

  mood->srcpad =
      gst_pad_new_from_template 
          (gst_element_class_get_pad_template (klass, "src"), "src");


  gst_element_add_pad (GST_ELEMENT (mood), mood->sinkpad);
  gst_element_add_pad (GST_ELEMENT (mood), mood->srcpad);

  /* These are set once the (sink) capabilities are determined */
  mood->rate = 0;
  mood->size = 0;
  mood->barkband_table = NULL;
  
  /* These are allocated when we change to PAUSED */
  mood->r = NULL;
  mood->g = NULL;
  mood->b = NULL;
  mood->numframes = 0;

  /* Property */
  mood->height = HEIGHT_DEFAULT;
  mood->max_width = MAX_WIDTH_DEFAULT;
}


static void gst_moodbar_set_property (GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec)
{
  GstMoodbar *mood = GST_MOODBAR (object);

  switch (prop_id) 
    {
    case ARG_HEIGHT:
      mood->height = (guint) g_value_get_int (value);
      break;
    case ARG_MAX_WIDTH:
      mood->max_width = (guint) g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
  
}


static void gst_moodbar_get_property (GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec)
{
  GstMoodbar *mood = GST_MOODBAR (object);

  switch (prop_id) 
    {
    case ARG_HEIGHT:
      g_value_set_int (value, (int) mood->height);
      break;
    case ARG_MAX_WIDTH:
      g_value_set_int (value, (int) mood->max_width);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}




/***************************************************************/
/* Pad handling                                                */
/***************************************************************/


/* This calculates a table that caches which bark band slot each
 * incoming band is supposed to go in. */
static void
calc_barkband_table (GstMoodbar *mood)
{
  guint i;
  guint barkband = 0;

  /* Avoid divide-by-zero */
  if (!mood->size  ||  !mood->rate)
    return;

  if (mood->barkband_table)
    g_free (mood->barkband_table);

  mood->barkband_table = g_malloc (NUMFREQS (mood) * sizeof (guint));
  
  for (i = 0; i < NUMFREQS (mood); ++i)
    {
      if (barkband < 23 &&
	  (guint) GST_SPECTRUM_BAND_FREQ (i, mood->size, mood->rate) 
	    >= bark_bands[barkband])
	barkband++;

      mood->barkband_table[i] = barkband;

      /*
      GST_LOG ("Band %d (frequency %f) -> barkband %d (frequency %d)",
	       i, GST_SPECTRUM_BAND_FREQ (i, mood->size, mood->rate),
	       barkband, bark_bands[barkband]);
      */
    }
}


/* Setting the sink caps just gets the rate and size parameters.
 * Note that we do not support upstream caps renegotiation, since
 * we could only possibly scale the height anyway.
 */

static gboolean
gst_moodbar_set_sink_caps (GstPad *pad, GstCaps *caps)
{
  GstMoodbar *mood;
  GstStructure *newstruct;
  gint rate, size;
  gboolean res = FALSE;

  mood = GST_MOODBAR (gst_pad_get_parent (pad));

  newstruct = gst_caps_get_structure (caps, 0);
  if (!gst_structure_get_int (newstruct, "rate", &rate) ||
      !gst_structure_get_int (newstruct, "size", &size))
    goto out;

  res = TRUE;
  
  mood->rate = rate;
  mood->size = (guint) size;
  calc_barkband_table (mood);
 
 out:
  gst_object_unref (mood);

  return res;
}


static gboolean
gst_moodbar_sink_event (GstPad *pad, GstEvent *event)
{
  GstMoodbar *mood;
  gboolean res = TRUE;

  mood = GST_MOODBAR (gst_pad_get_parent (pad));

  if (GST_EVENT_TYPE (event) == GST_EVENT_EOS)
    gst_moodbar_finish (mood);
  
  res = gst_pad_push_event (mood->srcpad, event);
  gst_object_unref (mood);

  return res;
}


/***************************************************************/
/* Actual analysis                                             */
/***************************************************************/


static GstStateChangeReturn 
gst_moodbar_change_state (GstElement *element, GstStateChange transition)
{
  GstMoodbar *mood = GST_MOODBAR (element);
  GstStateChangeReturn res;

  switch (transition)
    {
    case GST_STATE_CHANGE_NULL_TO_READY:
      calc_barkband_table (mood);
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      mood->r = (gdouble *) g_malloc (FRAME_CHUNK * sizeof(gdouble));
      mood->g = (gdouble *) g_malloc (FRAME_CHUNK * sizeof(gdouble));
      mood->b = (gdouble *) g_malloc (FRAME_CHUNK * sizeof(gdouble));
      mood->numframes = 0;
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
      g_free (mood->r);
      g_free (mood->g);
      g_free (mood->b);
      mood->r = NULL;
      mood->g = NULL;
      mood->b = NULL;
      mood->numframes = 0;
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
      g_free (mood->barkband_table);
      mood->barkband_table = NULL;
      break;
    default:
      break;
    }

  return res;
}


/* We allocate r, g, b frames in chunks of FRAME_CHUNK so we don't
 * have to realloc every time a buffer comes in.
 */
static gboolean
allocate_another_frame (GstMoodbar *mood)
{
  mood->numframes++;

  /* Failsafe */
  if (mood->numframes == MAX_TRIPLES)
    return FALSE;

  if(mood->numframes % FRAME_CHUNK == 0)
    {
      guint size = (mood->numframes + FRAME_CHUNK) * sizeof (gdouble);

      mood->r = (gdouble *) g_realloc (mood->r, size);
      mood->g = (gdouble *) g_realloc (mood->g, size);
      mood->b = (gdouble *) g_realloc (mood->b, size);

      if (mood->r == NULL || mood->g == NULL || mood->b == NULL)
	return FALSE;
    }

  return TRUE;
}


/* This function does most of the analysis on the spectra we
 * get as input and caches them.  We actually push buffers
 * once we receive an EOS signal.
 */
static GstFlowReturn
gst_moodbar_chain (GstPad *pad, GstBuffer *buf)
{
  GstMoodbar *mood = GST_MOODBAR (gst_pad_get_parent (pad));
  guint i;
  gdouble amplitudes[24], rgb[3] = {0.f, 0.f, 0.f};
  gdouble *out, real, imag;
  guint numfreqs = NUMFREQS (mood);

  if (GST_BUFFER_SIZE (buf) != numfreqs * sizeof (gdouble) * 2)
    {
      gst_object_unref (mood);
      return GST_FLOW_ERROR;
    }

  out = (gdouble *) GST_BUFFER_DATA (buf);

  if (!allocate_another_frame (mood))
    return GST_FLOW_ERROR;

  /* Calculate total amplitudes for the different bark bands */
  
  for (i = 0; i < 24; ++i)
    amplitudes[i] = 0.f;

  for (i = 0; i < numfreqs; ++i)
    {
      real = out[2*i];  imag = out[2*i + 1];
      amplitudes[mood->barkband_table[i]] += sqrtf (real*real + imag*imag);
    }

  /* Now divide the bark bands into thirds and compute their total 
   * amplitudes */

  for (i = 0; i < 24; ++i)
    rgb[i/8] += amplitudes[i] * amplitudes[i];

  rgb[0] = sqrtf (rgb[0]);
  rgb[1] = sqrtf (rgb[1]);
  rgb[2] = sqrtf (rgb[2]);

  mood->r[mood->numframes] = rgb[0];
  mood->g[mood->numframes] = rgb[1];
  mood->b[mood->numframes] = rgb[2];

  gst_buffer_unref (buf);
  gst_object_unref (mood);

  return GST_FLOW_OK;
}


/* The normalization code was copied from Gav Wood's Exscalibar
 * library, normalise.cpp
 */
static void
normalize (gdouble *vals, guint numvals)
{
  gdouble mini, maxi, tu = 0.f, tb = 0.f;
  gdouble avgu = 0.f, avgb = 0.f, delta, avg = 0.f;
  gdouble avguu = 0.f, avgbb = 0.f;
  guint i;
  gint t = 0;

  if (!numvals) 
    return;

  mini = maxi = vals[0];

  for (i = 1; i < numvals; i++)
    {
      if (vals[i] > maxi) 
	maxi = vals[i];
      else if (vals[i] < mini) 
	mini = vals[i];
    }

  for (i = 0; i < numvals; i++)
    {
      if(vals[i] != mini && vals[i] != maxi)
	{
    avg += vals[i] / ((gdouble) numvals);
	  t++; 
	}
    }

  for (i = 0; i < numvals; i++)
    {
      if (vals[i] != mini && vals[i] != maxi)
	{
	  if (vals[i] > avg) 
	    { 
	      avgu += vals[i]; 
	      tu++; 
	    }
	  else 
	    { 
	      avgb += vals[i]; 
	      tb++; 
	    }
	}
    }

  avgu /= (gdouble) tu;
  avgb /= (gdouble) tb;

  tu = 0.f; 
  tb = 0.f;
  for (i = 0; i < numvals; i++)
    {
      if (vals[i] != mini && vals[i] != maxi)
	{
	  if (vals[i] > avgu) 
	    { 
	      avguu += vals[i]; 
	      tu++; 
	    }

	  else if (vals[i] < avgb) 
	    { 
	      avgbb += vals[i]; 
	      tb++; 
	    }
	}
    }

  avguu /= (gdouble) tu;
  avgbb /= (gdouble) tb;

  mini = MAX (avg + (avgb - avg) * 2.f, avgbb);
  maxi = MIN (avg + (avgu - avg) * 2.f, avguu);
  delta = maxi - mini;

  if (delta == 0.f)
    delta = 1.f;

  for (i = 0; i < numvals; i++)
    vals[i] = isfinite (vals[i]) ? MIN(1.f, MAX(0.f, (vals[i] - mini) / delta))
                               : 0.f;
}



/* This function normalizes all of the cached r,g,b data and 
 * finally pushes a monster buffer with all of our output.
 */
static void 
gst_moodbar_finish (GstMoodbar *mood)
{
  GstBuffer *buf;
  guchar *data;
  guint line;
  guint output_width;

  if (mood->max_width == 0
        || mood->numframes <= mood->max_width)
    output_width = mood->numframes;
  else
    output_width = mood->max_width;

  normalize (mood->r, mood->numframes);
  normalize (mood->g, mood->numframes);
  normalize (mood->b, mood->numframes);

  buf = gst_buffer_new_and_alloc 
            (output_width * mood->height * 3 * sizeof (guchar));
  if (!buf)
    return;
  /* Don't set the timestamp, duration, etc. since it's irrelevant */
  GST_BUFFER_OFFSET (buf) = 0;
  data = (guchar *) GST_BUFFER_DATA (buf);

  gdouble r, g, b;
  guint i, j, n;
  guint start, end;
  for (line = 0; line < mood->height; ++line)
    {
      for (i = 0; i < output_width; ++i)
	{
	  r = 0.f;  g = 0.f;  b = 0.f;
	  start = i * mood->numframes / output_width;
	  end = (i + 1) * mood->numframes / output_width;
	  if ( start == end )
	    end = start + 1;

	  for( j = start; j < end; j++ )
	    {
	      r += mood->r[j] * 255.f;
	      g += mood->g[j] * 255.f;
	      b += mood->b[j] * 255.f;
	    }

	  n = end - start;

    *(data++) = (guchar) (r / ((gdouble) n));
    *(data++) = (guchar) (g / ((gdouble) n));
    *(data++) = (guchar) (b / ((gdouble) n));
	}
    }

  { /* Now we (finally) know the width of the image we're pushing */
    GstCaps *caps = gst_caps_copy (gst_pad_get_caps (mood->srcpad));
    gboolean res;
    gst_caps_set_simple (caps, "width", G_TYPE_INT, output_width, NULL);
    gst_caps_set_simple (caps, "height", G_TYPE_INT, mood->height, NULL);
    res = gst_pad_set_caps (mood->srcpad, caps);
    if (res)
      gst_buffer_set_caps (buf, caps);
    gst_caps_unref (caps);
    if (!res)
      return;
  }

  gst_pad_push (mood->srcpad, buf);
}
