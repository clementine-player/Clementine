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

#ifndef COMMANDLINEOPTIONS_H
#define COMMANDLINEOPTIONS_H

#include <QList>
#include <QUrl>
#include <QDataStream>

class CommandlineOptions {
  friend QDataStream& operator<<(QDataStream& s, const CommandlineOptions& a);
  friend QDataStream& operator>>(QDataStream& s, CommandlineOptions& a);

 public:
  CommandlineOptions(int argc = 0, char* *argv = NULL);

  static const char* kHelpText;
  static const char* kVersionText;

  // Don't change the values or order, these get serialised and sent to
  // possibly a different version of Clementine
  enum UrlListAction { UrlList_Append = 0, UrlList_Load = 1, };
  enum PlayerAction {
    Player_None = 0,
    Player_Play = 1,
    Player_PlayPause = 2,
    Player_Pause = 3,
    Player_Stop = 4,
    Player_Previous = 5,
    Player_Next = 6,
    Player_RestartOrPrevious = 7,
  };

  bool Parse();

  bool is_empty() const;

  UrlListAction url_list_action() const { return url_list_action_; }
  PlayerAction player_action() const { return player_action_; }
  int set_volume() const { return set_volume_; }
  int volume_modifier() const { return volume_modifier_; }
  int seek_to() const { return seek_to_; }
  int seek_by() const { return seek_by_; }
  int play_track_at() const { return play_track_at_; }
  bool show_osd() const { return show_osd_; }
  bool toggle_pretty_osd() const { return toggle_pretty_osd_; }
  QList<QUrl> urls() const { return urls_; }
  QString language() const { return language_; }
  QString log_levels() const { return log_levels_; }

  QByteArray Serialize() const;
  void Load(const QByteArray& serialized);

 private:
  // These are "invalid" characters to pass to getopt_long for options that
  // shouldn't have a short (single character) option.
  enum LongOptions {
    VolumeUp = 256,
    VolumeDown,
    SeekTo,
    SeekBy,
    Quiet,
    Verbose,
    LogLevels,
    Version,
    VolumeIncreaseBy,
    VolumeDecreaseBy,
    RestartOrPrevious
  };

  QString tr(const char* source_text);
  void RemoveArg(const QString& starts_with, int count);

 private:
  int argc_;
  char** argv_;

  UrlListAction url_list_action_;
  PlayerAction player_action_;

  // Don't change the type of these.
  int set_volume_;
  int volume_modifier_;
  int seek_to_;
  int seek_by_;
  int play_track_at_;
  bool show_osd_;
  bool toggle_pretty_osd_;
  QString language_;
  QString log_levels_;

  QList<QUrl> urls_;
};

QDataStream& operator<<(QDataStream& s, const CommandlineOptions& a);
QDataStream& operator>>(QDataStream& s, CommandlineOptions& a);

#endif  // COMMANDLINEOPTIONS_H
