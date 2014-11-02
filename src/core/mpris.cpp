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

#include "mpris.h"
#include "mpris1.h"
#include "mpris2.h"

namespace mpris {

Mpris::Mpris(Application* app, QObject* parent)
    : QObject(parent),
      mpris1_(new mpris::Mpris1(app, this)),
      mpris2_(new mpris::Mpris2(app, mpris1_, this)) {
  connect(mpris2_, SIGNAL(RaiseMainWindow()), SIGNAL(RaiseMainWindow()));
}

}  // namespace mpris
