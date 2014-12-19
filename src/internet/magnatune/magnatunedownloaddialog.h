/* This file is part of Clementine.
   Copyright 2010, David Sansome <davidsansome@gmail.com>
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

#ifndef INTERNET_MAGNATUNE_MAGNATUNEDOWNLOADDIALOG_H_
#define INTERNET_MAGNATUNE_MAGNATUNEDOWNLOADDIALOG_H_

#include <memory>

#include <QDialog>
#include <QNetworkReply>
#include <QStringList>

#include "core/song.h"

class MagnatuneService;
class Ui_MagnatuneDownloadDialog;

class QFile;
class QXmlStreamReader;

class MagnatuneDownloadDialog : public QDialog {
  Q_OBJECT

 public:
  explicit MagnatuneDownloadDialog(MagnatuneService* service, QWidget* parent = nullptr);
  ~MagnatuneDownloadDialog();

  void Show(const SongList& songs);

 signals:
  void Finished(const QStringList& albums);

 public slots:
  void accept();

 protected:
  void closeEvent(QCloseEvent* e);

 private slots:
  void Browse();

  void DownloadNext();
  void Error(QNetworkReply::NetworkError);
  void MetadataFinished();
  void DownloadProgress(qint64 received, qint64 total);
  void DownloadReadyRead();
  void DownloadFinished();

  void ShowError(const QString& message);
  void AllFinished(bool error);

 private:
  QString GetOutputFilename();

 private:
  Ui_MagnatuneDownloadDialog* ui_;
  MagnatuneService* service_;

  QNetworkAccessManager* network_;
  QNetworkReply* current_reply_;
  std::unique_ptr<QFile> download_file_;

  int next_row_;
};

#endif  // INTERNET_MAGNATUNE_MAGNATUNEDOWNLOADDIALOG_H_
