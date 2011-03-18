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

#ifndef MAGNATUNECONFIG_H
#define MAGNATUNECONFIG_H

#include <QWidget>

class QAuthenticator;
class QNetworkReply;

class NetworkAccessManager;
class Ui_MagnatuneConfig;

class MagnatuneConfig : public QWidget {
  Q_OBJECT
public:
  MagnatuneConfig(QWidget* parent = 0);
  ~MagnatuneConfig();

  bool NeedsValidation() const;
  void Validate();

public slots:
  void Load();
  void Save();

signals:
  void ValidationComplete(bool);

private slots:
  void MembershipChanged(int value);
  void ValidationFinished();
  void AuthenticationRequired(QNetworkReply* reply, QAuthenticator* auth);
  void CredentialsChanged();

private:
  bool credentials_changed_;
  NetworkAccessManager* network_;
  Ui_MagnatuneConfig* ui_;
};

#endif // MAGNATUNECONFIG_H
