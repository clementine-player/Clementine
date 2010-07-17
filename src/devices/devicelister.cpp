/* This file is part of Clementine.

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

#include "config.h"
#include "devicelister.h"

#include <QFile>
#include <QThread>
#include <QtDebug>

DeviceLister::DeviceLister()
  : thread_(NULL)
{
}

DeviceLister::~DeviceLister() {
  qDebug() << __PRETTY_FUNCTION__;
  if (thread_) {
    thread_->quit();
    thread_->wait(1000);
  }
}

void DeviceLister::Start() {
  thread_ = new QThread;
  connect(thread_, SIGNAL(started()), SLOT(ThreadStarted()));

  moveToThread(thread_);
  thread_->start();
}

void DeviceLister::ThreadStarted() {
  Init();
}

QUrl DeviceLister::MakeUrlFromLocalPath(const QString &path) {
#ifdef HAVE_LIBGPOD
  if (QFile::exists(path + "/iTunes_Control") ||
      QFile::exists(path + "/iPod_Control")) {
    return QUrl("ipod://" + path);
  }
#endif

  return QUrl::fromLocalFile(path);
}
