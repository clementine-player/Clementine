/* This file is part of Clementine.
   Copyright 2010-2012, David Sansome <me@davidsansome.com>
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

#ifndef CORE_MPRIS_H_
#define CORE_MPRIS_H_

#include <QObject>

class Application;

namespace mpris {

class Mpris1;
class Mpris2;

class Mpris : public QObject {
  Q_OBJECT

 public:
  explicit Mpris(Application* app, QObject* parent = nullptr);

 signals:
  void RaiseMainWindow();

 private:
  Mpris1* mpris1_ = nullptr;
  Mpris2* mpris2_;
};

}  // namespace mpris

#endif  // CORE_MPRIS_H_
