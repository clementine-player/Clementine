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

#ifndef SIMPLESEARCHPROVIDER_H
#define SIMPLESEARCHPROVIDER_H

#include "searchprovider.h"

class SimpleSearchProvider : public BlockingSearchProvider {
public:
  SimpleSearchProvider(QObject* parent);

  static const int kDefaultResultLimit;

  // BlockingSearchProvider
  ResultList Search(int id, const QString& query);

  // SearchProvider
  void LoadTracksAsync(int id, const Result& result);

protected:
  struct Item {
    Item() {}
    Item(const QString& title, const QUrl& url,
         const QString& keyword = QString());

    QString keyword_;
    Song metadata_;
  };
  typedef QList<Item> ItemList;

  int result_limit() const { return result_limit_; }
  void set_result_limit(int result_limit) { result_limit_ = result_limit; }
  QStringList safe_words() const { return safe_words_; }
  void set_safe_words(const QStringList& safe_words) { safe_words_ = safe_words; }

  void SetItems(const ItemList& items);

private:
  int result_limit_;
  QStringList safe_words_;

  QMutex items_mutex_;
  ItemList items_;
};

#endif // SIMPLESEARCHPROVIDER_H
