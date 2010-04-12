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

#ifndef COMMANDLINEOPTIONS_H
#define COMMANDLINEOPTIONS_H

#include <QList>
#include <QUrl>
#include <QDataStream>

class CommandlineOptions {
  friend QDataStream& operator<<(QDataStream& s, const CommandlineOptions& a);
  friend QDataStream& operator>>(QDataStream& s, CommandlineOptions& a);

 public:
  CommandlineOptions(int argc = 0, char** argv = NULL);

  static const char* kHelpText;

  // Don't change the values or order, these get serialised and sent to
  // possibly a different version of Clementine
  enum UrlListAction {
    UrlList_Append = 0,
    UrlList_Load = 1,
  };
  enum PlayerAction {
    Player_None = 0,
    Player_Play = 1,
    Player_PlayPause = 2,
    Player_Pause = 3,
    Player_Stop = 4,
    Player_Previous = 5,
    Player_Next = 6,
  };

  bool Parse();

  UrlListAction url_list_action() const { return url_list_action_; }
  PlayerAction player_action() const { return player_action_; }
  QList<QUrl> urls() const { return urls_; }

  QByteArray Serialize() const;
  void Load(const QByteArray& serialized);

 private:
  QString tr(const char* source_text);

 private:
  int argc_;
  char** argv_;

  UrlListAction url_list_action_;
  PlayerAction player_action_;

  QList<QUrl> urls_;
};

QDataStream& operator<<(QDataStream& s, const CommandlineOptions& a);
QDataStream& operator>>(QDataStream& s, CommandlineOptions& a);

#endif // COMMANDLINEOPTIONS_H
