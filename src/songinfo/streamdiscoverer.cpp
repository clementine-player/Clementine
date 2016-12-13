#include "streamdiscoverer.h"

#include <gst/pbutils/pbutils.h>
#include "core/logging.h"
#include "core/signalchecker.h"

#include <QEventLoop>

const unsigned int StreamDiscoverer::kDiscoveryTimeoutS = 10;

StreamDiscoverer::StreamDiscoverer() : QObject(nullptr) {
  // Setting up a discoverer:
  discoverer_ = gst_discoverer_new(kDiscoveryTimeoutS * GST_SECOND, NULL);
  if (discoverer_ == NULL) {
    qLog(Error) << "Error creating discoverer" << endl;
    return;
  }

  // Connecting its signals:
  CHECKED_GCONNECT(discoverer_, "discovered", &on_discovered_cb, this);
  CHECKED_GCONNECT(discoverer_, "finished", &on_finished_cb, this);

  // Starting the discoverer process:
  gst_discoverer_start(discoverer_);
}

StreamDiscoverer::~StreamDiscoverer() {
  gst_discoverer_stop(discoverer_);
  g_object_unref(discoverer_);
}

void StreamDiscoverer::Discover(QString url) {
  // Adding the request to discover the url given as a parameter:
  qLog(Debug) << "Discover" << url;
  std::string url_std = url.toStdString();
  const char* url_c = url_std.c_str();
  if (!gst_discoverer_discover_uri_async(discoverer_, url_c)) {
    qLog(Error) << "Failed to start discovering" << url
                << endl;
    g_object_unref(discoverer_);
    return;
  }

  // Creating a loop and setting it to run. That way we can wait for signals.
  QEventLoop loop;
  loop.connect(this, SIGNAL(DiscoverererFinished()), SLOT(quit()));
  loop.exec();
}

void StreamDiscoverer::on_discovered_cb(GstDiscoverer* discoverer,
                                        GstDiscovererInfo* info, GError* err,
                                        gpointer self) {
  StreamDiscoverer* instance = reinterpret_cast<StreamDiscoverer*>(self);

  QString discovered_url(gst_discoverer_info_get_uri(info));

  GstDiscovererResult result = gst_discoverer_info_get_result(info);
  if (result != GST_DISCOVERER_OK) {
    qLog(Error) << "Discovery failed:" << gstDiscovererErrorMessage(result) << endl;
    emit instance->Error(tr("Error discovering %1: %2").arg(discovered_url).arg(
        gstDiscovererErrorMessage(result)));
    return;
  }

  // Get audio streams (we will only care about the first one, which should be the only one).
  GList* audio_streams = gst_discoverer_info_get_audio_streams(info);

  if (audio_streams != NULL) {
    qLog(Debug) << "Discovery successful" << endl;
    // We found a valid audio stream, extracting and saving its info:
    GstDiscovererStreamInfo* stream_audio_info =
        (GstDiscovererStreamInfo*)g_list_first(audio_streams)->data;

    StreamDetails stream_details;
    stream_details.url = discovered_url;
    stream_details.bitrate = gst_discoverer_audio_info_get_bitrate(
        GST_DISCOVERER_AUDIO_INFO(stream_audio_info));
    stream_details.channels = gst_discoverer_audio_info_get_channels(
        GST_DISCOVERER_AUDIO_INFO(stream_audio_info));
    stream_details.depth = gst_discoverer_audio_info_get_depth(
        GST_DISCOVERER_AUDIO_INFO(stream_audio_info));
    stream_details.sample_rate = gst_discoverer_audio_info_get_sample_rate(
        GST_DISCOVERER_AUDIO_INFO(stream_audio_info));

    // Human-readable codec name:
    GstCaps* stream_caps =
        gst_discoverer_stream_info_get_caps(stream_audio_info);
    gchar* decoder_description =
        gst_pb_utils_get_codec_description(stream_caps);
    stream_details.format = (decoder_description == NULL)
                                ? QString(tr("Unknown"))
                                : QString(decoder_description);

    gst_caps_unref(stream_caps);
    g_free(decoder_description);

    emit instance->DataReady(stream_details);

  } else {
    emit instance->Error(
        tr("Could not detect an audio stream in %1").arg(discovered_url));
  }

  gst_discoverer_stream_info_list_free(audio_streams);
}

void StreamDiscoverer::on_finished_cb(GstDiscoverer* discoverer,
                                      gpointer self) {
  // The discoverer doesn't have any more urls in its queue. Let the loop know
  // it can exit.
  StreamDiscoverer* instance = reinterpret_cast<StreamDiscoverer*>(self);
  emit instance->DiscoverererFinished();
}

QString StreamDiscoverer::gstDiscovererErrorMessage(
    GstDiscovererResult result) {
  switch (result) {
    case (GST_DISCOVERER_URI_INVALID):
      return tr("Invalid URL");
    case (GST_DISCOVERER_TIMEOUT):
      return tr("Connection timed out");
    case (GST_DISCOVERER_BUSY):
      return tr("The discoverer is busy");
    case (GST_DISCOVERER_MISSING_PLUGINS):
      return tr("Missing plugins");
    case (GST_DISCOVERER_ERROR):
    default:
      return tr("Could not get details");
  }
}
