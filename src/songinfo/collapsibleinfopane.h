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

#ifndef COLLAPSIBLEINFOPANE_H
#define COLLAPSIBLEINFOPANE_H

#include <QIcon>
#include <QWidget>

class CollapsibleInfoHeader;

class CollapsibleInfoPane : public QWidget {
  Q_OBJECT

public:
  struct Data {
    Data() : type_(Type_Biography), relevance_(0) {}

    bool operator <(const Data& other) const;

    enum Type {
      Type_PlayCounts,
      Type_Tags,
      Type_Similar,
      Type_Biography,
      Type_Lyrics,

      TypeCount
    };

    QString id_;
    QString title_;
    QIcon icon_;
    Type type_;
    int relevance_;

    QWidget* contents_;
    QObject* content_object_;
  };

  CollapsibleInfoPane(const Data& data, QWidget* parent = 0);

  const Data& data() const { return data_; }

public slots:
  void Expand();
  void Collapse();

signals:
  void Toggled(bool expanded);

private slots:
  void ExpandedToggled(bool expanded);

private:
  Data data_;
  CollapsibleInfoHeader* header_;
};

#endif // COLLAPSIBLEINFOPANE_H
