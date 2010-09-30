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

#include "lyricview.h"
#include "songinfobuttonbox.h"

#include <QSettings>
#include <QSplitter>
#include <QStackedWidget>

const char* SongInfoButtonBox::kSettingsGroup = "SongInfo";

SongInfoButtonBox::SongInfoButtonBox(QWidget* parent)
  : SlimButtonBox(parent),
    splitter_(NULL),
    stack_(NULL),
    lyrics_(NULL)
{
  setEnabled(false);
  AddButton(tr("Lyrics"));
  AddButton(tr("Song info"));
  AddButton(tr("Artist info"));

  connect(this, SIGNAL(CurrentChanged(int)), SLOT(SetActive(int)));
}

void SongInfoButtonBox::SetSplitter(QSplitter* splitter) {
  splitter_ = splitter;
  connect(splitter_, SIGNAL(splitterMoved(int,int)), SLOT(SplitterSizeChanged()));

  stack_ = new QStackedWidget;
  splitter->addWidget(stack_);
  stack_->setVisible(false);

  lyrics_ = new LyricView;
  stack_->addWidget(lyrics_);
  stack_->addWidget(new QWidget);
  stack_->addWidget(new QWidget);

  // Restore settings
  QSettings s;
  s.beginGroup(kSettingsGroup);

  const int current_index = s.value("current_index", -1).toInt();
  SetCurrentButton(current_index);
  SetActive(current_index);

  if (!splitter_->restoreState(s.value("splitter_state").toByteArray())) {
    // Set sensible default sizes
    splitter_->setSizes(QList<int>() << 850 << 150);
  }
}

LyricFetcher* SongInfoButtonBox::lyric_fetcher() const {
  return lyrics_->fetcher();
}

void SongInfoButtonBox::SongChanged(const Song& metadata) {
  setEnabled(true);

  metadata_ = metadata;

  if (IsAnyButtonChecked()) {
    stack_->show();
    UpdateCurrentSong();
  }
}

void SongInfoButtonBox::SongFinished() {
  metadata_ = Song();
  setEnabled(false);
  stack_->hide();
}

void SongInfoButtonBox::SetActive(int index) {
  if (index == -1) {
    stack_->hide();
  } else {
    if (isEnabled())
      stack_->show();
    stack_->setCurrentIndex(index);
    if (metadata_.is_valid())
      UpdateCurrentSong();
  }

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("current_index", index);
}

void SongInfoButtonBox::SplitterSizeChanged() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("splitter_state", splitter_->saveState());
}

void SongInfoButtonBox::UpdateCurrentSong() {
  QMetaObject::invokeMethod(stack_->currentWidget(), "SongChanged",
                            Q_ARG(Song, metadata_));
}
