#ifndef STREAMPIPELINE_H
#define STREAMPIPELINE_H

#include <QIODevice>

#include <gst/gst.h>

#include "core/song.h"

// Transcodes a song into an OGG Vorbis stream.
class StreamPipeline : public QIODevice {
  Q_OBJECT
 public:
  StreamPipeline(QObject* parent = 0);
  virtual ~StreamPipeline();

  void Init(const Song& song);

 private slots:
  void StartPipeline();

 protected:
  // QIODevice
  virtual qint64 readData(char* data, qint64 max_size);
  virtual qint64 writeData(const char* data, qint64 max_size);

 private:
  GstElement* CreateElement(
      const QString& factory_name, GstElement* bin = NULL);
  static void HandleMessage(GstMessage* msg);

  GstElement* pipeline_;
  GstElement* app_sink_;
  GstElement* convert_bin_;
  int bus_cb_id_;

  QByteArray buffer_;

  static gboolean BusCallback(GstBus*, GstMessage*, gpointer);
  static GstBusSyncReply BusCallbackSync(GstBus*, GstMessage*, gpointer);
  static void NewBufferCallback(GstElement*, gpointer);
  static void NewPadCallback(GstElement*, GstPad*, gpointer);

  static const char* kPipeline;
};

#endif
