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
#include "utils.h"
#include <QStringBuilder>
#include <QTextDocument>
#include <QUrl>

//#define MAX_ENTITY 258
//extern const struct QTextHtmlEntity { const char *name; quint16 code; } entities[MAX_ENTITY];

namespace Vreen {

QString join(IdList ids)
{
    QString result;
    if (ids.isEmpty())
        return result;

    result = QString::number(ids.takeFirst());
    foreach (auto id, ids)
        result += QLatin1Literal(",") % QString::number(id);
    return result;
}

QString toCamelCase(QString string)
{
    int from = 0;
    while ((from = string.indexOf("_", from)) != -1) {
        auto index = from + 1;
        string.remove(from, 1);
        auto letter = string.at(index);
        string.replace(index, 1, letter.toUpper());
    }
    return string;
}

QString fromHtmlEntities(const QString &source)
{
    //Simple hack from slashdot
    QTextDocument text;
    text.setHtml(source);
    return text.toPlainText();
}

QString toHtmlEntities(const QString &source)
{
    return Qt::escape(source);
}

} //namespace Vreen

