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

#ifndef __GST_MOODBAR_H__
#define __GST_MOODBAR_H__

#include <gst/gst.h>

G_BEGIN_DECLS

/* #defines don't like whitespacey bits */
#define GST_TYPE_MOODBAR \
  (gst_moodbar_get_type())
#define GST_MOODBAR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MOODBAR,GstMoodbar))
#define GST_MOODBAR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_MOODBAR,GstMoodbarClass))

typedef struct _GstMoodbar      GstMoodbar;
typedef struct _GstMoodbarClass GstMoodbarClass;

struct _GstMoodbar
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  /* Stream data */
  gint rate, size;
  
  /* Cached band -> bark band table */
  guint *barkband_table;

  /* Queued moodbar data */
  gdouble *r, *g, *b;
  guint numframes;

  /* Property */
  guint height;
  guint max_width;
};

struct _GstMoodbarClass 
{
  GstElementClass parent_class;
};

GType gst_moodbar_get_type (void);

G_END_DECLS

#endif /* __GST_MOODBAR_H__ */
