/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "engines/gstdecodeaudioonly.h"

#include <gst/gst.h>

#include <QRegularExpression>
#include <QStringList>
#include <QTemporaryDir>

#include "gtest/gtest.h"

namespace {

// What a uridecodebin made and exposed while it prerolled a file.
struct Decoded {
  QStringList decoders;  // The klass of each decoder it made.
  QStringList pads;      // The media type of each pad it exposed.
};

void ElementAdded(GstBin*, GstBin*, GstElement* element, gpointer data) {
  GstElementFactory* factory = gst_element_get_factory(element);
  if (!factory) return;
  const char* klass =
      gst_element_factory_get_metadata(factory, GST_ELEMENT_METADATA_KLASS);
  if (klass && strstr(klass, "Decoder")) {
    static_cast<Decoded*>(data)->decoders << klass;
  }
}

// Gives every pad a fakesink, so that the pipeline prerolls whatever's
// exposed.
void PadAdded(GstElement* decodebin, GstPad* pad, gpointer data) {
  GstCaps* caps = gst_pad_get_current_caps(pad);
  static_cast<Decoded*>(data)->pads
      << gst_structure_get_name(gst_caps_get_structure(caps, 0));
  gst_caps_unref(caps);

  GstElement* pipeline = GST_ELEMENT(gst_element_get_parent(decodebin));
  GstElement* sink = gst_element_factory_make("fakesink", nullptr);
  g_object_set(sink, "sync", FALSE, nullptr);
  gst_bin_add(GST_BIN(pipeline), sink);
  gst_element_sync_state_with_parent(sink);
  GstPad* sink_pad = gst_element_get_static_pad(sink, "sink");
  gst_pad_link(pad, sink_pad);
  gst_object_unref(sink_pad);
  gst_object_unref(pipeline);
}

// Waits for a pipeline to finish what it's doing, and says whether it worked.
bool Wait(GstElement* pipeline, GstMessageType done) {
  GstBus* bus = gst_element_get_bus(pipeline);
  GstMessage* message = gst_bus_timed_pop_filtered(
      bus, 10 * GST_SECOND,
      static_cast<GstMessageType>(done | GST_MESSAGE_ERROR));
  gst_object_unref(bus);
  const bool ok = message && GST_MESSAGE_TYPE(message) == done;
  if (message) gst_message_unref(message);
  return ok;
}

class GstDecodeAudioOnlyTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() { gst_init(nullptr, nullptr); }

  void SetUp() override {
    for (const char* name : {"matroskamux", "matroskademux", "jpegenc",
                             "jpegdec", "audiotestsrc", "videotestsrc"}) {
      GstElementFactory* factory = gst_element_factory_find(name);
      if (!factory) GTEST_SKIP() << "No " << name << " element";
      gst_object_unref(factory);
    }

    // A short file with a stream of each: raw audio, and JPEG video.
    ASSERT_TRUE(dir_.isValid());
    const QString filename = dir_.filePath("video.mkv");
    GError* error = nullptr;
    GstElement* pipeline = gst_parse_launch(
        QString("matroskamux name=mux ! filesink location=\"%1\" "
                "audiotestsrc num-buffers=10 ! "
                "audio/x-raw,format=S16LE,rate=44100,channels=2 ! mux. "
                "videotestsrc num-buffers=5 ! "
                "video/x-raw,width=64,height=64,framerate=10/1 ! jpegenc ! "
                "mux.")
            .arg(filename)
            .toUtf8()
            .constData(),
        &error);
    ASSERT_EQ(nullptr, error) << error->message;
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    const bool written = Wait(pipeline, GST_MESSAGE_EOS);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    ASSERT_TRUE(written);

    gchar* uri = gst_filename_to_uri(filename.toUtf8().constData(), nullptr);
    uri_ = uri;
    g_free(uri);
  }

  Decoded Preroll(bool audio_only) {
    Decoded decoded;
    GstElement* pipeline = gst_pipeline_new(nullptr);
    GstElement* decodebin = gst_element_factory_make("uridecodebin", nullptr);
    gst_bin_add(GST_BIN(pipeline), decodebin);
    if (audio_only) GstDecodeAudioOnly(decodebin);
    g_object_set(decodebin, "uri", uri_.constData(), nullptr);
    g_signal_connect(decodebin, "deep-element-added", G_CALLBACK(ElementAdded),
                     &decoded);
    g_signal_connect(decodebin, "pad-added", G_CALLBACK(PadAdded), &decoded);

    gst_element_set_state(pipeline, GST_STATE_PAUSED);
    EXPECT_TRUE(Wait(pipeline, GST_MESSAGE_ASYNC_DONE));
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    return decoded;
  }

  QTemporaryDir dir_;
  QByteArray uri_;
};

// The JPEG stream's decoder depends on the platform: jpegdec, or a hardware
// one such as macOS's vtdec, which says it's a video decoder.
QStringList PictureDecoders(const Decoded& decoded) {
  return decoded.decoders.filter(QRegularExpression("Decoder/(Image|Video)"));
}

TEST_F(GstDecodeAudioOnlyTest, DecodesVideoByDefault) {
  const Decoded decoded = Preroll(false);
  EXPECT_NE(QStringList(), PictureDecoders(decoded))
      << decoded.decoders.join(", ").toStdString();
  EXPECT_TRUE(decoded.pads.contains("video/x-raw"));
}

TEST_F(GstDecodeAudioOnlyTest, DecodesOnlyAudio) {
  const Decoded decoded = Preroll(true);
  EXPECT_EQ(QStringList(), PictureDecoders(decoded));
  EXPECT_EQ(QStringList() << "audio/x-raw", decoded.pads);
}

}  // namespace
