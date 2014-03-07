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
#include "connection_p.h"

namespace Vreen {

Connection::Connection(QObject *parent) :
    QNetworkAccessManager(parent),
    d_ptr(new ConnectionPrivate(this))
{
}

Connection::Connection(ConnectionPrivate *data, QObject *parent) :
    QNetworkAccessManager(parent),
    d_ptr(data)
{
}

Connection::~Connection()
{
}

QNetworkReply *Connection::get(QNetworkRequest request)
{
    decorateRequest(request);
    return QNetworkAccessManager::get(request);
}

QNetworkReply *Connection::get(const QString &method, const QVariantMap &args)
{
	return QNetworkAccessManager::get(makeRequest(method, args));
}

QNetworkReply *Connection::put(const QString &method, QIODevice *data, const QVariantMap &args)
{
	return QNetworkAccessManager::put(makeRequest(method, args), data);
}

QNetworkReply *Connection::put(const QString &method, const QByteArray &data, const QVariantMap &args)
{
	 return QNetworkAccessManager::put(makeRequest(method, args), data);
}

/*!
 * \brief Connection::clear auth data. Default implementation doesn't nothing.
 */
void Connection::clear()
{
}

void Connection::setConnectionOption(Connection::ConnectionOption option, const QVariant &value)
{
    Q_D(Connection);
    d->options[option] = value;
}

QVariant Connection::connectionOption(Connection::ConnectionOption option) const
{
    return d_func()->options[option];
}

void Connection::decorateRequest(QNetworkRequest &)
{
}

} // namespace Vreen

