/* This file is part of Clementine.
   Copyright 2011, 2013, David Sansome <me@davidsansome.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#ifndef CORE_CACHEDLIST_H_
#define CORE_CACHEDLIST_H_

#include <algorithm>

#include <QDateTime>
#include <QSettings>

template <typename T>
class CachedList {
 public:
  // Use a CachedList when you want to download and save a list of things from a
  // remote service, updating it only periodically.
  // T must be a registered metatype and must support being stored in
  // QSettings.  This usually means you have to implement QDataStream streaming
  // operators, and use qRegisterMetaTypeStreamOperators.

  typedef QList<T> ListType;

  CachedList(const QString& settings_group, const QString& name,
             int cache_duration_secs)
      : settings_group_(settings_group),
        name_(name),
        cache_duration_secs_(cache_duration_secs) {}

  void Load() {
    QSettings s;
    s.beginGroup(settings_group_);

    last_updated_ = s.value("last_refreshed_" + name_).toDateTime();
    data_.clear();

    const int count = s.beginReadArray(name_ + "_data");
    for (int i = 0; i < count; ++i) {
      s.setArrayIndex(i);
      data_ << s.value("value").value<T>();
    }
    s.endArray();
  }

  void Save() const {
    QSettings s;
    s.beginGroup(settings_group_);

    s.setValue("last_refreshed_" + name_, last_updated_);

    s.beginWriteArray(name_ + "_data", data_.size());
    for (int i = 0; i < data_.size(); ++i) {
      s.setArrayIndex(i);
      s.setValue("value", QVariant::fromValue(data_[i]));
    }
    s.endArray();
  }

  void Update(const ListType& data) {
    data_ = data;
    last_updated_ = QDateTime::currentDateTime();
    Save();
  }

  bool IsStale() const {
    return last_updated_.isNull() ||
           last_updated_.secsTo(QDateTime::currentDateTime()) >
               cache_duration_secs_;
  }

  void Sort() { std::sort(data_.begin(), data_.end()); }

  const ListType& Data() const { return data_; }
  operator ListType() const { return data_; }

  // Q_FOREACH support
  typedef typename ListType::const_iterator const_iterator;
  const_iterator begin() const { return data_.begin(); }
  const_iterator end() const { return data_.end(); }

 private:
  const QString settings_group_;
  const QString name_;
  const int cache_duration_secs_;

  QDateTime last_updated_;
  ListType data_;
};

#endif  // CORE_CACHEDLIST_H_
