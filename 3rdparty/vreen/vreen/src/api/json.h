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
#ifndef VK_JSON_H
#define VK_JSON_H
#include "vk_global.h"
#include <QVariantMap>

namespace Vreen {

namespace JSON {
    VK_SHARED_EXPORT QVariant parse(const QByteArray &data);
    VK_SHARED_EXPORT QByteArray generate(const QVariant &data, int indent = 0);
} //namespace JSON

} // namespace Vreen

#endif // VK_JSON_H

