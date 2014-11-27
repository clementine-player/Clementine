/* This file is part of Clementine.
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
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

#ifndef CORE_BOUNDFUTUREWATCHER_H_
#define CORE_BOUNDFUTUREWATCHER_H_

#include <QFutureWatcher>

#include <boost/noncopyable.hpp>

template <typename T, typename D>
class BoundFutureWatcher : public QFutureWatcher<T>, boost::noncopyable {
 public:
  explicit BoundFutureWatcher(const D& data, QObject* parent = nullptr)
      : QFutureWatcher<T>(parent), data_(data) {}

  ~BoundFutureWatcher() {}

  const D& data() const { return data_; }

 private:
  D data_;
};

#endif  // CORE_BOUNDFUTUREWATCHER_H_
