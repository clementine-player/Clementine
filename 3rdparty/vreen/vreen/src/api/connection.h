/****************************************************************************
**
** Vreen - vk.com API Qt bindings
**
** Copyright © 2012 Aleksey Sidorov <gorthauer87@ya.ru>
**
*****************************************************************************
**
** $VREEN_BEGIN_LICENSE$
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licenses/.
** $VREEN_END_LICENSE$
**
****************************************************************************/
#ifndef VK_CONNECTION_H
#define VK_CONNECTION_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QVariantMap>
#include "client.h"

namespace Vreen {

class Reply;
class ConnectionPrivate;

class VK_SHARED_EXPORT Connection : public QNetworkAccessManager
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Connection)
    Q_ENUMS(ConnectionOption)
public:
    Connection(QObject *parent = 0);
    Connection(ConnectionPrivate *data, QObject *parent = 0);
    ~Connection();

    enum ConnectionOption {
        ShowAuthDialog,
        KeepAuthData
    };

    virtual void connectToHost(const QString &login, const QString &password) = 0;
    virtual void disconnectFromHost() = 0;

    QNetworkReply *get(QNetworkRequest request);
    QNetworkReply *get(const QString &method, const QVariantMap &args = QVariantMap());
    QNetworkReply *put(const QString &method, QIODevice *data, const QVariantMap &args = QVariantMap());
    QNetworkReply *put(const QString &method, const QByteArray &data, const QVariantMap &args = QVariantMap());

    virtual Client::State connectionState() const = 0;
    virtual int uid() const = 0;
    virtual void clear();

    Q_INVOKABLE void setConnectionOption(ConnectionOption option, const QVariant &value);
    Q_INVOKABLE QVariant connectionOption(ConnectionOption option) const;
signals:
    void connectionStateChanged(Vreen::Client::State connectionState);
    void error(Vreen::Client::Error);
protected:
	virtual QNetworkRequest makeRequest(const QString &method, const QVariantMap &args = QVariantMap()) = 0;
    virtual void decorateRequest(QNetworkRequest &);
    QScopedPointer<ConnectionPrivate> d_ptr;
};

} //namespace Vreen

Q_DECLARE_METATYPE(Vreen::Connection*)

#endif // VK_CONNECTION_H

