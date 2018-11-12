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

#include "about.h"
#include "config.h"
#include "ui_about.h"

#include <algorithm>

#include <QCoreApplication>
#include <QPushButton>

const char* About::kUrl = "http://www.clementine-player.org/";

About::About(QWidget* parent) : QDialog(parent) {
  ui_.setupUi(this);

  setWindowTitle(tr("About %1").arg(QCoreApplication::applicationName()));
  ui_.title->setText(QCoreApplication::applicationName());
  ui_.version->setText(
      tr("Version %1").arg(QCoreApplication::applicationVersion()));

  QFont title_font;
  title_font.setBold(true);
  title_font.setPointSize(title_font.pointSize() + 4);
  ui_.title->setFont(title_font);

  authors_ << Person("David Sansome", "me@davidsansome.com")
           << Person("John Maguire", "john.maguire@gmail.com")
           << Person(QString::fromUtf8("Paweł Bara"), "keirangtp@gmail.com")
           << Person("Arnaud Bienner", "arnaud.bienner@gmail.com");
  thanks_to_ << Person("Mark Kretschmann", "kretschmann@kde.org")
             << Person("Max Howell", "max.howell@methylblue.com")
             << Person(QString::fromUtf8("Bartłomiej Burdukiewicz"),
                       "dev.strikeu@gmail.com")
             << Person("Jakub Stachowski", "qbast@go2.pl")
             << Person("Paul Cifarelli", "paul@cifarelli.net")
             << Person("Felipe Rivera", "liebremx@users.sourceforge.net")
             << Person("Alexander Peitz")
             << Person("Artur Rona", "artur.rona@gmail.com")
             << Person("Andreas Muttscheller", "asfa194@gmail.com")
             << Person("Mark Furneaux", "mark@furneaux.ca");

  std::sort(authors_.begin(), authors_.end());
  std::sort(thanks_to_.begin(), thanks_to_.end());

  ui_.content->setHtml(MakeHtml());

  ui_.buttonBox->button(QDialogButtonBox::Close)
      ->setShortcut(QKeySequence::Close);
}

QString About::MakeHtml() const {
  QString ret = QString(
                    "<p><a href=\"%1\">%2</a></p>"
                    "<p><b>%3:</b>").arg(kUrl, kUrl, tr("Authors"));

  for (const Person& person : authors_) {
    ret += "<br />" + MakeHtml(person);
  }

  ret += QString("</p><p><b>%3:</b>").arg(tr("Thanks to"));

  for (const Person& person : thanks_to_) {
    ret += "<br />" + MakeHtml(person);
  }
  ret += QString(
      "<br />" + tr("All the translators") +
      " &lt;<a href=\"https://www.transifex.com/projects/p/clementine/\">"
      "https://www.transifex.com/projects/p/clementine/</a>&gt;");

  ret += QString("<br />%1</p>").arg(tr("...and all the Amarok contributors"));
  ret += QString("<p><b>%1</b>").arg(tr("And:"));
  ret += QString("<br /><a href=\"http://rainymood.com\">Rainy Mood</a>");
  ret += QString(
      "<br /><a href=\"http://www.smitelli.com/?page=blog&p=54\">Scott "
      "Smitelli</a>");
  ret += QString(
      "<br /><a href=\"http://hyperboleandahalf.blogspot.com\">Allie "
      "Brosh</a></p>");

  ret +=
      "<p>This product uses Music by Spotify but is not endorsed, certified "
      "or otherwise approved in any way by Spotify. Spotify is the registered "
      "trade mark of the Spotify Group.</p>";

  return ret;
}

QString About::MakeHtml(const Person& person) const {
  if (person.email.isNull())
    return person.name;
  else
    return QString("%1 &lt;<a href=\"mailto:%2\">%3</a>&gt;")
        .arg(person.name, person.email, person.email);
}
