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

#include <memory>

#include "songinfobase.h"

class UltimateLyricsProvider;
class UltimateLyricsReader;

class SongInfoView : public SongInfoBase {
  Q_OBJECT

public:
  SongInfoView(QWidget* parent = 0);
  ~SongInfoView();

  static const char* kSettingsGroup;

  QList<const UltimateLyricsProvider*> lyric_providers() const;

public slots:
  void ReloadSettings();

protected:
  bool NeedsUpdate(const Song& old_metadata, const Song& new_metadata) const;

protected slots:
  void ResultReady(int id, const SongInfoFetcher::Result& result);

private:
  SongInfoProvider* ProviderByName(const QString& name) const;

private slots:
  void UltimateLyricsParsed();

private:
  std::unique_ptr<UltimateLyricsReader> ultimate_reader_;
};

#endif // SONGINFOVIEW_H
