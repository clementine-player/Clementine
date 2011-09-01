#include "globalshortcutgrabber.h"

#import <AppKit/NSEvent.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSViewController.h>
#import <QuartzCore/CALayer.h>

#include <QKeySequence>

#include <boost/noncopyable.hpp>

#import "core/mac_utilities.h"

class MacMonitorWrapper : boost::noncopyable {
 public:
  explicit MacMonitorWrapper(id monitor)
      : local_monitor_(monitor) {
  }

  ~MacMonitorWrapper() {
    [NSEvent removeMonitor: local_monitor_];
  }

 private:
  id local_monitor_;
};

bool GlobalShortcutGrabber::HandleMacEvent(NSEvent* event) {
  ret_ = mac::KeySequenceFromNSEvent(event);
  UpdateText();
  if ([[event charactersIgnoringModifiers] length] != 0) {
    accept();
    return true;
  }
  return ret_ == QKeySequence(Qt::Key_Escape);
}

void GlobalShortcutGrabber::SetupMacEventHandler() {
  id monitor = [NSEvent addLocalMonitorForEventsMatchingMask: NSKeyDownMask
      handler:^(NSEvent* event) {
        return HandleMacEvent(event) ? event : nil;
      }];
  wrapper_ = new MacMonitorWrapper(monitor);
}

void GlobalShortcutGrabber::TeardownMacEventHandler() {
  delete wrapper_;
}
