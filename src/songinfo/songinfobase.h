/* This file is part of Clementine.

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

#ifndef SONGINFOBASE_H
#define SONGINFOBASE_H

#include <QWidget>

#include "collapsibleinfopane.h"
#include "songinfofetcher.h"
#include "core/song.h"
#include "widgets/widgetfadehelper.h"

class CollapsibleInfoPane;
class NetworkAccessManager;
class WidgetFadeHelper;

class QScrollArea;
class QVBoxLayout;

class SongInfoBase : public QWidget {
  Q_OBJECT

public:
  SongInfoBase(NetworkAccessManager* network, QWidget* parent = 0);

public slots:
  void SongChanged(const Song& metadata);
  void SongFinished();

protected:
  void showEvent(QShowEvent* e);

  virtual void Update(const Song& metadata);
  virtual bool NeedsUpdate(const Song& old_metadata, const Song& new_metadata) const { return true; }

  void AddWidget(QWidget* widget);
  void AddSection(CollapsibleInfoPane* section);
  void Clear();

protected slots:
  virtual void ResultReady(int id, const SongInfoFetcher::Result& result);

protected:
  NetworkAccessManager* network_;
  SongInfoFetcher* fetcher_;
  int current_request_id_;

private:
  void MaybeUpdate(const Song& metadata);

private:
  QScrollArea* scroll_area_;

  QVBoxLayout* container_;
  QList<QWidget*> widgets_;

  QWidget* section_container_;
  QList<CollapsibleInfoPane*> sections_;

  WidgetFadeHelper* fader_;

  Song queued_metadata_;
  Song old_metadata_;
  bool dirty_;
};

#endif // SONGINFOBASE_H
