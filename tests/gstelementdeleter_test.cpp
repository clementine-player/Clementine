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

#include "engines/gstelementdeleter.h"

#include <gst/gst.h>

#include <QCoreApplication>

#include "gtest/gtest.h"

namespace {

void SetFinalized(gpointer data, GObject*) { *static_cast<bool*>(data) = true; }

class GstElementDeleterTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() { gst_init(nullptr, nullptr); }
};

// GstEnginePipeline swaps decode bins this way for gapless playback: it takes
// the old bin out of the pipeline from a streaming thread, and has the deleter
// stop it later in the main thread.
TEST_F(GstElementDeleterTest, StopsAnElementTakenOutOfABin) {
  GstElement* pipeline = gst_pipeline_new(nullptr);
  GstElement* bin = gst_bin_new(nullptr);
  bool finalized = false;
  g_object_weak_ref(G_OBJECT(bin), &SetFinalized, &finalized);

  gst_bin_add(GST_BIN(pipeline), bin);
  gst_element_set_state(pipeline, GST_STATE_PAUSED);
  // Drop the state change messages, which hold references to the bin.
  GstBus* bus = gst_element_get_bus(pipeline);
  gst_bus_set_flushing(bus, TRUE);
  gst_object_unref(bus);

  gst_object_ref(bin);
  gst_bin_remove(GST_BIN(pipeline), bin);

  GstElementDeleter deleter;
  deleter.DeleteElementLater(bin);

  // Nothing happens until the main thread gets to it.
  EXPECT_FALSE(finalized);
  EXPECT_EQ(GST_STATE_PAUSED, GST_STATE(bin));

  QCoreApplication::sendPostedEvents(&deleter);

  // It was stopped, and the deleter's reference was the last one.
  EXPECT_TRUE(finalized);

  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(pipeline);
}

TEST_F(GstElementDeleterTest, LeavesOtherReferencesAlone) {
  GstElement* bin = gst_bin_new(nullptr);
  bool finalized = false;
  g_object_weak_ref(G_OBJECT(bin), &SetFinalized, &finalized);
  gst_object_ref_sink(bin);

  gst_element_set_state(bin, GST_STATE_PAUSED);

  // The caller's own reference, handed to the deleter.
  gst_object_ref(bin);
  GstElementDeleter deleter;
  deleter.DeleteElementLater(bin);
  QCoreApplication::sendPostedEvents(&deleter);

  EXPECT_FALSE(finalized);
  EXPECT_EQ(GST_STATE_NULL, GST_STATE(bin));

  gst_object_unref(bin);
  EXPECT_TRUE(finalized);
}

}  // namespace
