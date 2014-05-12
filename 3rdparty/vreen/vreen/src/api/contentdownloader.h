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
#ifndef VK_CONTENTDOWNLOADER_H
#define VK_CONTENTDOWNLOADER_H

#include <QObject>
#include "vk_global.h"

class QUrl;

namespace Vreen {

class VK_SHARED_EXPORT ContentDownloader : public QObject
{
    Q_OBJECT
public:
    explicit ContentDownloader(QObject *parent = 0);
    Q_INVOKABLE QString download(const QUrl &link);
signals:
    void downloadFinished(const QString &fileName);
private slots:
    void replyDone();
};

} // namespace Vreen

#endif // VK_CONTENTDOWNLOADER_H

