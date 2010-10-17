/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "fmpsparser.h"

#include <QStringList>
#include <QtDebug>

#include <boost/bind.hpp>

FMPSParser::FMPSParser()
  : float_re_("\\s*([+-]?\\d+(?:\\.\\d+)?)\\s*(?:$|(?=::|;;))"),
    string_re_("((?:[^\\\\;:]|(?:\\\\[\\\\:;]))+)(?:$|(?=::|;;))"),
    escape_re_("\\\\([\\\\:;])")
{
}

template <char Separator, typename F, typename T>
static int ParseContainer(const QStringRef& data, F f, QList<T>* ret) {
  ret->clear();

  T value;
  int pos = 0;
  while (pos < data.length()) {
    const int len = data.length() - pos;
    int matched_len = f(QStringRef(data.string(), data.position() + pos, len), &value);
    if (matched_len == -1 || matched_len > len)
      break;

    ret->append(value);
    pos += matched_len;

    if (pos + 2 <= data.length() && data.at(pos)   == Separator
                                 && data.at(pos+1) == Separator) {
      pos += 2;
    } else {
      break;
    }
  }

  return pos;
}

bool FMPSParser::Parse(const QString& data) {
  result_ = Result();
  return ParseListList(data, &result_) == data.length();
}

int FMPSParser::ParseValue(const QString& data, QVariant* ret) const {
  return ParseValueRef(QStringRef(&data), ret);
}

int FMPSParser::ParseValueRef(const QStringRef& data, QVariant* ret) const {
  int pos = float_re_.indexIn(*data.string(), data.position());
  if (pos == data.position()) {
    *ret = float_re_.cap(1).toDouble();
    return float_re_.matchedLength();
  }

  pos = string_re_.indexIn(*data.string(), data.position());
  if (pos == data.position()) {
    QString value = string_re_.cap(1);
    value.replace(escape_re_, "\\1");
    *ret = value;
    return string_re_.matchedLength();
  }

  return -1;
}

int FMPSParser::ParseList(const QString& data, QVariantList* ret) const {
  return ParseListRef(QStringRef(&data), ret);
}

int FMPSParser::ParseListRef(const QStringRef& data, QVariantList* ret) const {
  return ParseContainer<':'>(data, boost::bind(&FMPSParser::ParseValueRef, this, _1, _2), ret);
}

int FMPSParser::ParseListList(const QString& data, Result* ret) const {
  return ParseListListRef(QStringRef(&data), ret);
}

int FMPSParser::ParseListListRef(const QStringRef& data, Result* ret) const {
  return ParseContainer<';'>(data, boost::bind(&FMPSParser::ParseListRef, this, _1, _2), ret);
}
