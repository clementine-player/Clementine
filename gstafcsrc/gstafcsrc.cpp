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

#include <stdlib.h>
#include <string.h>
#include <gst/gst.h>


// Signals
enum {
  LAST_SIGNAL
};

// Properties
enum {
  PROP_0,
  PROP_LOCATION,
};

GST_DEBUG_CATEGORY_STATIC(gst_afc_src_debug);
#define GST_CAT_DEFAULT gst_afc_src_debug

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
);

static void gst_afc_src_interface_init(GType type);
static void gst_afc_src_uri_handler_init(gpointer iface, gpointer data);
static void gst_afc_src_set_property(GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_afc_src_get_property(GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);
static void gst_afc_src_finalize(GObject* object);
static gboolean gst_afc_src_start(GstBaseSrc* src);
static gboolean gst_afc_src_stop(GstBaseSrc* src);
static GstFlowReturn gst_afc_src_create(GstBaseSrc* src, guint64 offset, guint length, GstBuffer** buffer);
static gboolean gst_afc_src_is_seekable(GstBaseSrc* src);
static gboolean gst_afc_src_get_size(GstBaseSrc* src, guint64* size);

GST_BOILERPLATE_FULL(GstAfcSrc, gst_afc_src, GstBaseSrc, GST_TYPE_BASE_SRC, gst_afc_src_interface_init);


static void gst_afc_src_interface_init(GType type) {
  static const GInterfaceInfo urihandler_info = {
    gst_afc_src_uri_handler_init,
    NULL,
    NULL
  };

  GST_DEBUG_CATEGORY_INIT (gst_afc_src_debug, "afcsrc", 0, "iPod/iPhone Source");

  g_type_add_interface_static(type, GST_TYPE_URI_HANDLER, &urihandler_info);
}

static void gst_afc_src_base_init(gpointer gclass) {
  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_set_details_simple(element_class,
    "iPod/iPhone Source",
    "Source/Afc",
    "Read using libimobiledevice",
    "David Sansome <me@davidsansome.com>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_factory));
}

static void gst_afc_src_class_init (GstAfcSrcClass* klass) {
  GObjectClass* gobject_class = (GObjectClass*) klass;
  GstBaseSrcClass* gstbasesrc_class = (GstBaseSrcClass*) klass;

  gobject_class->set_property = gst_afc_src_set_property;
  gobject_class->get_property = gst_afc_src_get_property;
  gobject_class->finalize = gst_afc_src_finalize;

  g_object_class_install_property(gobject_class, PROP_LOCATION,
    g_param_spec_string(
        "location", "URI",
        "The URI of the file to read, must be of the form afc://uuid/filename", NULL,
        GParamFlags(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | GST_PARAM_MUTABLE_READY)));

  gstbasesrc_class->start = gst_afc_src_start;
  gstbasesrc_class->stop = gst_afc_src_stop;
  gstbasesrc_class->create = gst_afc_src_create;
  gstbasesrc_class->is_seekable = gst_afc_src_is_seekable;
  gstbasesrc_class->get_size = gst_afc_src_get_size;
}

static GstURIType gst_afc_src_uri_get_type() {
  return GST_URI_SRC;
}

static gchar** gst_afc_src_uri_get_protocols() {
  static const gchar* protocols[] = { "afc", NULL };
  return (char**) protocols;
}

static const gchar* gst_afc_src_uri_get_uri(GstURIHandler* handler) {
  GstAfcSrc* self = GST_AFCSRC(handler);
  return self->location_;
}

static gboolean gst_afc_src_uri_set_uri(GstURIHandler* handler, const gchar* uri) {
  GstAfcSrc* self = GST_AFCSRC(handler);
  self->location_ = g_strdup(uri);
  return true;
}

static void gst_afc_src_uri_handler_init(gpointer g_iface, gpointer) {
  GstURIHandlerInterface* iface = (GstURIHandlerInterface*) g_iface;

  iface->get_type = gst_afc_src_uri_get_type;
  iface->get_protocols = gst_afc_src_uri_get_protocols;
  iface->set_uri = gst_afc_src_uri_set_uri;
  iface->get_uri = gst_afc_src_uri_get_uri;
}


static void gst_afc_src_init(GstAfcSrc* element, GstAfcSrcClass* gclass) {
  element->location_ = NULL;
  element->uuid_ = NULL;
  element->path_ = NULL;
  element->afc_ = NULL;
  element->afc_port_ = 0;
  element->device_ = NULL;
  element->file_handle_ = 0;
  element->lockdown_ = NULL;
}

static void gst_afc_src_finalize(GObject* object) {
  GstAfcSrc* src = GST_AFCSRC(object);
  free(src->location_);
  free(src->uuid_);
  free(src->path_);

  G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void gst_afc_src_set_property(
    GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
  GstAfcSrc* self = GST_AFCSRC(object);

  switch (prop_id) {
    case PROP_LOCATION:
      self->location_ = g_strdup(g_value_get_string(value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void gst_afc_src_get_property(
    GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
  GstAfcSrc* self = GST_AFCSRC(object);

  switch (prop_id) {
    case PROP_LOCATION:
      g_value_set_string(value, self->location_);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean gst_afc_src_start(GstBaseSrc* src) {
  GstAfcSrc* self = GST_AFCSRC(src);

  // Check that a URI has been passed
  if (!self->location_ || self->location_[0] == '\0') {
    GST_ELEMENT_ERROR(src, RESOURCE, NOT_FOUND, ("No URI specified"), (NULL));
    return false;
  }

  // Parse the URI
  // HERE BE DRAGONS
  gchar* location = gst_uri_get_location(self->location_);
  char* path_pos = strstr(location, "/");

  self->uuid_ = (char*) malloc(path_pos - location + 1);
  memcpy(self->uuid_, location, path_pos - location);
  self->uuid_[path_pos - location] = '\0';
  self->path_ = g_strdup(path_pos);

  g_free(location);

  // Open the device
  idevice_error_t err = idevice_new(&self->device_, self->uuid_);
  if (err != IDEVICE_E_SUCCESS) {
    GST_ELEMENT_ERROR(src, RESOURCE, NOT_FOUND, ("idevice error: %d", err), (NULL));
    return false;
  }

  lockdownd_error_t lockdown_err =
      lockdownd_client_new_with_handshake(self->device_, &self->lockdown_, "GstAfcSrc");
  if (lockdown_err != LOCKDOWN_E_SUCCESS) {
    GST_ELEMENT_ERROR(src, RESOURCE, NOT_FOUND, ("lockdown error: %d", lockdown_err), (NULL));
    return false;
  }

  lockdown_err = lockdownd_start_service(self->lockdown_, "com.apple.afc", &self->afc_port_);
  if (lockdown_err != LOCKDOWN_E_SUCCESS) {
    GST_ELEMENT_ERROR(src, RESOURCE, NOT_FOUND, ("lockdown error: %d", lockdown_err), (NULL));
    return false;
  }

  afc_error_t afc_err = afc_client_new(self->device_, self->afc_port_, &self->afc_);
  if (afc_err != 0) {
    GST_ELEMENT_ERROR(src, RESOURCE, NOT_FOUND, ("afc error: %d", afc_err), (NULL));
    return false;
  }

  // Try opening the file
  afc_err = afc_file_open(self->afc_, self->path_, AFC_FOPEN_RDONLY, &self->file_handle_);
  if (afc_err != 0) {
    GST_ELEMENT_ERROR(src, RESOURCE, NOT_FOUND, ("afc error: %d", afc_err), (NULL));
    return false;
  }

  return true;
}

static gboolean gst_afc_src_stop(GstBaseSrc* src) {
  GstAfcSrc* self = GST_AFCSRC(src);

  if (self->file_handle_) {
    afc_file_close(self->afc_, self->file_handle_);
  }

  if (self->afc_) {
    afc_client_free(self->afc_);
  }
  if (self->lockdown_) {
    lockdownd_client_free(self->lockdown_);
  }
  if (self->device_) {
    idevice_free(self->device_);
  }

  return true;
}

static GstFlowReturn gst_afc_src_create(GstBaseSrc* src, guint64 offset, guint length, GstBuffer** buffer) {
  GstAfcSrc* self = GST_AFCSRC(src);

  GstBuffer* buf = gst_buffer_try_new_and_alloc(length);
  if (buf == NULL && length > 0) {
    GST_ERROR_OBJECT(src, "Failed to allocate %u bytes", length);
    return GST_FLOW_ERROR;
  }

  uint32_t bytes_read = 0;
  if (length > 0) {
    afc_file_seek(self->afc_, self->file_handle_, offset, SEEK_SET);
    afc_file_read(self->afc_, self->file_handle_, (char*)(GST_BUFFER_DATA(buf)), length, &bytes_read);
  }

  *buffer = buf;
  return GST_FLOW_OK;
}

static gboolean gst_afc_src_is_seekable(GstBaseSrc* src) {
  return true;
}

static gboolean gst_afc_src_get_size(GstBaseSrc* src, guint64* size) {
  GstAfcSrc* self = GST_AFCSRC(src);

  char** infolist = NULL;
  afc_error_t err = afc_get_file_info(self->afc_, self->path_, &infolist);
  if (err != AFC_E_SUCCESS || !infolist) {
    GST_ELEMENT_ERROR(src, RESOURCE, NOT_FOUND, ("afc error: %d", err), (NULL));
    return false;
  }

  bool found_size = false;

  char** p = infolist;
  while (*p != NULL) {
    if (g_strcmp0(*p, "st_size") == 0) {
      *size = strtoll(*(p+1), NULL, 0);
      found_size = true;
    }
    free(*p); ++p;
    free(*p); ++p;
  }
  free(infolist);

  if (!found_size) {
    *size = 0;
    return false;
  }

  return true;
}
