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
  Q_OBJECT

 public:
  SimpleSearchProvider(Application* app, QObject* parent);

  static const int kDefaultResultLimit;

  // BlockingSearchProvider
  ResultList Search(int id, const QString& query);

  // SearchProvider
  QStringList GetSuggestions(int count);

 protected slots:
  // Calls RecreateItems now if the user has done a global search with this
  // provider at least once before.  Otherwise will schedule RecreateItems the
  // next time the user does a search.
  void MaybeRecreateItems();

 protected:
  struct Item {
    Item() {}
    Item(const QString& title, const QUrl& url,
         const QString& keyword = QString());
    Item(const Song& song, const QString& keyword = QString());

    QString keyword_;
    Song metadata_;
  };
  typedef QList<Item> ItemList;

  int result_limit() const { return result_limit_; }
  void set_result_limit(int result_limit) { result_limit_ = result_limit; }
  void set_max_suggestion_count(int count) { max_suggestion_count_ = count; }
  QStringList safe_words() const { return safe_words_; }
  void set_safe_words(const QStringList& safe_words) {
    safe_words_ = safe_words;
  }

  void SetItems(const ItemList& items);

  // Subclasses should fetch the list of items they want to show in results and
  // call SetItems with the new list.
  virtual void RecreateItems() = 0;

 private:
  int result_limit_;
  QStringList safe_words_;
  int max_suggestion_count_;

  QMutex items_mutex_;
  ItemList items_;

  bool items_dirty_;
  bool has_searched_before_;
};

#endif  // SIMPLESEARCHPROVIDER_H
