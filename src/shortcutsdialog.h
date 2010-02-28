#ifndef SHORTCUTSDIALOG_H
#define SHORTCUTSDIALOG_H

#include <QDialog>
#include <QSettings>

#include "ui_shortcutsdialog.h"

class ShortcutsDialog : public QDialog {
  Q_OBJECT

  public:
    ShortcutsDialog(QWidget* parent = 0);

  private slots:
    void DefaultShortcuts();
    void SaveShortcuts();
    void CancelEvent();


  private:
    Ui::ShortcutsDialog ui_;
    QSettings settings_;
    static const char* kSettingsGroup;
};

#endif // SHORTCUTSDIALOG_H