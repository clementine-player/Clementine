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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gstfftwspectrum.h"
#include "gstmoodbar.h"
#include "spectrum.h"


/***************************************************************/
/* Plugin managing                                             */
/***************************************************************/

GST_DEBUG_CATEGORY_EXTERN (gst_fftwspectrum_debug);
GST_DEBUG_CATEGORY_EXTERN (gst_moodbar_debug);


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and pad templates
 * register the features
 *
 * exchange the string 'plugin' with your elemnt name
 */
static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "fftwspectrum",
			     GST_RANK_NONE, GST_TYPE_FFTWSPECTRUM))
    return FALSE;

  if (!gst_element_register (plugin, "moodbar",
			     GST_RANK_NONE, GST_TYPE_MOODBAR))
    return FALSE;

  GST_DEBUG_CATEGORY_INIT (gst_fftwspectrum_debug, "fftwspectrum",
      0, "FFTW Sample-to-Spectrum Converter Plugin");
  GST_DEBUG_CATEGORY_INIT (gst_moodbar_debug, "moodbar",
      0, "Moodbar analyzer");

  return TRUE;
}

void gstmoodbar_register_static() {
  gst_plugin_register_static(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "moodbar",
    "Frequency analyzer and converter plugin",
    plugin_init,
    "0.1.2",
    "GPL",
    "Moodbar",
    "Moodbar",
    "http://amarok.kde.org/wiki/Moodbar");
}
