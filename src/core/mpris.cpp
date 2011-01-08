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

#include "mpris.h"
#include "mpris1.h"
#include "mpris2.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentRun>

namespace mpris {

Mpris::Mpris(Player* player, ArtLoader* art_loader, QObject* parent)
  : QObject(parent),
    player_(player),
    art_loader_(art_loader),
    mpris1_(NULL),
    mpris2_(NULL)
{
  QFuture<void> future = QtConcurrent::run(this, &Mpris::Init);

  QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
  watcher->setFuture(future);

  connect(watcher, SIGNAL(finished()), SLOT(Initialised()));
}

void Mpris::Init() {
  mpris1_ = new mpris::Mpris1(player_, art_loader_);
  mpris2_ = new mpris::Mpris2(player_, art_loader_, mpris1_);

  mpris1_->moveToThread(thread());
  mpris2_->moveToThread(thread());

  mpris1_->setParent(this);
  mpris2_->setParent(this);

  connect(mpris2_, SIGNAL(RaiseMainWindow()), SIGNAL(RaiseMainWindow()));
}

void Mpris::Initialised() {
  mpris2_->InitLibIndicate();
}

} // namespace mpris
