#ifndef STREAMDISCOVERER_H
#define STREAMDISCOVERER_H

#include <gst/pbutils/pbutils.h>

#include <QMetaType>
#include <QObject>
#include <QString>

struct StreamDetails {
  QString url;
  QString format;
  int bitrate;
  int depth;
  int channels;
  int sample_rate;
};
Q_DECLARE_METATYPE(StreamDetails)

class StreamDiscoverer : public QObject {
  Q_OBJECT

 public:
  StreamDiscoverer();
  ~StreamDiscoverer();

  void Discover(const QString& url);

signals:
  void DiscoverFinished();
  void DataReady(const StreamDetails& data);
  void Error(const QString& message);

 private:
  GstDiscoverer* discoverer_;

  static const int kDiscoveryTimeoutS;

  // GstDiscoverer callbacks:
  static void OnDiscovered(GstDiscoverer* discoverer, GstDiscovererInfo* info,
                           GError* err, gpointer instance);
  static void OnFinished(GstDiscoverer* discoverer, gpointer instance);

  // Helper to return descriptive error messages:
  static QString GSTdiscovererErrorMessage(GstDiscovererResult result);
};

#endif  // STREAMDISCOVERER_H
