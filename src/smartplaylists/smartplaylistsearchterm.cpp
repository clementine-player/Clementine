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

#include "smartplaylistsearchterm.h"
#include "playlist/playlist.h"

SmartPlaylistSearchTerm::SmartPlaylistSearchTerm()
{
}

QString SmartPlaylistSearchTerm::ToSql() const {
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
      return col + " LIKE '" + value + "'";
    case Op_GreaterThan:
      return col + " > '" + value + "'";
    case Op_LessThan:
      return col + " < '" + value + "'";
  }

  return QString();
}

SmartPlaylistSearchTerm::Type SmartPlaylistSearchTerm::TypeOf(Field field) {
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

OperatorList SmartPlaylistSearchTerm::OperatorsForType(Type type) {
  switch (type) {
    case Type_Text:
      return OperatorList() << Op_Contains << Op_NotContains << Op_Equals
                            << Op_StartsWith << Op_EndsWith;
    default:
      return OperatorList() << Op_Equals << Op_GreaterThan << Op_LessThan;
  }
}

QString SmartPlaylistSearchTerm::OperatorText(Type type, Operator op) {
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

QString SmartPlaylistSearchTerm::FieldColumnName(Field field) {
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

QString SmartPlaylistSearchTerm::FieldName(Field field) {
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

QDataStream& operator <<(QDataStream& s, const SmartPlaylistSearchTerm& term) {
  s << quint8(term.field_);
  s << quint8(term.operator_);
  s << term.value_;
  return s;
}

QDataStream& operator >>(QDataStream& s, SmartPlaylistSearchTerm& term) {
  quint8 field, op;
  s >> field >> op >> term.value_;
  term.field_ = SmartPlaylistSearchTerm::Field(field);
  term.operator_ = SmartPlaylistSearchTerm::Operator(op);
  return s;
}
