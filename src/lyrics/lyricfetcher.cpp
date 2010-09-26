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

#include "lyricfetcher.h"
#include "ultimatelyricsreader.h"

#include <QFutureWatcher>
#include <QSettings>
#include <QtConcurrentRun>
#include <QtDebug>

typedef QList<LyricProvider*> ProviderList;

const char* LyricFetcher::kSettingsGroup = "Lyrics";

LyricFetcher::LyricFetcher(NetworkAccessManager* network, QObject* parent)
  : QObject(parent),
    network_(network),
    next_id_(1),
    ultimate_reader_(new UltimateLyricsReader(network))
{
  // Parse the ultimate lyrics xml file in the background
  QFuture<ProviderList> future = QtConcurrent::run(
      ultimate_reader_.get(), &UltimateLyricsReader::Parse,
      QString(":lyrics/ultimate_providers.xml"));
  QFutureWatcher<ProviderList>* watcher = new QFutureWatcher<ProviderList>(this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(UltimateLyricsParsed()));
}

LyricFetcher::~LyricFetcher() {
  qDeleteAll(providers_);
}

void LyricFetcher::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  // Put the providers in the right order
  QList<LyricProvider*> ordered_providers;

  QVariant saved_order = s.value("search_order");
  if (saved_order.isNull()) {
    // Hardcoded default order
    ordered_providers << ProviderByName("lyrics.wikia.com")
                      << ProviderByName("lyricsplugin.com")
                      << ProviderByName("lyricstime.com")
                      << ProviderByName("lyricsreg.com")
                      << ProviderByName("lyricsmania.com")
                      << ProviderByName("metrolyrics.com")
                      << ProviderByName("seeklyrics.com")
                      << ProviderByName("azlyrics.com")
                      << ProviderByName("mp3lyrics.org")
                      << ProviderByName("songlyrics.com")
                      << ProviderByName("lyricsmode.com")
                      << ProviderByName("elyrics.net")
                      << ProviderByName("lyricsdownload.com")
                      << ProviderByName("lyrics.com")
                      << ProviderByName("lyricsbay.com")
                      << ProviderByName("directlyrics.com")
                      << ProviderByName("loudson.gs")
                      << ProviderByName("teksty.org")
                      << ProviderByName("tekstowo.pl (Polish translations)")
                      << ProviderByName("vagalume.uol.com.br")
                      << ProviderByName("vagalume.uol.com.br (Portuguese translations)");
  } else {
    foreach (const QVariant& name, saved_order.toList()) {
      LyricProvider* provider = ProviderByName(name.toString());
      if (provider)
        ordered_providers << provider;
    }
  }

  foreach (LyricProvider* provider, ordered_providers)
    provider->set_enabled(true);

  // Any providers we don't have in ordered_providers are considered disabled
  QList<LyricProvider*> disabled_providers;
  foreach (LyricProvider* provider, providers_) {
    if (ordered_providers.contains(provider))
      continue;
    provider->set_enabled(false);
    disabled_providers << provider;
  }

  providers_ = ordered_providers;
  providers_.append(disabled_providers);
}

LyricProvider* LyricFetcher::ProviderByName(const QString& name) const {
  foreach (LyricProvider* provider, providers_) {
    if (provider->name() == name)
      return provider;
  }
  qWarning() << __PRETTY_FUNCTION__ << "provider not found" << name;
  return NULL;
}

void LyricFetcher::UltimateLyricsParsed() {
  QFutureWatcher<ProviderList>* watcher =
      static_cast<QFutureWatcher<ProviderList>*>(sender());

  providers_.append(watcher->future().results()[0]);

  watcher->deleteLater();
  ultimate_reader_.reset();

  ReloadSettings();
}

int LyricFetcher::SearchAsync(const Song& metadata) {
  const int id = next_id_ ++;

  QtConcurrent::run(this, &LyricFetcher::DoSearch, metadata, id);

  return id;
}

void LyricFetcher::DoSearch(const Song& metadata, int id) {
  foreach (LyricProvider* provider, providers_) {
    emit SearchProgress(id, provider->name());

    LyricProvider::Result result = provider->Search(metadata);
    if (result.valid) {
      emit SearchResult(id, true, result.title, result.content);
      return;
    }
  }

  emit SearchResult(id, false, QString(), QString());
}

