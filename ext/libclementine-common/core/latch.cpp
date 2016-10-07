/* This file is part of Clementine.
   Copyright 2016, John Maguire <john.maguire@gmail.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "latch.h"

#include "core/logging.h"

CountdownLatch::CountdownLatch() : count_(0) {}

void CountdownLatch::Wait() {
  QMutexLocker l(&mutex_);
  ++count_;
}

void CountdownLatch::CountDown() {
  QMutexLocker l(&mutex_);
  Q_ASSERT(count_ > 0);
  --count_;
  if (count_ == 0) {
    emit Done();
  }
}
