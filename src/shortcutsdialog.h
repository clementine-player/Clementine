#ifndef SHORTCUTSDIALOG_H
#define SHORTCUTSDIALOG_H

#include <QDialogButtonBox>
#include <QDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSettings>

#include "ui_shortcutsdialog.h"

// Proper row names
#define SD_ROW_PLAY "Play"
#define SD_ROW_PAUSE "Pause"
#define SD_ROW_PLAY_PAUSE "Play/Pause"
#define SD_ROW_STOP "Stop"
#define SD_ROW_STOP_AFTER "Stop Playing After Current Track"
#define SD_ROW_NEXT_TRACK "Next Track"
#define SD_ROW_PREV_TRACK "Previous Track"
#define SD_ROW_INC_VOLUME "Increase Volume"
#define SD_ROW_DEC_VOLUME "Decrease Volume"
#define SD_ROW_MUTE "Mute"
#define SD_ROW_SEEK_FORWARDS "Seek Forwards"
#define SD_ROW_SEEK_BACKWARDS "Seek Backwards"

// Defaults from Amarok 1.4
// Meta = Windows key or Apple key
#define SD_DEFAULT_PLAY "Meta+X"
#define SD_DEFAULT_PLAY_PAUSE "Meta+C"
#define SD_DEFAULT_STOP "Meta+V"
#define SD_DEFAULT_STOP_AFTER "Meta+Ctrl+V"
#define SD_DEFAULT_NEXT_TRACK "Meta+B"
#define SD_DEFAULT_PREV_TRACK "Meta+Z"
#define SD_DEFAULT_INC_VOLUME "Meta+KP_Add"
#define SD_DEFAULT_DEC_VOLUME "Meta+KP_Subtract"
#define SD_DEFAULT_MUTE_VOLUME "Meta+M"
#define SD_DEFAULT_SEEK_FORWARDS "Meta+Shift+KP_Add"
#define SD_DEFAULT_SEEK_BACKWARDS "Meta+Shift+KP_Subtract"

class ShortcutsDialog : public QDialog {
  Q_OBJECT

  public:
    ShortcutsDialog(QWidget* parent = 0);

  private slots:
    void ResetShortcuts();
    void SaveShortcuts();
    void CancelEvent();
    void GetShortcut(QKeyEvent* event);
    void CellClickedEvent();
    void DefaultText(QString str);
    void DefaultRadioClickedEvent();

  private:
    Ui::ShortcutsDialog ui_;
    QSettings settings_;
    static const char* kSettingsGroup;
    QTableWidgetItem *item; // current cell
    QString currentDefault_;
    QString currentKey_;
    QStringList keys_;
};

#endif // SHORTCUTSDIALOG_H