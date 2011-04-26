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

#ifndef SPOTIFYCONFIG_H
#define SPOTIFYCONFIG_H

#include <QWidget>

class QAuthenticator;
class QNetworkReply;

class NetworkAccessManager;
class Ui_SpotifyConfig;
class SpotifyService;

class SpotifyConfig : public QWidget {
  Q_OBJECT

public:
  SpotifyConfig(QWidget* parent = 0);
  ~SpotifyConfig();

  bool NeedsValidation() const;
  void Validate();

signals:
  void ValidationComplete(bool success);

public slots:
  void Load();
  void Save();

private slots:
  void LoginFinished(bool success);

private:
  NetworkAccessManager* network_;
  Ui_SpotifyConfig* ui_;
  SpotifyService* service_;

  bool needs_validation_;
};

#endif // SPOTIFYCONFIG_H
