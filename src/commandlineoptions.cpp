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

#include "commandlineoptions.h"

#include <cstdlib>
#include <getopt.h>
#include <iostream>

#include <QFileInfo>
#include <QBuffer>

const char* CommandlineOptions::kHelpText =
    "%1: clementine [%2] [%3]\n"
    "\n"
    "%4:\n"
    "  -p, --play                %5\n"
    "  -t, --play-pause          %6\n"
    "  -u, --pause               %7\n"
    "  -s, --stop                %8\n"
    "  -r, --previous            %9\n"
    "  -f, --next                %10\n"
    "\n"
    "%11:\n"
    "  -a, --append              %12\n"
    "  -l, --load                %13\n";


CommandlineOptions::CommandlineOptions(int argc, char** argv)
  : argc_(argc),
    argv_(argv),
    url_list_action_(UrlList_Append),
    player_action_(Player_None)
{
}

bool CommandlineOptions::Parse() {
  static const struct option kOptions[] = {
    {"help",       no_argument, 0, 'h'},

    {"append",     no_argument, 0, 'a'},
    {"load",       no_argument, 0, 'l'},

    {"play",       no_argument, 0, 'p'},
    {"play-pause", no_argument, 0, 't'},
    {"pause",      no_argument, 0, 'u'},
    {"stop",       no_argument, 0, 's'},
    {"previous",   no_argument, 0, 'r'},
    {"next",       no_argument, 0, 'f'},

    {0, 0, 0, 0}
  };

  // Parse the arguments
  int option_index = 0;
  forever {
    int c = getopt_long(argc_, argv_, "", kOptions, &option_index);

    // End of the options
    if (c == -1)
      break;

    switch (c) {
      case 'h': {
        QString translated_help_text = QString(kHelpText).arg(
            tr("Usage"), tr("options"), tr("files or URL(s)"), tr("Options"),
            tr("Start the playlist currently playing"),
            tr("Play if stopped, pause if playing"),
            tr("Pause playback"),
            tr("Stop playback"),
            tr("Skip backwards in playlist")).arg(
            tr("Skip forwards in playlist"),
            tr("Additional options"),
            tr("Append files/URLs to the playlist"),
            tr("Loads files/URLs, replacing current playlist"));

        std::cout << translated_help_text.toLocal8Bit().constData();
        return false;
      }

      case 'a': url_list_action_ = UrlList_Append;   break;
      case 'l': url_list_action_ = UrlList_Load;     break;
      case 'p': player_action_   = Player_Play;      break;
      case 't': player_action_   = Player_PlayPause; break;
      case 'u': player_action_   = Player_Pause;     break;
      case 's': player_action_   = Player_Stop;      break;
      case 'r': player_action_   = Player_Previous;  break;
      case 'f': player_action_   = Player_Next;      break;
      case '?':
      default:
        return false;
    }
  }

  // Get any filenames or URLs following the arguments
  for (int i=option_index+1 ; i<argc_ ; ++i) {
    QString value = QString::fromLocal8Bit(argv_[i]);
    if (value.contains("://"))
      urls_ << value;
    else
      urls_ << QUrl::fromLocalFile(QFileInfo(value).absoluteFilePath());
  }

  return true;
}

QByteArray CommandlineOptions::Serialize() const {
  QBuffer buf;
  buf.open(QIODevice::WriteOnly);

  QDataStream s(&buf);
  s << *this;
  buf.close();

  return buf.data();
}

void CommandlineOptions::Load(const QByteArray &serialized) {
  QByteArray copy(serialized);
  QBuffer buf(&copy);
  buf.open(QIODevice::ReadOnly);

  QDataStream s(&buf);
  s >> *this;
}

QString CommandlineOptions::tr(const char *source_text) {
  return QObject::tr(source_text);
}

QDataStream& operator<<(QDataStream& s, const CommandlineOptions& a) {
  s << qint32(a.player_action_)
    << qint32(a.url_list_action_)
    << a.urls_;

  return s;
}

QDataStream& operator>>(QDataStream& s, CommandlineOptions& a) {
  s >> reinterpret_cast<qint32&>(a.player_action_)
    >> reinterpret_cast<qint32&>(a.url_list_action_)
    >> a.urls_;

  return s;
}
