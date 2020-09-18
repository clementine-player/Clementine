/* This file is part of Clementine.
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

#ifndef INTERNET_SPOTIFY_SPOTIFYBLOBDOWNLOADER_H_
#define INTERNET_SPOTIFY_SPOTIFYBLOBDOWNLOADER_H_

#include <QMap>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class QProgressDialog;

class SpotifyBlobDownloader : public QObject {
  Q_OBJECT

 public:
  SpotifyBlobDownloader(const QString& version, const QString& path,
                        QObject* parent = nullptr);
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

  bool CheckSignature(const QMap<QString, QByteArray>& file_data,
                      const QStringList& signature_filenames);
  static QByteArray ConvertPEMToDER(const QByteArray& pem);

 private:
  QString version_;
  QString path_;

  QNetworkAccessManager* network_;
  QList<QNetworkReply*> replies_;

  QProgressDialog* progress_;
};

#endif  // INTERNET_SPOTIFY_SPOTIFYBLOBDOWNLOADER_H_
