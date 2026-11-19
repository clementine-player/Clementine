/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>

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

#include "macfslistener.h"

#include <CoreFoundation/CFArray.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSString.h>

#include <QTimer>

#include "core/logging.h"
#include "core/scoped_nsobject.h"

MacFSListener::MacFSListener(QObject* parent)
    : FileSystemWatcherInterface(parent),
      queue_(dispatch_queue_create("org.clementine-player.fslistener",
                                   DISPATCH_QUEUE_SERIAL)),
      stream_(nullptr),
      update_timer_(new QTimer(this)) {
  update_timer_->setSingleShot(true);
  update_timer_->setInterval(2000);
  connect(update_timer_, SIGNAL(timeout()), SLOT(UpdateStream()));
}

MacFSListener::~MacFSListener() {
  StopStream();
  dispatch_release(queue_);
}

void MacFSListener::EventStreamCallback(ConstFSEventStreamRef stream, void* user_data,
                                        size_t num_events, void* event_paths,
                                        const FSEventStreamEventFlags event_flags[],
                                        const FSEventStreamEventId event_ids[]) {
  MacFSListener* me = reinterpret_cast<MacFSListener*>(user_data);
  char** paths = reinterpret_cast<char**>(event_paths);
  for (int i = 0; i < num_events; ++i) {
    QString path = QString::fromUtf8(paths[i]);
    qLog(Debug) << "Something changed at:" << path;
    while (path.endsWith('/')) {
      path.chop(1);
    }
    emit me->PathChanged(path);
  }
}

bool MacFSListener::AddPath(const QString& path) {
  paths_.insert(path);
  UpdateStreamAsync();
  return true;
}

void MacFSListener::RemovePath(const QString& path) {
  paths_.remove(path);
  UpdateStreamAsync();
}

void MacFSListener::Clear() {
  paths_.clear();
  UpdateStreamAsync();
}

void MacFSListener::UpdateStreamAsync() { update_timer_->start(); }

// The stream's callback is handed this listener, so once this returns, the
// callback mustn't be running or be called again.
void MacFSListener::StopStream() {
  if (!stream_) return;

  FSEventStreamStop(stream_);
  FSEventStreamInvalidate(stream_);
  FSEventStreamRelease(stream_);
  stream_ = nullptr;

  // Stopping the stream stops new callbacks, but one can already be running
  // on the queue. The queue is serial, so this waits for it to finish.
  dispatch_sync(queue_, ^{
  });
}

void MacFSListener::UpdateStream() {
  StopStream();

  if (paths_.empty()) {
    return;
  }

  scoped_nsobject<NSMutableArray> array([[NSMutableArray alloc] init]);

  for (const QString& path : paths_) {
    scoped_nsobject<NSString> string(
        [[NSString alloc] initWithUTF8String:path.toUtf8().constData()]);
    [array addObject:string.get()];
  }

  FSEventStreamContext context;
  memset(&context, 0, sizeof(context));
  context.info = this;
  CFAbsoluteTime latency = 1.0;

  stream_ =
      FSEventStreamCreate(nullptr, &EventStreamCallback, &context,  // Copied
                          reinterpret_cast<CFArrayRef>(array.get()), kFSEventStreamEventIdSinceNow,
                          latency, kFSEventStreamCreateFlagNone);

  FSEventStreamSetDispatchQueue(stream_, queue_);
  FSEventStreamStart(stream_);
}
