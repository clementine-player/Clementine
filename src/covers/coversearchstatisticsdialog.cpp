/* This file is part of Clementine.
   Copyright 2011, David Sansome <davidsansome@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#include "coversearchstatisticsdialog.h"
#include "ui_coversearchstatisticsdialog.h"
#include "core/utilities.h"

#include <algorithm>

CoverSearchStatisticsDialog::CoverSearchStatisticsDialog(QWidget* parent)
    : QDialog(parent), ui_(new Ui_CoverSearchStatisticsDialog) {
  ui_->setupUi(this);
  details_layout_ = new QVBoxLayout(ui_->details);
  details_layout_->setSpacing(0);

  setStyleSheet(
      "#details {"
      "  background-color: palette(base);"
      "}"
      "#details QLabel[type=\"label\"] {"
      "  border: 2px solid transparent;"
      "  border-right: 2px solid palette(midlight);"
      "  margin-right: 10px;"
      "}"
      "#details QLabel[type=\"value\"] {"
      "  font-weight: bold;"
      "  max-width: 100px;"
      "}");
}

CoverSearchStatisticsDialog::~CoverSearchStatisticsDialog() { delete ui_; }

void CoverSearchStatisticsDialog::Show(
    const CoverSearchStatistics& statistics) {
  QStringList providers(statistics.total_images_by_provider_.keys());
  std::sort(providers.begin(), providers.end());

  ui_->summary->setText(
      tr("Got %1 covers out of %2 (%3 failed)")
          .arg(statistics.chosen_images_)
          .arg(statistics.chosen_images_ + statistics.missing_images_)
          .arg(statistics.missing_images_));

  for (const QString& provider : providers) {
    AddLine(tr("Covers from %1").arg(provider),
            QString::number(statistics.chosen_images_by_provider_[provider]));
  }

  if (!providers.isEmpty()) {
    AddSpacer();
  }

  AddLine(tr("Total network requests made"),
          QString::number(statistics.network_requests_made_));
  AddLine(tr("Average image size"), statistics.AverageDimensions());
  AddLine(tr("Total bytes transferred"),
          statistics.bytes_transferred_
              ? Utilities::PrettySize(statistics.bytes_transferred_)
              : "0 bytes");

  details_layout_->addStretch();

  show();
}

void CoverSearchStatisticsDialog::AddLine(const QString& label,
                                          const QString& value) {
  QLabel* label1 = new QLabel(label);
  QLabel* label2 = new QLabel(value);

  label1->setProperty("type", "label");
  label2->setProperty("type", "value");

  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(label1);
  layout->addWidget(label2);
  details_layout_->addLayout(layout);
}

void CoverSearchStatisticsDialog::AddSpacer() {
  details_layout_->addSpacing(20);
}
