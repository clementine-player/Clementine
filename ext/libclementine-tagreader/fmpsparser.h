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

#ifndef FMPSPARSER_H
#define FMPSPARSER_H

#include <QRegExp>
#include <QVariantList>

class FMPSParser {
 public:
  FMPSParser();

  // A FMPS result is a list of lists of values (where a value is a string or
  // a float).
  typedef QList<QVariantList> Result;

  // Parses a FMPS value and returns true on success.
  bool Parse(const QString& data);

  // Gets the result of the last successful Parse.
  Result result() const { return result_; }

  // Returns true if result() is empty.
  bool is_empty() const { return result().isEmpty() || result()[0].isEmpty(); }

  // Internal functions, public for unit tests
  int ParseValue(const QString& data, QVariant* ret) const;
  int ParseValueRef(const QStringRef& data, QVariant* ret) const;

  int ParseList(const QString& data, QVariantList* ret) const;
  int ParseListRef(const QStringRef& data, QVariantList* ret) const;

  int ParseListList(const QString& data, Result* ret) const;
  int ParseListListRef(const QStringRef& data, Result* ret) const;

 private:
  QRegExp float_re_;
  QRegExp string_re_;
  QRegExp escape_re_;
  Result result_;
};

#endif  // FMPSPARSER_H
