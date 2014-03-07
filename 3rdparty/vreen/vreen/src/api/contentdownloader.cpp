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
#include "contentdownloader_p.h"
#include "utils.h"
#include <QPointer>
#include <QThread>
#include <QFileInfo>
#include <QNetworkReply>
#include <QTimer>

namespace Vreen {

static QPointer<NetworkAccessManager> networkManager;

ContentDownloader::ContentDownloader(QObject *parent) :
    QObject(parent)
{
    if (!networkManager) {
        networkManager = new NetworkAccessManager;
        //use another thread for more smooth gui
        //auto thread = new QThread;
        //networkManager->moveToThread(thread);
        //connect(networkManager.data(), SIGNAL(destroyed()), thread, SLOT(quit()));
        //connect(thread, SIGNAL(finished()), SLOT(deleteLater()));
        //thread->start(QThread::LowPriority);
    }
}

QString ContentDownloader::download(const QUrl &link)
{
    QString path = networkManager->cacheDir()
            + networkManager->fileHash(link)
            + QLatin1String(".")
            + QFileInfo(link.path()).completeSuffix();

    if (QFileInfo(path).exists()) {
        //FIXME it maybe not work in some cases (use event instead emit)
        emit downloadFinished(path);
    } else {
        QNetworkRequest request(link);
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        auto reply = networkManager->get(request);
        reply->setProperty("path", path);
        connect(reply, SIGNAL(finished()), this, SLOT(replyDone()));
    }
    return path;
}

void ContentDownloader::replyDone()
{
    auto reply = sender_cast<QNetworkReply*>(sender());
    QString cacheDir = networkManager->cacheDir();
    QDir dir(cacheDir);
    if (!dir.exists()) {
        if(!dir.mkpath(cacheDir)) {
            qWarning("Unable to create cache dir");
            return;
        }
    }
    //TODO move method to manager in other thread
    QString path = reply->property("path").toString();
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Unable to write file!");
        return;
    }
    file.write(reply->readAll());
    file.close();

    emit downloadFinished(path);
}

} // namespace Vreen

#include "moc_contentdownloader.cpp"

