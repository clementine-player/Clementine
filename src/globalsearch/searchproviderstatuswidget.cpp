/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#include "searchproviderstatuswidget.h"

#include <QMouseEvent>

#include "core/application.h"
#include "globalsearch.h"
#include "searchprovider.h"
#include "ui/iconloader.h"
#include "ui_searchproviderstatuswidget.h"

SearchProviderStatusWidget::SearchProviderStatusWidget(
    const QIcon& warning_icon, GlobalSearch* engine, SearchProvider* provider,
    QWidget* parent)
    : QWidget(parent),
      ui_(new Ui_SearchProviderStatusWidget),
      engine_(engine),
      provider_(provider) {
  ui_->setupUi(this);

  ui_->icon->setPixmap(provider->icon().pixmap(16));
  ui_->name->setText(provider->name());

  const bool enabled = engine->is_provider_enabled(provider);
  const bool logged_in = provider->IsLoggedIn();

  if (enabled && logged_in) {
    ui_->disabled_group->hide();
  } else {
    const QString disabled_text =
        tr("Disabled", "Refers to search provider's status.");
    const QString not_logged_in_text = tr("Not logged in");
    const int disabled_width = fontMetrics().width("    ") +
                               qMax(fontMetrics().width(disabled_text),
                                    fontMetrics().width(not_logged_in_text));

    ui_->disabled_reason->setMinimumWidth(disabled_width);
    ui_->disabled_reason->setText(logged_in ? disabled_text
                                            : not_logged_in_text);
    ui_->disabled_icon->setPixmap(warning_icon.pixmap(16));

    ui_->disabled_reason->installEventFilter(this);
  }
}

SearchProviderStatusWidget::~SearchProviderStatusWidget() { delete ui_; }

bool SearchProviderStatusWidget::eventFilter(QObject* object, QEvent* event) {
  if (object != ui_->disabled_reason) {
    return QWidget::eventFilter(object, event);
  }

  QFont font(ui_->disabled_reason->font());

  switch (event->type()) {
    case QEvent::Enter:
      font.setUnderline(true);
      ui_->disabled_reason->setFont(font);
      break;

    case QEvent::Leave:
      font.setUnderline(false);
      ui_->disabled_reason->setFont(font);
      break;

    case QEvent::MouseButtonRelease: {
      QMouseEvent* e = static_cast<QMouseEvent*>(event);
      if (e->button() == Qt::LeftButton) {
        if (!provider_->IsLoggedIn()) {
          provider_->ShowConfig();
        } else {
          engine_->application()->OpenSettingsDialogAtPage(
              SettingsDialog::Page_GlobalSearch);
        }
      }
      break;
    }

    default:
      return false;
  }

  return true;
}
