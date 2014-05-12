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
#ifndef CONTENTDOWNLOADER_P_H
#define CONTENTDOWNLOADER_P_H
#include <QNetworkAccessManager>
#include <QDir>
#include <QUrl>
#include <QDesktopServices>
#include <QCryptographicHash>
#include "contentdownloader.h"

namespace Vreen {

class NetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    NetworkAccessManager(QObject *parent = 0) : QNetworkAccessManager(parent)
    {

    }

    QString fileHash(const QUrl &url) const
    {
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(url.toString().toUtf8());
        return hash.result().toHex();
    }
    QString cacheDir() const
    {
        auto dir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
        return dir + QLatin1String("/vk/");
    }
};

} //namespace Vreen

#endif // CONTENTDOWNLOADER_P_H
