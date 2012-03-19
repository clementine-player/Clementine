/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "test_utils.h"

#include <QDir>
#include <QNetworkRequest>
#include <QString>
#include <QUrl>

std::ostream& operator<<(std::ostream& stream, const QString& str) {
  stream << str.toStdString();
  return stream;
}

std::ostream& operator <<(std::ostream& stream, const QUrl& url) {
  stream << url.toString().toStdString();
  return stream;
}

std::ostream& operator <<(std::ostream& stream, const QNetworkRequest& req) {
  stream << req.url().toString().toStdString();
  return stream;
}

std::ostream& operator <<(std::ostream& stream, const QVariant& var) {
  stream << var.toString().toStdString();
  return stream;
}

void PrintTo(const ::QString& str, std::ostream& os) {
  os << str.toStdString();
}

void PrintTo(const ::QVariant& var, std::ostream& os) {
  os << var.toString().toStdString();
}

void PrintTo(const ::QUrl& url, std::ostream& os) {
  os << url.toString().toStdString();
}

TemporaryResource::TemporaryResource(const QString& filename) {
  setFileTemplate(QDir::tempPath() + "/clementine_test-XXXXXX." +
                  filename.section('.', -1, -1));
  open();

  QFile resource(filename);
  resource.open(QIODevice::ReadOnly);
  write(resource.readAll());

  reset();
}

TestQObject::TestQObject(QObject* parent)
  : QObject(parent),
    invoked_(0) {
}

void TestQObject::Emit() {
  emit Emitted();
}

void TestQObject::Invoke() {
  ++invoked_;
}
