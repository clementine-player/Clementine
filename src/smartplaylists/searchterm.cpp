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

#include "searchterm.h"
#include "playlist/playlist.h"

namespace smart_playlists {

SearchTerm::SearchTerm()
  : field_(Field_Title),
    operator_(Op_Equals)
{
}

SearchTerm::SearchTerm(
    Field field, Operator op, const QVariant& value)
  : field_(field),
    operator_(op),
    value_(value)
{
}

QString SearchTerm::ToSql() const {
  const QString col = FieldColumnName(field_);
  QString value = value_.toString();
  value.replace('\'', "''");

  switch (operator_) {
    case Op_Contains:
      return col + " LIKE '%" + value + "%'";
    case Op_NotContains:
      return col + " NOT LIKE '%" + value + "%'";
    case Op_StartsWith:
      return col + " LIKE '" + value + "%'";
    case Op_EndsWith:
      return col + " LIKE '%" + value + "'";
    case Op_Equals:
      if (TypeOf(field_) == Type_Text)
        return col + " LIKE '" + value + "'";
      return col + " = '" + value + "'";
    case Op_GreaterThan:
      return col + " > '" + value + "'";
    case Op_LessThan:
      return col + " < '" + value + "'";
  }

  return QString();
}

bool SearchTerm::is_valid() const {
  switch (TypeOf(field_)) {
    case Type_Text:   return !value_.toString().isEmpty();
    case Type_Date:   return value_.toInt() != 0;
    case Type_Number: return value_.toInt() >= 0;
    case Type_Rating: return value_.toFloat() >= 0.0;
    case Type_Time:   return true;
  }
  return false;
}

bool SearchTerm::operator ==(const SearchTerm& other) const {
  return field_ == other.field_ &&
         operator_ == other.operator_ &&
         value_ == other.value_;
}

SearchTerm::Type SearchTerm::TypeOf(Field field) {
  switch (field) {
    case Field_Length:
      return Type_Time;

    case Field_Track:
    case Field_Disc:
    case Field_Year:
    case Field_BPM:
    case Field_Bitrate:
    case Field_Samplerate:
    case Field_Filesize:
    case Field_PlayCount:
    case Field_SkipCount:
    case Field_Score:
      return Type_Number;

    case Field_LastPlayed:
    case Field_DateCreated:
    case Field_DateModified:
      return Type_Date;

    case Field_Rating:
      return Type_Rating;

    default:
      return Type_Text;
  }
}

OperatorList SearchTerm::OperatorsForType(Type type) {
  switch (type) {
    case Type_Text:
      return OperatorList() << Op_Contains << Op_NotContains << Op_Equals
                            << Op_StartsWith << Op_EndsWith;
    default:
      return OperatorList() << Op_Equals << Op_GreaterThan << Op_LessThan;
  }
}

QString SearchTerm::OperatorText(Type type, Operator op) {
  if (type == Type_Date) {
    switch (op) {
      case Op_GreaterThan: return QObject::tr("after");
      case Op_LessThan:    return QObject::tr("before");
      case Op_Equals:      return QObject::tr("on");
      default:             return QString();
    }
  }

  switch (op) {
    case Op_Contains:    return QObject::tr("contains");
    case Op_NotContains: return QObject::tr("does not contain");
    case Op_StartsWith:  return QObject::tr("starts with");
    case Op_EndsWith:    return QObject::tr("ends with");
    case Op_GreaterThan: return QObject::tr("greater than");
    case Op_LessThan:    return QObject::tr("less than");
    case Op_Equals:      return QObject::tr("equals");
  }

  return QString();
}

QString SearchTerm::FieldColumnName(Field field) {
  switch (field) {
    case Field_Length:      return "length";
    case Field_Track:       return "track";
    case Field_Disc:        return "disc";
    case Field_Year:        return "year";
    case Field_BPM:         return "bpm";
    case Field_Bitrate:     return "bitrate";
    case Field_Samplerate:  return "samplerate";
    case Field_Filesize:    return "filesize";
    case Field_PlayCount:   return "playcount";
    case Field_SkipCount:   return "skipcount";
    case Field_LastPlayed:  return "lastplayed";
    case Field_DateCreated: return "ctime";
    case Field_DateModified:return "mtime";
    case Field_Rating:      return "rating";
    case Field_Score:       return "score";
    case Field_Title:       return "title";
    case Field_Artist:      return "artist";
    case Field_Album:       return "album";
    case Field_AlbumArtist: return "albumartist";
    case Field_Composer:    return "composer";
    case Field_Genre:       return "genre";
    case Field_Comment:     return "comment";
    case FieldCount:        Q_ASSERT(0);
  }
  return QString();
}

QString SearchTerm::FieldName(Field field) {
  switch (field) {
    case Field_Length:      return Playlist::column_name(Playlist::Column_Length);
    case Field_Track:       return Playlist::column_name(Playlist::Column_Track);
    case Field_Disc:        return Playlist::column_name(Playlist::Column_Disc);
    case Field_Year:        return Playlist::column_name(Playlist::Column_Year);
    case Field_BPM:         return Playlist::column_name(Playlist::Column_BPM);
    case Field_Bitrate:     return Playlist::column_name(Playlist::Column_Bitrate);
    case Field_Samplerate:  return Playlist::column_name(Playlist::Column_Samplerate);
    case Field_Filesize:    return Playlist::column_name(Playlist::Column_Filesize);
    case Field_PlayCount:   return Playlist::column_name(Playlist::Column_PlayCount);
    case Field_SkipCount:   return Playlist::column_name(Playlist::Column_SkipCount);
    case Field_LastPlayed:  return Playlist::column_name(Playlist::Column_LastPlayed);
    case Field_DateCreated: return Playlist::column_name(Playlist::Column_DateCreated);
    case Field_DateModified:return Playlist::column_name(Playlist::Column_DateModified);
    case Field_Rating:      return Playlist::column_name(Playlist::Column_Rating);
    case Field_Score:       return Playlist::column_name(Playlist::Column_Score);
    case Field_Title:       return Playlist::column_name(Playlist::Column_Title);
    case Field_Artist:      return Playlist::column_name(Playlist::Column_Artist);
    case Field_Album:       return Playlist::column_name(Playlist::Column_Album);
    case Field_AlbumArtist: return Playlist::column_name(Playlist::Column_AlbumArtist);
    case Field_Composer:    return Playlist::column_name(Playlist::Column_Composer);
    case Field_Genre:       return Playlist::column_name(Playlist::Column_Genre);
    case Field_Comment:     return QObject::tr("Comment");
    case FieldCount:        Q_ASSERT(0);
  }
  return QString();
}

QString SearchTerm::FieldSortOrderText(Type type, bool ascending) {
  switch (type) {
    case Type_Text:   return ascending ? QObject::tr("A-Z")            : QObject::tr("Z-A");
    case Type_Date:   return ascending ? QObject::tr("oldest first")   : QObject::tr("newest first");
    case Type_Time:   return ascending ? QObject::tr("shortest first") : QObject::tr("longest first");
    case Type_Number:
    case Type_Rating: return ascending ? QObject::tr("smallest first") : QObject::tr("biggest first");
  }
  return QString();
}

} // namespace

QDataStream& operator <<(QDataStream& s, const smart_playlists::SearchTerm& term) {
  s << quint8(term.field_);
  s << quint8(term.operator_);
  s << term.value_;
  return s;
}

QDataStream& operator >>(QDataStream& s, smart_playlists::SearchTerm& term) {
  quint8 field, op;
  s >> field >> op >> term.value_;
  term.field_ = smart_playlists::SearchTerm::Field(field);
  term.operator_ = smart_playlists::SearchTerm::Operator(op);
  return s;
}
