/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#ifndef CRASHSENDER_H
#define CRASHSENDER_H

#include <QList>
#include <QObject>
#include <QPair>

class QFile;
class QNetworkAccessManager;
class QProgressDialog;


// Asks the user if he wants to send a crash report, and displays a progress
// dialog while uploading it if he does.
class CrashSender : public QObject {
  Q_OBJECT

public:
  CrashSender(const QString& minidump_filename, const QString& log_filename);

  // Returns false if the user doesn't want to send the crash report (caller
  // should exit), or true if he does (caller should start the Qt event loop).
  bool Start();

private:
  typedef QPair<QString, QString> ClientInfoPair;

private slots:
  void RedirectFinished();
  void UploadProgress(qint64 bytes, qint64 total);
  void UploadFinished();

  QList<ClientInfoPair> ClientInfo() const;

private:
  static const char* kUploadURL;

  QNetworkAccessManager* network_;

  QString minidump_filename_;
  QString log_filename_;
  QFile* minidump_;
  QFile* log_;
  QProgressDialog* progress_;
};

#endif // CRASHSENDER_H
