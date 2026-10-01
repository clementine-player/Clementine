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

#include "songinfo/songinfoview.h"

#include <QCoreApplication>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>

#include "core/player.h"
#include "core/timeconstants.h"
#include "engines/enginebase.h"
#include "songinfo/lastfmtrackinfoprovider.h"
#include "songinfo/lyricsinfoprovider.h"
#include "songinfo/syncedlyricsview.h"

const char* SongInfoView::kSettingsGroup = "SongInfo";

namespace {
// How often timed lyrics check where the song is.
const int kLyricsUpdateMsec = 100;
// How long to leave the pane where the user scrolled it.
const int kUserScrollSecs = 5;
}  // namespace

SongInfoView::SongInfoView(QWidget* parent)
    : SongInfoBase(parent), player_(nullptr), lyrics_timer_(new QTimer(this)) {
  fetcher_->AddProvider(new LastfmTrackInfoProvider);
  fetcher_->AddProvider(new LyricsInfoProvider);

  lyrics_timer_->setInterval(kLyricsUpdateMsec);
  connect(lyrics_timer_, &QTimer::timeout, this,
          &SongInfoView::UpdateLyricsPosition);
  // Wheel, drag, keys: not sections being added or collapsed.
  connect(scroll_area()->verticalScrollBar(), &QScrollBar::actionTriggered,
          this, [this] { user_scrolled_ = QDateTime::currentDateTime(); });
}

void SongInfoView::SetPlayer(Player* player) { player_ = player; }

SongInfoView::~SongInfoView() {}

bool SongInfoView::NeedsUpdate(const Song& old_metadata,
                               const Song& new_metadata) const {
  if (new_metadata.title().isEmpty() || new_metadata.artist().isEmpty())
    return false;

  return old_metadata.title() != new_metadata.title() ||
         old_metadata.artist() != new_metadata.artist();
}

void SongInfoView::InfoResultReady(int id,
                                   const CollapsibleInfoPane::Data& data) {
  if (id != current_request_id_) return;

  AddSection(new CollapsibleInfoPane(data, this));
  CollapseSections();

  if (SyncedLyricsView* lyrics =
          qobject_cast<SyncedLyricsView*>(data.contents_)) {
    synced_lyrics_ = lyrics;
    connect(lyrics, &SyncedLyricsView::CurrentLineMoved, this,
            &SongInfoView::ScrollToLyricsLine);
    UpdateLyricsTimer();
  }
}

void SongInfoView::showEvent(QShowEvent* e) {
  SongInfoBase::showEvent(e);
  // Once the sections are shown and laid out too.
  QTimer::singleShot(0, this, &SongInfoView::UpdateLyricsTimer);
}

void SongInfoView::hideEvent(QHideEvent* e) {
  SongInfoBase::hideEvent(e);
  UpdateLyricsTimer();
}

void SongInfoView::UpdateLyricsTimer() {
  // Only while there's something to follow the song, and it can be seen.
  if (synced_lyrics_ && player_ && isVisible()) {
    UpdateLyricsPosition();
    // Back to the line being sung, if it hasn't changed since it was hidden.
    synced_lyrics_->EmitCurrentLine();
    lyrics_timer_->start();
  } else {
    lyrics_timer_->stop();
  }
}

void SongInfoView::UpdateLyricsPosition() {
  // Gone with the song it was for.
  if (!synced_lyrics_ || !player_) {
    lyrics_timer_->stop();
    return;
  }
  synced_lyrics_->SetPosition(player_->engine()->position_nanosec() /
                              kNsecPerMsec);
}

void SongInfoView::ScrollToLyricsLine(int y, int height) {
  if (!synced_lyrics_ || !synced_lyrics_->isVisible()) return;
  if (user_scrolled_.isValid() &&
      user_scrolled_.secsTo(QDateTime::currentDateTime()) < kUserScrollSecs) {
    return;
  }

  // The pane grows to fit the lyrics in layout requests that might still be
  // queued, when the section has just been added or shown.
  QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);

  // Keep a third of the pane below the line, for the lines coming up.
  const QPoint point =
      synced_lyrics_->mapTo(scroll_area()->widget(), QPoint(0, y + height / 2));
  scroll_area()->ensureVisible(point.x(), point.y(), 0,
                               scroll_area()->viewport()->height() / 3);
}

void SongInfoView::ResultReady(int id, const SongInfoFetcher::Result& result) {}
