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

#ifndef SMARTPLAYLISTSEARCHTERM_H
#define SMARTPLAYLISTSEARCHTERM_H

#include <QDataStream>
#include <QVariant>

namespace smart_playlists {

class SearchTerm {
 public:
  // These values are persisted, so add to the end of the enum only
  enum Field {
    Field_Title = 0,
    Field_Artist,
    Field_Album,
    Field_AlbumArtist,
    Field_Composer,
    Field_Length,
    Field_Track,
    Field_Disc,
    Field_Year,
    Field_Genre,
    Field_BPM,
    Field_Bitrate,
    Field_Samplerate,
    Field_Filesize,
    Field_DateCreated,
    Field_DateModified,
    Field_Rating,
    Field_Score,
    Field_PlayCount,
    Field_SkipCount,
    Field_LastPlayed,
    Field_Comment,
    Field_Filepath,
    Field_Performer,
    Field_Grouping,
    Field_OriginalYear,
    FieldCount
  };

  // These values are persisted, so add to the end of the enum only
  enum Operator {
    // For text
    Op_Contains = 0,
    Op_NotContains = 1,
    Op_StartsWith = 2,
    Op_EndsWith = 3,

    // For numbers
    Op_GreaterThan = 4,
    Op_LessThan = 5,

    // For everything
    Op_Equals = 6,
    Op_NotEquals = 9,

    // For numeric dates (e.g. in the last X days)
    Op_NumericDate = 7,
    // For relative dates
    Op_RelativeDate = 8,

    // For numeric dates (e.g. not in the last X days)
    Op_NumericDateNot = 10,

    Op_Empty = 11,
    Op_NotEmpty = 12,

    // Next value = 13
  };

  enum Type {
    Type_Text,
    Type_Date,
    Type_Time,
    Type_Number,
    Type_Rating,
    Type_Invalid
  };

  // These values are persisted, so add to the end of the enum only
  enum DateType { Date_Hour = 0, Date_Day, Date_Week, Date_Month, Date_Year, };

  SearchTerm();
  SearchTerm(Field field, Operator op, const QVariant& value);

  Field field_;
  Operator operator_;
  QVariant value_;
  DateType date_;
  // For relative dates, we need a second parameter, might be useful somewhere
  // else
  QVariant second_value_;

  QString ToSql() const;
  bool is_valid() const;
  bool operator==(const SearchTerm& other) const;
  bool operator!=(const SearchTerm& other) const { return !(*this == other); }

  static Type TypeOf(Field field);
  static QList<Operator> OperatorsForType(Type type);
  static QString OperatorText(Type type, Operator op);
  static QString FieldName(Field field);
  static QString FieldColumnName(Field field);
  static QString FieldSortOrderText(Type type, bool ascending);
  static QString DateName(DateType date, bool forQuery);
};

typedef QList<SearchTerm::Operator> OperatorList;

}  // namespace

QDataStream& operator<<(QDataStream& s,
                        const smart_playlists::SearchTerm& term);
QDataStream& operator>>(QDataStream& s, smart_playlists::SearchTerm& term);

#endif  // SMARTPLAYLISTSEARCHTERM_H
