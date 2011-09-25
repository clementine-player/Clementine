/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#ifndef __GST_AFCSRC_H__
#define __GST_AFCSRC_H__

#include <gst/gst.h>
#include <gst/base/gstbasesrc.h>

#include <libimobiledevice/afc.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

#ifdef __cplusplus
extern "C" {
#endif
  void afcsrc_register_static();
#ifdef __cplusplus
}
#endif

G_BEGIN_DECLS

#define GST_TYPE_AFCSRC \
  (gst_afc_src_get_type())
#define GST_AFCSRC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_AFCSRC,GstAfcSrc))
#define GST_AFCSRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_AFCSRC,GstAfcSrcClass))
#define GST_IS_AFCSRC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_AFCSRC))
#define GST_IS_AFCSRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_AFCSRC))

typedef struct _GstAfcSrc      GstAfcSrc;
typedef struct _GstAfcSrcClass GstAfcSrcClass;

struct _GstAfcSrc {
  GstBaseSrc element;

  char* location_;
  char* uuid_;
  char* path_;

  gboolean connected_;
  idevice_t device_;
  afc_client_t afc_;

  uint16_t afc_port_;

  uint64_t file_handle_;

  gboolean buffer_is_valid_;
  guint64 buffer_offset_;
  guint buffer_length_;
  char* buffer_;
};

struct _GstAfcSrcClass {
  GstBaseSrcClass parent_class;
};

GType gst_afc_src_get_type (void);

G_END_DECLS

#endif
