#ifndef MACFSLISTENER_H
#define MACFSLISTENER_H

#include <CoreServices/CoreServices.h>

#include <QObject>
#include <QSet>

class MacFSListener : public QObject {
  Q_OBJECT

 public:
  explicit MacFSListener(QObject* parent = 0);
  void Init();

  void AddPath(const QString& path);

 signals:
  void PathChanged(const QString& path);

 private:
  void UpdateStream();

  static void EventStreamCallback(
      ConstFSEventStreamRef stream,
      void* user_data,
      size_t num_events,
      void* event_paths,
      const FSEventStreamEventFlags event_flags[],
      const FSEventStreamEventId event_ids[]);

  CFRunLoopRef run_loop_;
  FSEventStreamRef stream_;

  QSet<QString> paths_;
};

#endif
