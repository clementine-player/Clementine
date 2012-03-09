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

#ifndef GPODDERSYNC_H
#define GPODDERSYNC_H

#include <QObject>
#include <QScopedPointer>

class Application;

class QNetworkAccessManager;
class QNetworkReply;

namespace mygpo {
  class ApiRequest;
}

class GPodderSync : public QObject {
  Q_OBJECT

public:
  GPodderSync(Application* app, QObject* parent = 0);
  ~GPodderSync();

  static const char* kSettingsGroup;

  static QString DefaultDeviceName();
  static QString DeviceId();

  bool is_logged_in() const;

  // Tries to login using the given username and password.  Also sets the
  // device name and type on gpodder.net.  You do NOT need to deleteLater()
  // the QNetworkReply returned from this function.
  // If login succeeds the username and password will be saved in QSettings.
  QNetworkReply* Login(const QString& username, const QString& password,
                       const QString& device_name);

  // Clears any saved username and password from QSettings.
  void Logout();

private slots:
  void ReloadSettings();
  void LoginFinished(QNetworkReply* reply,
                     const QString& username, const QString& password);

private:
  Application* app_;
  QNetworkAccessManager* network_;
  QScopedPointer<mygpo::ApiRequest> api_;

  QString username_;
  QString password_;
};

#endif // GPODDERSYNC_H
