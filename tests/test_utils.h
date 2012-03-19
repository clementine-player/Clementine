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

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <iostream>

#include <QMetaType>
#include <QModelIndex>
#include <QTemporaryFile>

class QNetworkRequest;
class QString;
class QUrl;
class QVariant;

std::ostream& operator <<(std::ostream& stream, const QString& str);
std::ostream& operator <<(std::ostream& stream, const QVariant& var);
std::ostream& operator <<(std::ostream& stream, const QUrl& url);
std::ostream& operator <<(std::ostream& stream, const QNetworkRequest& req);

template <typename T>
std::ostream& operator <<(std::ostream& stream, const QList<T>& list) {
  stream << "QList(";
  foreach (const T& item, list) {
    stream << item << ",";
  }
  stream << ")";
  return stream;
}

void PrintTo(const ::QString& str, std::ostream& os);
void PrintTo(const ::QVariant& var, std::ostream& os);
void PrintTo(const ::QUrl& url, std::ostream& os);

#define EXPOSE_SIGNAL0(n) \
    void Emit##n() { emit n(); }
#define EXPOSE_SIGNAL1(n, t1) \
    void Emit##n(const t1& a1) { emit n(a1); }
#define EXPOSE_SIGNAL2(n, t1, t2) \
    void Emit##n(const t1& a1, const t2& a2) { emit n(a1, a2); }

Q_DECLARE_METATYPE(QModelIndex);

class TemporaryResource : public QTemporaryFile {
public:
  TemporaryResource(const QString& filename);
};

class TestQObject : public QObject {
  Q_OBJECT
 public:
  TestQObject(QObject* parent = 0);

  void Emit();

  int invoked() const { return invoked_; }

 signals:
  void Emitted();

 public slots:
  void Invoke();

 private:
  int invoked_;
};

#endif  // TEST_UTILS_H
