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

#include "config.h"
#include "songinfoprovider.h"
#include "songinfoview.h"
#include "ultimatelyricsprovider.h"
#include "ultimatelyricsreader.h"

#ifdef HAVE_LIBLASTFM
#include "lastfmtrackinfoprovider.h"
#endif

#include <QFuture>
#include <QFutureWatcher>
#include <QSettings>
#include <QtConcurrentRun>

const char* SongInfoView::kSettingsGroup = "SongInfo";

typedef QList<SongInfoProvider*> ProviderList;

SongInfoView::SongInfoView(QWidget* parent)
    : SongInfoBase(parent), ultimate_reader_(new UltimateLyricsReader(this)) {
  // Parse the ultimate lyrics xml file in the background
  QFuture<ProviderList> future =
      QtConcurrent::run(ultimate_reader_.get(), &UltimateLyricsReader::Parse,
                        QString(":lyrics/ultimate_providers.xml"));
  QFutureWatcher<ProviderList>* watcher =
      new QFutureWatcher<ProviderList>(this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(UltimateLyricsParsed()));

#ifdef HAVE_LIBLASTFM
  fetcher_->AddProvider(new LastfmTrackInfoProvider);
#endif
}

SongInfoView::~SongInfoView() {}

void SongInfoView::UltimateLyricsParsed() {
  QFutureWatcher<ProviderList>* watcher =
      static_cast<QFutureWatcher<ProviderList>*>(sender());

  for (SongInfoProvider* provider : watcher->result()) {
    fetcher_->AddProvider(provider);
  }

  watcher->deleteLater();
  ultimate_reader_.reset();

  ReloadSettings();
}

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
}

void SongInfoView::ResultReady(int id, const SongInfoFetcher::Result& result) {}

void SongInfoView::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  // Put the providers in the right order
  QList<SongInfoProvider*> ordered_providers;

  QVariantList default_order;
  default_order << "lyrics.wikia.com"
                << "lyricstime.com"
                << "lyricsreg.com"
                << "lyricsmania.com"
                << "metrolyrics.com"
                << "azlyrics.com"
                << "songlyrics.com"
                << "elyrics.net"
                << "lyricsdownload.com"
                << "lyrics.com"
                << "lyricsbay.com"
                << "directlyrics.com"
                << "loudson.gs"
                << "teksty.org"
                << "tekstowo.pl (Polish translations)"
                << "vagalume.uol.com.br"
                << "vagalume.uol.com.br (Portuguese translations)"
                << "darklyrics.com";

  QVariant saved_order = s.value("search_order", default_order);
  for (const QVariant& name : saved_order.toList()) {
    SongInfoProvider* provider = ProviderByName(name.toString());
    if (provider) ordered_providers << provider;
  }

  // Enable all the providers in the list and rank them
  int relevance = 100;
  for (SongInfoProvider* provider : ordered_providers) {
    provider->set_enabled(true);
    qobject_cast<UltimateLyricsProvider*>(provider)->set_relevance(relevance--);
  }

  // Any lyric providers we don't have in ordered_providers are considered
  // disabled
  for (SongInfoProvider* provider : fetcher_->providers()) {
    if (qobject_cast<UltimateLyricsProvider*>(provider) &&
        !ordered_providers.contains(provider)) {
      provider->set_enabled(false);
    }
  }

  SongInfoBase::ReloadSettings();
}

SongInfoProvider* SongInfoView::ProviderByName(const QString& name) const {
  for (SongInfoProvider* provider : fetcher_->providers()) {
    if (UltimateLyricsProvider* lyrics =
            qobject_cast<UltimateLyricsProvider*>(provider)) {
      if (lyrics->name() == name) return provider;
    }
  }
  return nullptr;
}

namespace {
bool CompareLyricProviders(const UltimateLyricsProvider* a,
                           const UltimateLyricsProvider* b) {
  if (a->is_enabled() && !b->is_enabled()) return true;
  if (!a->is_enabled() && b->is_enabled()) return false;
  return a->relevance() > b->relevance();
}
}

QList<const UltimateLyricsProvider*> SongInfoView::lyric_providers() const {
  QList<const UltimateLyricsProvider*> ret;
  for (SongInfoProvider* provider : fetcher_->providers()) {
    if (UltimateLyricsProvider* lyrics =
            qobject_cast<UltimateLyricsProvider*>(provider)) {
      ret << lyrics;
    }
  }
  qSort(ret.begin(), ret.end(), CompareLyricProviders);
  return ret;
}
