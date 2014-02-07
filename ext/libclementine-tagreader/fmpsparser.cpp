/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include <functional>

#include <QStringList>
#include <QtDebug>

using std::placeholders::_1;
using std::placeholders::_2;

FMPSParser::FMPSParser()
    :  // The float regex ends with (?:$|(?=::|;;)) to ensure it matches all the
       // way
      // up to the end of the value.  Without it, it would match a string that
      // starts with a number, like "123abc".
      float_re_("\\s*([+-]?\\d+(?:\\.\\d+)?)\\s*(?:$|(?=::|;;))"),

      // Matches any character except unescaped slashes, colons and semicolons.
      string_re_("((?:[^\\\\;:]|(?:\\\\[\\\\:;]))+)(?:$|(?=::|;;))"),

      // Used for replacing escaped characters.
      escape_re_("\\\\([\\\\:;])") {}

// Parses a list of things (of type T) that are separated by two consecutive
// Separator characters.  Each individual thing is parsed by the F function.
// For example, to parse this data:
//    foo::bar::baz
// Use:
//    QVariantList ret;
//    ParseContainer<':'>(data, ParseValue, &ret);
// ret will then contain "foo", "bar", and "baz".
// Returns the number of characters that were consumed from data.
//
// You can parse lists of lists by using different separator characters:
//    ParseContainer<';'>(data, ParseContainer<':'>, &ret);
template <char Separator, typename F, typename T>
static int ParseContainer(const QStringRef& data, F f, QList<T>* ret) {
  ret->clear();

  T value;
  int pos = 0;
  while (pos < data.length()) {
    const int len = data.length() - pos;
    int matched_len =
        f(QStringRef(data.string(), data.position() + pos, len), &value);
    if (matched_len == -1 || matched_len > len) break;

    ret->append(value);
    pos += matched_len;

    // Expect two separators in a row
    if (pos + 2 <= data.length() && data.at(pos) == Separator &&
        data.at(pos + 1) == Separator) {
      pos += 2;
    } else {
      break;
    }
  }

  return pos;
}

bool FMPSParser::Parse(const QString& data) {
  result_ = Result();

  // Only return success if we matched the whole string
  return ParseListList(data, &result_) == data.length();
}

int FMPSParser::ParseValueRef(const QStringRef& data, QVariant* ret) const {
  // Try to match a float
  int pos = float_re_.indexIn(*data.string(), data.position());
  if (pos == data.position()) {
    *ret = float_re_.cap(1).toDouble();
    return float_re_.matchedLength();
  }

  // Otherwise try to match a string
  pos = string_re_.indexIn(*data.string(), data.position());
  if (pos == data.position()) {
    // Replace escape sequences with their actual characters
    QString value = string_re_.cap(1);
    value.replace(escape_re_, "\\1");
    *ret = value;
    return string_re_.matchedLength();
  }

  return -1;
}

// Parses an inner list - a list of values
int FMPSParser::ParseListRef(const QStringRef& data, QVariantList* ret) const {
  return ParseContainer<':'>(
      data, std::bind(&FMPSParser::ParseValueRef, this, _1, _2), ret);
}

// Parses an outer list - a list of lists
int FMPSParser::ParseListListRef(const QStringRef& data, Result* ret) const {
  return ParseContainer<';'>(
      data, std::bind(&FMPSParser::ParseListRef, this, _1, _2), ret);
}

// Convenience functions that take QStrings instead of QStringRefs.  Use the
// QStringRef versions if possible, they're faster.
int FMPSParser::ParseValue(const QString& data, QVariant* ret) const {
  return ParseValueRef(QStringRef(&data), ret);
}
int FMPSParser::ParseList(const QString& data, QVariantList* ret) const {
  return ParseListRef(QStringRef(&data), ret);
}
int FMPSParser::ParseListList(const QString& data, Result* ret) const {
  return ParseListListRef(QStringRef(&data), ret);
}
