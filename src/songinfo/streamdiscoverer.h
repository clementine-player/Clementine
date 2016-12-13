#ifndef STREAMDISCOVERER_H
#define STREAMDISCOVERER_H

#include <gst/pbutils/pbutils.h>

#include <QMetaType>
#include <QObject>
#include <QString>

struct StreamDetails {
  QString url;
  QString format;
  unsigned int bitrate;
  unsigned int depth;
  unsigned int channels;
  unsigned int sample_rate;
};
Q_DECLARE_METATYPE(StreamDetails)

class StreamDiscoverer : public QObject {
  Q_OBJECT

 public:
  StreamDiscoverer();
  ~StreamDiscoverer();

  void Discover(const QString& url);

signals:
  void DiscoverererFinished();
  void DataReady(StreamDetails data);
  void Error(const QString& message);

 private:
  GstDiscoverer* discoverer_;

  static const unsigned int kDiscoveryTimeoutS;

  // GstDiscoverer callbacks:
  static void on_discovered_cb(GstDiscoverer* discoverer,
                               GstDiscovererInfo* info, GError* err,
                               gpointer instance);
  static void on_finished_cb(GstDiscoverer* discoverer, gpointer instance);

  // Helper to return descriptive error messages:
  static QString gstDiscovererErrorMessage(GstDiscovererResult result);
};

#endif  // STREAMDISCOVERER_H
