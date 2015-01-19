/* This file is part of Clementine.
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2011, David Sansome <me@davidsansome.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#ifndef COVERS_COVERPROVIDERS_H_
#define COVERS_COVERPROVIDERS_H_

#include <QMap>
#include <QMutex>
#include <QObject>

class AlbumCoverFetcherSearch;
class CoverProvider;

// This is a repository for cover providers.
// Providers are automatically unregistered from the repository when they are
// deleted.  The class is thread safe.
class CoverProviders : public QObject {
  Q_OBJECT

 public:
  explicit CoverProviders(QObject* parent = nullptr);

  // Lets a cover provider register itself in the repository.
  void AddProvider(CoverProvider* provider);
  void RemoveProvider(CoverProvider* provider);

  // Returns a list of cover providers
  QList<CoverProvider*> List() const { return cover_providers_.keys(); }

  // Returns true if this repository has at least one registered provider.
  bool HasAnyProviders() const { return !cover_providers_.isEmpty(); }

  int NextId();

 private slots:
  void ProviderDestroyed();

 private:
  Q_DISABLE_COPY(CoverProviders);

  QMap<CoverProvider*, QString> cover_providers_;
  QMutex mutex_;

  QAtomicInt next_id_;
};

#endif  // COVERS_COVERPROVIDERS_H_
