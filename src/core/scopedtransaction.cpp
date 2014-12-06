/* This file is part of Clementine.
   Copyright 2010-2011, David Sansome <davidsansome@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#include "scopedtransaction.h"
#include "core/logging.h"

#include <QSqlDatabase>
#include <QtDebug>

ScopedTransaction::ScopedTransaction(QSqlDatabase* db)
    : db_(db), pending_(true) {
  db->transaction();
}

ScopedTransaction::~ScopedTransaction() {
  if (pending_) {
    qLog(Warning) << "Rolling back transaction";
    db_->rollback();
  }
}

void ScopedTransaction::Commit() {
  if (!pending_) {
    qLog(Warning) << "Tried to commit a ScopedTransaction twice";
    return;
  }

  db_->commit();
  pending_ = false;
}
