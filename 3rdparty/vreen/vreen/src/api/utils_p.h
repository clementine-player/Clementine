/****************************************************************************
**
** Vreen - vk.com API Qt bindings
**
** Copyright © 2013 Aleksey Sidorov <gorthauer87@ya.ru>
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
#ifndef UTILS_P_H
#define UTILS_P_H
#include "utils.h"

namespace Vreen {

QString VK_SHARED_EXPORT join(IdList ids);
QString VK_SHARED_EXPORT toCamelCase(QString string);
QString VK_SHARED_EXPORT fromHtmlEntities(const QString &source);

} //namespace Vreen

#endif // UTILS_P_H
