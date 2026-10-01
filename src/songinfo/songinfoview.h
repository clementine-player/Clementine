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

#ifndef SONGINFOVIEW_H
#define SONGINFOVIEW_H

#include <QDateTime>
#include <QPointer>

#include "songinfobase.h"

class Player;
class SyncedLyricsView;

class QTimer;

class SongInfoView : public SongInfoBase {
  Q_OBJECT

 public:
  SongInfoView(QWidget* parent = nullptr);
  ~SongInfoView();

  static const char* kSettingsGroup;

  // Where timed lyrics get the song's position from.
  void SetPlayer(Player* player);

 protected:
  bool NeedsUpdate(const Song& old_metadata, const Song& new_metadata) const;
  void showEvent(QShowEvent* e) override;
  void hideEvent(QHideEvent* e) override;

 protected slots:
  virtual void InfoResultReady(int id, const CollapsibleInfoPane::Data& data);
  virtual void ResultReady(int id, const SongInfoFetcher::Result& result);

 private:
  void UpdateLyricsTimer();
  void UpdateLyricsPosition();
  void ScrollToLyricsLine(int y, int height);

  Player* player_;
  QTimer* lyrics_timer_;
  // The current song's timed lyrics, if it has them.
  QPointer<SyncedLyricsView> synced_lyrics_;
  // When the user last scrolled the pane: we leave it alone for a while.
  QDateTime user_scrolled_;
};

#endif  // SONGINFOVIEW_H
