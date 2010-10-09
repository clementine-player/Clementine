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

#include "artistinfofetcher.h"
#include "echonestartistinfo.h"

ArtistInfoFetcher::ArtistInfoFetcher(QObject* parent)
  : QObject(parent),
    next_id_(1)
{
  AddProvider(new EchoNestArtistInfo(this));
}

void ArtistInfoFetcher::AddProvider(ArtistInfoProvider* provider) {
  providers_ << provider;
  connect(provider, SIGNAL(ImageReady(int,QUrl)), SIGNAL(ImageReady(int,QUrl)));
  connect(provider, SIGNAL(InfoReady(int,CollapsibleInfoPane::Data)), SIGNAL(InfoReady(int,CollapsibleInfoPane::Data)));
}

int ArtistInfoFetcher::FetchInfo(const QString& artist) {
  const int id = next_id_ ++;
  foreach (ArtistInfoProvider* provider, providers_) {
    provider->FetchInfo(id, artist);
  }
  return id;
}
