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

#include "stylesheetloader.h"
#include "core/logging.h"

#include <QFile>
#include <QEvent>

StyleSheetLoader::StyleSheetLoader(QObject* parent) : QObject(parent) {}

void StyleSheetLoader::SetStyleSheet(QWidget* widget, const QString& filename) {
  filenames_[widget] = filename;
  widget->installEventFilter(this);
  UpdateStyleSheet(widget);
}

void StyleSheetLoader::UpdateStyleSheet(QWidget* widget) {
  QString filename(filenames_[widget]);

  // Load the file
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
    qLog(Warning) << "error opening" << filename;
    return;
  }
  QString contents(file.readAll());

  // Replace %palette-role with actual colours
  QPalette p(widget->palette());

  QColor alt = p.color(QPalette::AlternateBase);
  alt.setAlpha(50);
  contents.replace("%palette-alternate-base", QString("rgba(%1,%2,%3,%4%)")
                                                  .arg(alt.red())
                                                  .arg(alt.green())
                                                  .arg(alt.blue())
                                                  .arg(alt.alpha()));

  ReplaceColor(&contents, "Window", p, QPalette::Window);
  ReplaceColor(&contents, "Background", p, QPalette::Background);
  ReplaceColor(&contents, "WindowText", p, QPalette::WindowText);
  ReplaceColor(&contents, "Foreground", p, QPalette::Foreground);
  ReplaceColor(&contents, "Base", p, QPalette::Base);
  ReplaceColor(&contents, "AlternateBase", p, QPalette::AlternateBase);
  ReplaceColor(&contents, "ToolTipBase", p, QPalette::ToolTipBase);
  ReplaceColor(&contents, "ToolTipText", p, QPalette::ToolTipText);
  ReplaceColor(&contents, "Text", p, QPalette::Text);
  ReplaceColor(&contents, "Button", p, QPalette::Button);
  ReplaceColor(&contents, "ButtonText", p, QPalette::ButtonText);
  ReplaceColor(&contents, "BrightText", p, QPalette::BrightText);
  ReplaceColor(&contents, "Light", p, QPalette::Light);
  ReplaceColor(&contents, "Midlight", p, QPalette::Midlight);
  ReplaceColor(&contents, "Dark", p, QPalette::Dark);
  ReplaceColor(&contents, "Mid", p, QPalette::Mid);
  ReplaceColor(&contents, "Shadow", p, QPalette::Shadow);
  ReplaceColor(&contents, "Highlight", p, QPalette::Highlight);
  ReplaceColor(&contents, "HighlightedText", p, QPalette::HighlightedText);
  ReplaceColor(&contents, "Link", p, QPalette::Link);
  ReplaceColor(&contents, "LinkVisited", p, QPalette::LinkVisited);

#ifdef Q_OS_DARWIN
  contents.replace("darwin", "*");
#endif

  widget->setStyleSheet(contents);
}

void StyleSheetLoader::ReplaceColor(QString* css, const QString& name,
                                    const QPalette& palette,
                                    QPalette::ColorRole role) const {
  css->replace("%palette-" + name + "-lighter",
               palette.color(role).lighter().name(), Qt::CaseInsensitive);
  css->replace("%palette-" + name + "-darker",
               palette.color(role).darker().name(), Qt::CaseInsensitive);
  css->replace("%palette-" + name, palette.color(role).name(),
               Qt::CaseInsensitive);
}

bool StyleSheetLoader::eventFilter(QObject* obj, QEvent* event) {
  if (event->type() != QEvent::PaletteChange) return false;

  QWidget* widget = qobject_cast<QWidget*>(obj);
  if (!widget || !filenames_.contains(widget)) return false;

  UpdateStyleSheet(widget);
  return false;
}
