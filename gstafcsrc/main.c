/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gstafcsrc.h"

#include <gst/gst.h>

GST_DEBUG_CATEGORY_STATIC(gst_afc_src_debug);
#define GST_CAT_DEFAULT gst_afc_src_debug

static gboolean afcsrc_init(GstPlugin* afcsrc) {
  GST_DEBUG_CATEGORY_INIT (gst_afc_src_debug, "afcsrc", 0, "iPod/iPhone Source");

  return gst_element_register(afcsrc, "afcsrc", GST_RANK_NONE, GST_TYPE_AFCSRC);
}

#define PACKAGE "Clementine"

GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "afcsrc",
    "iPod/iPhone Source",
    afcsrc_init,
    "0.1",
    "GPL",
    "Clementine",
    "http://www.clementine-player.org/"
)
