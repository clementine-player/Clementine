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

#include "edittagdialog.h"
#include "ui_edittagdialog.h"
#include "library/library.h"
#include "playlist/playlistdelegates.h"

#include <QtDebug>

const char* EditTagDialog::kHintText = QT_TR_NOOP("(different across multiple songs)");

EditTagDialog::EditTagDialog(QWidget* parent)
  : QDialog(parent),
    ui_(new Ui_EditTagDialog),
    ignore_edits_(false)
{
  ui_->setupUi(this);
  ui_->splitter->setSizes(QList<int>() << 200 << width() - 200);

  // An editable field is one that has a label as a buddy.  The label is
  // important because it gets turned bold when the field is changed.
  foreach (QLabel* label, findChildren<QLabel*>()) {
    QWidget* widget = label->buddy();
    if (widget) {
      // Store information about the field
      fields_ << FieldData(label, widget, widget->objectName());

      // Connect the Reset signal
      if (dynamic_cast<ExtendedEditor*>(widget)) {
        connect(widget, SIGNAL(Reset()), SLOT(ResetField()));
      }

      // Connect the edited signal
      if (qobject_cast<QLineEdit*>(widget)) {
        connect(widget, SIGNAL(textChanged(QString)), SLOT(FieldValueEdited()));
      } else if (qobject_cast<QPlainTextEdit*>(widget)) {
        connect(widget, SIGNAL(textChanged()), SLOT(FieldValueEdited()));
      } else if (qobject_cast<QSpinBox*>(widget)) {
        connect(widget, SIGNAL(valueChanged(int)), SLOT(FieldValueEdited()));
      }
    }
  }

  connect(ui_->song_list->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          SLOT(SelectionChanged()));
}

EditTagDialog::~EditTagDialog() {
  delete ui_;
}

bool EditTagDialog::SetSongs(const SongList& s) {
  data_.clear();

  foreach (const Song& song, s) {
    if (song.IsEditable()) {
      // Try reloading the tags from file
      Song copy(song);
      copy.InitFromFile(copy.filename(), copy.directory_id());

      if (copy.is_valid())
        data_ << Data(copy);
    }
  }

  if (data_.count() == 0)
    return false;

  // Add the filenames to the list
  ui_->song_list->clear();
  foreach (const Data& data, data_) {
    ui_->song_list->addItem(data.current_.basefilename());
  }

  // Select all
  ui_->song_list->selectAll();

  // Hide the list if there's only one song in it
  ui_->song_list->setVisible(data_.count() != 1);

  return true;
}

void EditTagDialog::SetTagCompleter(LibraryBackend* backend) {
  new TagCompleter(backend, Playlist::Column_Artist, ui_->artist);
  new TagCompleter(backend, Playlist::Column_Album, ui_->album);
}

QVariant EditTagDialog::Data::value(const Song& song, const QString& id) {
  if (id == "title")       return song.title();
  if (id == "artist")      return song.artist();
  if (id == "album")       return song.album();
  if (id == "albumartist") return song.albumartist();
  if (id == "composer")    return song.composer();
  if (id == "genre")       return song.genre();
  if (id == "comment")     return song.comment();
  if (id == "track")       return song.track();
  if (id == "disc")        return song.disc();
  if (id == "year")        return song.year();
  qDebug() << "Unknown ID" << id;
  return QVariant();
}

void EditTagDialog::Data::set_value(const QString& id, const QVariant& value) {
  if (id == "title")       current_.set_title(value.toString());
  if (id == "artist")      current_.set_artist(value.toString());
  if (id == "album")       current_.set_album(value.toString());
  if (id == "albumartist") current_.set_albumartist(value.toString());
  if (id == "composer")    current_.set_composer(value.toString());
  if (id == "genre")       current_.set_genre(value.toString());
  if (id == "comment")     current_.set_comment(value.toString());
  if (id == "track")       current_.set_track(value.toInt());
  if (id == "disc")        current_.set_disc(value.toInt());
  if (id == "year")        current_.set_year(value.toInt());
}

bool EditTagDialog::DoesValueVary(const QModelIndexList& sel, const QString& id) const {
  QVariant value = data_[sel.first().row()].current_value(id);
  for (int i=1 ; i<sel.count() ; ++i) {
    if (value != data_[sel[i].row()].current_value(id))
      return true;
  }
  return false;
}

bool EditTagDialog::IsValueModified(const QModelIndexList& sel, const QString& id) const {
  foreach (const QModelIndex& i, sel) {
    if (data_[i.row()].original_value(id) != data_[i.row()].current_value(id))
      return true;
  }
  return false;
}

void EditTagDialog::InitFieldValue(const FieldData& field, const QModelIndexList& sel) {
  const bool varies = DoesValueVary(sel, field.id_);
  const bool modified = IsValueModified(sel, field.id_);

  if (ExtendedEditor* editor = dynamic_cast<ExtendedEditor*>(field.editor_)) {
    editor->clear();
    editor->clear_hint();
    if (varies) {
      editor->set_hint(EditTagDialog::kHintText);
    } else {
      editor->set_text(data_[sel[0].row()].current_value(field.id_).toString());
    }
  }

  QFont new_font(font());
  new_font.setBold(modified);
  field.label_->setFont(new_font);
  field.editor_->setFont(new_font);
}

void EditTagDialog::UpdateFieldValue(const FieldData& field, const QModelIndexList& sel) {
  // Get the value from the field
  QVariant value;
  if (ExtendedEditor* editor = dynamic_cast<ExtendedEditor*>(field.editor_)) {
    value = editor->text();
  }

  // Did we get it?
  if (!value.isValid()) {
    return;
  }

  // Set it in each selected song
  foreach (const QModelIndex& i, sel) {
    data_[i.row()].set_value(field.id_, value);
  }

  // Update the boldness
  const bool modified = IsValueModified(sel, field.id_);

  QFont new_font(font());
  new_font.setBold(modified);
  field.label_->setFont(new_font);
  field.editor_->setFont(new_font);
}

void EditTagDialog::ResetFieldValue(const FieldData& field, const QModelIndexList& sel) {
  // Reset each selected song
  foreach (const QModelIndex& i, sel) {
    Data& data = data_[i.row()];
    data.set_value(field.id_, data.original_value(field.id_));
  }

  // Reset the field
  InitFieldValue(field, sel);
}

void EditTagDialog::SelectionChanged() {
  const QModelIndexList sel = ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty())
    return;

  ignore_edits_ = true;
  foreach (const FieldData& field, fields_) {
    InitFieldValue(field, sel);
  }
  ignore_edits_ = false;
}

void EditTagDialog::FieldValueEdited() {
  if (ignore_edits_)
    return;

  const QModelIndexList sel = ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty())
    return;

  QWidget* w = qobject_cast<QWidget*>(sender());

  // Find the field
  foreach (const FieldData& field, fields_) {
    if (field.editor_ == w) {
      UpdateFieldValue(field, sel);
      return;
    }
  }
}

void EditTagDialog::ResetField() {
  const QModelIndexList sel = ui_->song_list->selectionModel()->selectedIndexes();
  if (sel.isEmpty())
    return;

  QWidget* w = qobject_cast<QWidget*>(sender());

  // Find the field
  foreach (const FieldData& field, fields_) {
    if (field.editor_ == w) {
      ignore_edits_ = true;
      ResetFieldValue(field, sel);
      ignore_edits_ = false;
      return;
    }
  }
}
