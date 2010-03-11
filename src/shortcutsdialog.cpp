#include "shortcutsdialog.h"

#include <QtDebug>

const char* ShortcutsDialog::kSettingsGroup = "Shortcuts";
ShortcutsDialog::ShortcutsDialog(QWidget* parent)
  : QDialog(parent) {
  ui_.setupUi(this);
  ui_.shortcut_options->setEnabled(false);

  // Load settings

  int i = 0;
  QStringList rowLabels;
  QString str;

  keys_ << "play" << "pause" << "play_pause" << "stop" << "stop_after";
  keys_ << "next_track" << "prev_track" << "inc_volume" << "dec_volume";
  keys_ << "mute" << "seek_forwards" << "seek_backwards";

  settings_.beginGroup(kSettingsGroup);
  foreach(str, keys_) {
    if (settings_.contains(str)) {
      if (QString::compare(str, "play_pause") == 0) {
        rowLabels << SD_ROW_PLAY_PAUSE;
      }
      else if (QString::compare(str, "stop_after") == 0) {
        rowLabels << SD_ROW_STOP_AFTER;
      }
      else if (QString::compare(str, "next_track") == 0) {
        rowLabels << SD_ROW_NEXT_TRACK;
      }
      else if (QString::compare(str, "prev_track") == 0) {
        rowLabels << SD_ROW_PREV_TRACK;
      }
      else if (QString::compare(str, "inc_volume") == 0) {
        rowLabels << SD_ROW_INC_VOLUME;
      }
      else if (QString::compare(str, "dec_volume") == 0) {
        rowLabels << SD_ROW_DEC_VOLUME;
      }
      else if (QString::compare(str, "seek_forwards") == 0) {
        rowLabels << SD_ROW_SEEK_FORWARDS;
      }
      else if (QString::compare(str, "seek_backwards") == 0) {
        rowLabels << SD_ROW_SEEK_BACKWARDS;
      }
      else {
        // Uppercase first letter
        str[0] = str[0].toUpper();
        rowLabels << str;
      }
    }
    else {
      settings_.setValue(str, "");
    }
  }

  ui_.table->setRowCount(rowLabels.length());
  ui_.table->setVerticalHeaderLabels(rowLabels);



  // TODO: QKeySequence::toString() to load/save values

  //connect(ui_.button_box->button(QDialogButtonBox::Reset), SIGNAL(clicked()), SLOT(ResetShortcuts()));
  connect(ui_.button_box, SIGNAL(accepted()), SLOT(SaveShortcuts()));
  connect(ui_.button_box, SIGNAL(rejected()), SLOT(CancelEvent()));
  connect(ui_.table, SIGNAL(cellClicked(int, int)), SLOT(CellClickedEvent()));
  connect(ui_.radio_default, SIGNAL(clicked()), SLOT(DefaultRadioClickedEvent()));
}

/**
  * Reset shortcuts to defaults (none for now).
  */
void ShortcutsDialog::ResetShortcuts() {
  int ret = QMessageBox::warning(this, tr("Warning"),
    tr("You are about to reset to global shortcuts default values. Are you sure you want to continue?"),
    QMessageBox::Yes,
    QMessageBox::No);
  if (ret != QMessageBox::No) {
  }
}

void ShortcutsDialog::SaveShortcuts() {
  accept();
}

/**
  * Reset back to original values found in settings file and close
  */
void ShortcutsDialog::CancelEvent() {
  close();
}

void ShortcutsDialog::DefaultText(QString str) {
  if (QString::compare(str, SD_ROW_PLAY) == 0) {
    currentDefault_ = tr(SD_DEFAULT_PLAY);
    ui_.label->setText(tr("Default: %1").arg(SD_DEFAULT_PLAY));
  }
  else if (QString::compare(str, SD_ROW_PLAY_PAUSE) == 0) {
    currentDefault_ = tr(SD_DEFAULT_PLAY_PAUSE);
    ui_.label->setText(tr("Default: %1").arg(SD_DEFAULT_PLAY_PAUSE));
  }
  else if (QString::compare(str, SD_ROW_STOP) == 0) {
    currentDefault_ = tr(SD_DEFAULT_STOP);
    ui_.label->setText(tr("Default: %1").arg(SD_DEFAULT_STOP));
  }
  else if (QString::compare(str, SD_ROW_STOP_AFTER) == 0) {
    currentDefault_ = tr(SD_DEFAULT_STOP_AFTER);
    ui_.label->setText(tr("Default: %1").arg(SD_DEFAULT_STOP_AFTER));
  }
  else if (QString::compare(str, SD_ROW_NEXT_TRACK) == 0) {
    currentDefault_ = tr(SD_DEFAULT_NEXT_TRACK);
    ui_.label->setText(tr("Default: %1").arg(SD_DEFAULT_NEXT_TRACK));
  }
  else if (QString::compare(str, SD_ROW_PREV_TRACK) == 0) {
    currentDefault_ = tr(SD_DEFAULT_PREV_TRACK);
    ui_.label->setText(tr("Default: %1").arg(SD_DEFAULT_PREV_TRACK));
  }
  else if (QString::compare(str, SD_ROW_INC_VOLUME) == 0) {
    currentDefault_ = tr(SD_DEFAULT_INC_VOLUME);
    ui_.label->setText(tr("Default: %1").arg(SD_DEFAULT_INC_VOLUME));
  }
  else if (QString::compare(str, SD_ROW_DEC_VOLUME) == 0) {
    currentDefault_ = tr(SD_DEFAULT_DEC_VOLUME);
    ui_.label->setText(tr("Default: %1").arg(SD_DEFAULT_DEC_VOLUME));
  }
  else if (QString::compare(str, SD_ROW_MUTE) == 0) {
    currentDefault_ = tr(SD_DEFAULT_MUTE_VOLUME);
    ui_.label->setText(tr("Default: %1").arg(SD_DEFAULT_MUTE_VOLUME));
  }
  else if (QString::compare(str, SD_ROW_SEEK_FORWARDS) == 0) {
    currentDefault_ = tr(SD_DEFAULT_SEEK_FORWARDS);
    ui_.label->setText(tr("Default: %1").arg(SD_DEFAULT_SEEK_FORWARDS));
  }
  else if (QString::compare(str, SD_ROW_SEEK_BACKWARDS) == 0) {
    currentDefault_ = tr(SD_DEFAULT_SEEK_BACKWARDS);
    ui_.label->setText(tr("Default: %1").arg(SD_DEFAULT_SEEK_BACKWARDS));
  }
  else {
    currentDefault_ = tr("");
    ui_.label->setText(tr("Default: None"));
  }
}

void ShortcutsDialog::CellClickedEvent() {
  ui_.shortcut_options->setEnabled(true);
  DefaultText(ui_.table->verticalHeaderItem(ui_.table->currentRow())->text());

  currentKey_ = keys_.at(ui_.table->currentRow());

  //TODO: Read setting and set correct radio button
	//      Disable ALL hotkey functionality built-in to Qt for this widget
}

void ShortcutsDialog::DefaultRadioClickedEvent() {
  settings_.setValue(currentKey_, currentDefault_);
}

bool ShortcutsDialog::event(QEvent* event) {	
	if (event->type() == QEvent::ShortcutOverride) {
	  QKeyEvent *ke = static_cast<QKeyEvent *>(event);
		qDebug() << ke->text();
		qDebug() << ke->key();
		return true;
	}
	return QWidget::event(event);
}
