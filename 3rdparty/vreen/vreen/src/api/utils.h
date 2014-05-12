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
#ifndef UTILS_H
#define UTILS_H
#include "vk_global.h"
#include <QStringList>

#include <QDebug>

namespace Vreen {

typedef QList<int> IdList;

template <typename T>
Q_INLINE_TEMPLATE T sender_cast(QObject *sender)
{
#ifndef QT_NO_DEBUG
    Q_ASSERT(qobject_cast<T>(sender));
#endif
    return static_cast<T>(sender);
}

template<typename T, int N>
Q_INLINE_TEMPLATE int strToEnum(const T &str, const char *(&strings)[N])
{
    for(int i=0; i < N; i++) {
        if(QLatin1String(strings[i]) == str)
            return i;
    }
    return -1;
}

template<typename X,typename T, int N>
Q_INLINE_TEMPLATE X strToEnum(const T &str, const char *(&strings)[N])
{
    return static_cast<X>(strToEnum(str,strings));
}

template<int N>
Q_INLINE_TEMPLATE QLatin1String enumToStr(int i, const char *(&strings)[N])
{
    return QLatin1String((i < 0 || i >= N) ? 0 : strings[i]);
}

template<int N>
Q_INLINE_TEMPLATE QStringList flagsToStrList(int i, const char *(&strings)[N])
{
    QStringList list;
    for (int pos = 0; pos < N; pos++) {
        int flag = 1 << pos;
        if ((flag) & i)
            list.append(strings[pos]);
    }
    return list;
}

template<typename Container, typename T, typename LessThan>
int lowerBound(Container container, const T &value, LessThan lessThan)
{
    auto it =  qLowerBound(container.constBegin(), container.constEnd(), value, lessThan);
    int index = it - container.constBegin();
    return index;
}

template<int N>
Q_INLINE_TEMPLATE size_t strCount(const char *(&)[N])
{
    return N;
}

template <typename Item, typename Value, typename Method>
struct ComparatorBase
{
    ComparatorBase(Method method, Qt::SortOrder order = Qt::AscendingOrder) :
        method(method),
        sortOrder(order)
    {

    }
    inline bool operator()(const Item &a, const Item &b) const
    {
        return operator()(method(a), method(b));
    }
    inline bool operator()(const Item &a, int id) const
    {
        return operator()(method(a), id);
    }
    inline bool operator()(Value id, const Item &b) const
    {
        return operator()(id, method(b));
    }
    inline bool operator()(Value a, Value b) const
    {
        return sortOrder == Qt::AscendingOrder ? a < b
                                               : b < a;
    }

    const Method method;
    Qt::SortOrder sortOrder;
};

template <typename Item, typename Value>
struct Comparator : public ComparatorBase<Item, Value, Value (*)(const Item &)>
{
    typedef Value (*Method)(const Item &);
    Comparator(Method method, Qt::SortOrder order = Qt::AscendingOrder) :
        ComparatorBase<Item, Value, Value (*)(const Item &)>(method, order)
    {

    }
};

template <typename Container>
struct IdComparator : public Comparator<Container, int>
{
    IdComparator(Qt::SortOrder order = Qt::AscendingOrder) :
        Comparator<Container, int>(id_, order)
    {

    }
private:
    inline static int id_(const Container &a) { return a.id(); }
};


} //namespace Vreen

#endif // UTILS_H

