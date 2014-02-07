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

#ifndef SPOTIFYBLOBDOWNLOADER_H
#define SPOTIFYBLOBDOWNLOADER_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class QProgressDialog;

class SpotifyBlobDownloader : public QObject {
  Q_OBJECT

 public:
  SpotifyBlobDownloader(const QString& version, const QString& path,
                        QObject* parent = 0);
  ~SpotifyBlobDownloader();

  static const char* kSignatureSuffix;

  static bool Prompt();

  void Start();

signals:
  void Finished();

 private slots:
  void ReplyFinished();
  void ReplyProgress();
  void Cancel();

 private:
  void ShowError(const QString& message);
  void EmitFinished();

 private:
  QString version_;
  QString path_;

  QNetworkAccessManager* network_;
  QList<QNetworkReply*> replies_;

  QProgressDialog* progress_;
};

#endif  // SPOTIFYBLOBDOWNLOADER_H
