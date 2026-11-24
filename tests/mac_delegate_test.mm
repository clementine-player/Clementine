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

#import <AppKit/NSApplication.h>

#include "core/mac_delegate.h"

#include <QEventLoop>
#include <QTimer>

#include "gtest/gtest.h"

namespace {

TEST(MacDelegateTest, TerminatesNowOutsideTheEventLoop) {
  AppDelegate* delegate = [[AppDelegate alloc] initWithHandler:nullptr];
  EXPECT_EQ(NSTerminateNow, [delegate applicationShouldTerminate:NSApp]);
}

// AppKit calls exit() when the delegate answers NSTerminateNow, so quitting
// from inside the event loop must leave the loop instead (CLEMENTINE-G).
TEST(MacDelegateTest, LeavesTheEventLoopInsteadOfTerminating) {
  AppDelegate* delegate = [[AppDelegate alloc] initWithHandler:nullptr];
  NSApplicationTerminateReply reply = NSTerminateNow;

  QEventLoop loop;
  QTimer::singleShot(0, &loop, [&]() { reply = [delegate applicationShouldTerminate:NSApp]; });
  // Fails the test, rather than hanging it, if the delegate doesn't leave the
  // loop.
  QTimer timeout;
  timeout.setSingleShot(true);
  QObject::connect(&timeout, &QTimer::timeout, &loop, [&]() { loop.exit(1); });
  timeout.start(30000);

  EXPECT_EQ(0, loop.exec());
  EXPECT_EQ(NSTerminateCancel, reply);
}

}  // namespace
