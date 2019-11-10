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

#ifndef SMARTPLAYLISTSEARCHTERMWIDGET_H
#define SMARTPLAYLISTSEARCHTERMWIDGET_H

#include "searchterm.h"

#include <QPushButton>
#include <QWidget>

class LibraryBackend;
class Ui_SmartPlaylistSearchTermWidget;

class QPropertyAnimation;

namespace smart_playlists {

class SearchTermWidget : public QWidget {
  Q_OBJECT
  Q_PROPERTY(float overlay_opacity READ overlay_opacity WRITE
                 set_overlay_opacity)

 public:
  SearchTermWidget(LibraryBackend* library, QWidget* parent);
  ~SearchTermWidget();

  void SetActive(bool active);

  float overlay_opacity() const;
  void set_overlay_opacity(float opacity);

  void SetTerm(const SearchTerm& term);
  SearchTerm Term() const;

signals:
  void Clicked();
  void RemoveClicked();

  void Changed();

 protected:
  void showEvent(QShowEvent*);
  void enterEvent(QEvent*);
  void leaveEvent(QEvent*);
  void resizeEvent(QResizeEvent*);

 private slots:
  void FieldChanged(int index);
  void OpChanged(int index);
  void RelativeValueChanged();
  void Grab();

 private:
  class Overlay;
  friend class Overlay;

  Ui_SmartPlaylistSearchTermWidget* ui_;
  LibraryBackend* library_;

  Overlay* overlay_;
  QPropertyAnimation* animation_;
  bool active_;
  bool initialized_;

  SearchTerm::Type current_field_type_;
};

}  // namespace

#endif  // SMARTPLAYLISTSEARCHTERMWIDGET_H
