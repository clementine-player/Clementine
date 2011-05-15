/****************************************************************************
**
** Copyright (C) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Script Generator project on Qt Labs.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/


#ifndef SYMBOL_H
#define SYMBOL_H

#include <QtCore/QString>
#include <cstring>

#include <QtCore/QHash>
#include <QtCore/QPair>

struct NameSymbol
{
  const char *data;
  std::size_t count;

  inline QString as_string() const
  {
    return QString::fromUtf8(data, (int) count);
  }

  inline bool operator == (const NameSymbol &other) const
  {
    return count == other.count
      && std::strncmp(data, other.data, count) == 0;
  }

protected:
  inline NameSymbol() {}
  inline NameSymbol(const char *d, std::size_t c)
    : data(d), count(c) {}

private:
  void operator = (const NameSymbol &);

  friend class NameTable;
};

inline uint qHash(const NameSymbol &r)
{
  uint hash_value = 0;

  for (std::size_t i=0; i<r.count; ++i)
    hash_value = (hash_value << 5) - hash_value + r.data[i];

  return hash_value;
}

inline uint qHash(const QPair<const char*, std::size_t> &r)
{
  uint hash_value = 0;

  for (std::size_t i=0; i<r.second; ++i)
    hash_value = (hash_value << 5) - hash_value + r.first[i];

  return hash_value;
}

class NameTable
{
public:
  typedef QPair<const char *, std::size_t> KeyType;
  typedef QHash<KeyType, NameSymbol*> ContainerType;

public:
  NameTable() {}

  ~NameTable()
  {
    qDeleteAll(_M_storage);
  }

  inline const NameSymbol *findOrInsert(const char *str, std::size_t len)
  {
    KeyType key(str, len);

    NameSymbol *name = _M_storage.value(key);
    if (!name)
      {
	name = new NameSymbol(str, len);
	_M_storage.insert(key, name);
      }

    return name;
  }

  inline std::size_t count() const
  {
    return _M_storage.size();
  }

private:
  ContainerType _M_storage;

private:
  NameTable(const NameTable &other);
  void operator = (const NameTable &other);
};

#endif // SYMBOL_H

// kate: space-indent on; indent-width 2; replace-tabs on;
