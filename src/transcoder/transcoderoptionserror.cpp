/* This file is part of Clementine.
   Copyright 2021, Jim Broadus <jbroadus@gmail.com>

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

#include "transcoderoptionserror.h"

#include "core/utilities.h"
#include "transcoder.h"
#include "ui_transcoderoptionserror.h"

TranscoderOptionsError::TranscoderOptionsError(const QString& mime_type,
                                               const QString& element,
                                               QWidget* parent)
    : TranscoderOptionsInterface(parent), ui_(new Ui_TranscoderOptionsError) {
  ui_->setupUi(this);

  if (mime_type.isEmpty()) {
    // No codec for raw formats such as wav.
    ui_->info->setText(tr("No settings available for this type."));
  } else if (element.isEmpty()) {
    // Didn't find a suitable element.
    ui_->info->setText(tr("Could not find a suitable encoder element "
                          "for <b>%1</b>.")
                           .arg(mime_type));
  } else {
    // No settings page available for element.
    QString url = Utilities::MakeBugReportUrl(
        QString("transcoder settings: Unknown encoder element: ") + element);
    ui_->info->setText(tr("No settings page available for encoder "
                          "element <b>%1</b>. "
                          "Please report this issue:<br>"
                          "<a href=\"%2\">%2</a>")
                           .arg(element)
                           .arg(url));
  }
}

TranscoderOptionsError::~TranscoderOptionsError() { delete ui_; }
