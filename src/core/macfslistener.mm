#include "macfslistener.h"

#include <CoreFoundation/CFArray.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSString.h>

#include "core/logging.h"

MacFSListener::MacFSListener()
    : run_loop_(NULL),
      stream_(NULL) {
}

void MacFSListener::Init() {
  run_loop_ = CFRunLoopGetCurrent();
}

void MacFSListener::EventStreamCallback(
    ConstFSEventStreamRef stream,
    void* user_data,
    size_t num_events,
    void* event_paths,
    const FSEventStreamEventFlags event_flags[],
    const FSEventStreamEventId event_ids[]) {
  MacFSListener* me = reinterpret_cast<MacFSListener*>(user_data);
  char** paths = reinterpret_cast<char**>(event_paths);
  for (int i = 0; i < num_events; ++i) {
    QString path = QString::fromUtf8(paths[i]);
    qLog(Debug) << "Something changed at:" << path;
    emit me->PathChanged(path);
  }
}

void MacFSListener::AddPath(const QString& path) {
  paths_.insert(path);
  UpdateStream();
}

void MacFSListener::UpdateStream() {
  if (stream_) {
    FSEventStreamInvalidate(stream_);
    stream_ = NULL;
  }

  if (paths_.empty()) {
    return;
  }

  NSMutableArray* array = [[NSMutableArray alloc] init];

  foreach (const QString& path, paths_) {
    NSString* string = [[NSString alloc] initWithUTF8String: path.toUtf8().constData()];
    [array addObject: string];
    [string release];
  }

  FSEventStreamContext context;
  memset(&context, 0, sizeof(context));
  context.info = this;
  CFAbsoluteTime latency = 1.0;

  stream_ = FSEventStreamCreate(
      NULL,
      &EventStreamCallback,
      &context,  // Copied
      (CFArrayRef)array,
      kFSEventStreamEventIdSinceNow,
      latency,
      kFSEventStreamCreateFlagNone);

  FSEventStreamScheduleWithRunLoop(stream_, run_loop_, kCFRunLoopDefaultMode);
  FSEventStreamStart(stream_);

  [array release];
}
