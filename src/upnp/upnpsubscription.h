/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#ifndef UPNP_UPNPSUBSCRIPTION_H_
#define UPNP_UPNPSUBSCRIPTION_H_

#include <QByteArray>
#include <QHostAddress>
#include <QMap>
#include <QObject>
#include <QUrl>
#include <memory>

class EventCallbackTable;
class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

// A subscription to one UPnP service's events (GENA). The renderer sends
// them to the network remote's media server, at a URL with a token of this
// subscription's that only the renderer's address may use. It's renewed
// before it runs out, and cancelled when this is deleted.
class UpnpSubscription : public QObject {
  Q_OBJECT

 public:
  // How long subscriptions are asked for. Renderers may give less.
  static const int kTimeoutSec;

  UpnpSubscription(QNetworkAccessManager* network,
                   std::shared_ptr<EventCallbackTable> callbacks,
                   const QUrl& event_url, const QHostAddress& renderer,
                   QObject* parent = nullptr);
  ~UpnpSubscription();

  // Subscribes, with events sent to the media server at |base|, the URL
  // the renderer fetches media from.
  void Start(const QUrl& base);

  const QUrl& event_url() const { return event_url_; }
  const QUrl& base() const { return base_; }
  // Whether the renderer has said it'll send events.
  bool is_subscribed() const { return !sid_.isEmpty(); }

  // A NOTIFY body's variables. A LastChange among them is replaced by the
  // variables it lists; see ParseLastChange.
  static QMap<QString, QString> ParsePropertySet(const QByteArray& body);
  // LastChange's variables for InstanceID 0, such as "TransportState". Of
  // those per channel, such as Volume, only the Master channel's.
  static QMap<QString, QString> ParseLastChange(const QString& xml);

 signals:
  void Subscribed();
  // What changed. The first event has every variable.
  void Event(const QMap<QString, QString>& variables);
  // It couldn't subscribe, or stopped being able to renew.
  void Failed(const QString& reason);

 private slots:
  // From EventCallbackTable.
  void EventReceived(const QByteArray& body);
  void Renew();

 private:
  void Subscribe(bool renewing);
  void SubscribeFinished(QNetworkReply* reply, bool renewing);
  void Unsubscribe();

  QNetworkAccessManager* network_;
  std::shared_ptr<EventCallbackTable> callbacks_;
  const QUrl event_url_;
  const QHostAddress renderer_;
  const QByteArray token_;
  QUrl base_;
  QTimer* renew_timer_;

  QByteArray sid_;
};

#endif  // UPNP_UPNPSUBSCRIPTION_H_
