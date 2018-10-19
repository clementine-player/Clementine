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

#include "behavioursettingspage.h"
#include "mainwindow.h"
#include "core/player.h"
#include "ui_behavioursettingspage.h"
#include "playlist/playlist.h"
#include "playlist/playlisttabbar.h"

#include <algorithm>

#include <QDir>
#include <QSystemTrayIcon>

namespace {
bool LocaleAwareCompare(const QString& a, const QString& b) {
  return a.localeAwareCompare(b) < 0;
}
}  // namespace

BehaviourSettingsPage::BehaviourSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog), ui_(new Ui_BehaviourSettingsPage) {
  ui_->setupUi(this);

  connect(ui_->b_show_tray_icon_, SIGNAL(toggled(bool)),
          SLOT(ShowTrayIconToggled(bool)));

  ui_->doubleclick_addmode->setItemData(0, MainWindow::AddBehaviour_Append);
  ui_->doubleclick_addmode->setItemData(1, MainWindow::AddBehaviour_Load);
  ui_->doubleclick_addmode->setItemData(2, MainWindow::AddBehaviour_OpenInNew);
  ui_->doubleclick_addmode->setItemData(3, MainWindow::AddBehaviour_Enqueue);

  ui_->doubleclick_playmode->setItemData(0, MainWindow::PlayBehaviour_Never);
  ui_->doubleclick_playmode->setItemData(1,
                                         MainWindow::PlayBehaviour_IfStopped);
  ui_->doubleclick_playmode->setItemData(2, MainWindow::PlayBehaviour_Always);

  ui_->doubleclick_playlist_addmode->setItemData(
      0, MainWindow::PlaylistAddBehaviour_Play);
  ui_->doubleclick_playlist_addmode->setItemData(
      1, MainWindow::PlaylistAddBehaviour_Enqueue);

  ui_->menu_playmode->setItemData(0, MainWindow::PlayBehaviour_Never);
  ui_->menu_playmode->setItemData(1, MainWindow::PlayBehaviour_IfStopped);
  ui_->menu_playmode->setItemData(2, MainWindow::PlayBehaviour_Always);

  ui_->menu_previousmode->setItemData(0, Player::PreviousBehaviour_DontRestart);
  ui_->menu_previousmode->setItemData(1, Player::PreviousBehaviour_Restart);

  // Populate the language combo box.  We do this by looking at all the
  // compiled in translations.
  QDir dir(":/translations/");
  QStringList codes(dir.entryList(QStringList() << "*.qm"));
  QRegExp lang_re("^clementine_(.*).qm$");
  for (const QString& filename : codes) {
    // The regex captures the "ru" from "clementine_ru.qm"
    if (!lang_re.exactMatch(filename)) continue;

    QString code = lang_re.cap(1);
    QString lookup_code = QString(code)
                              .replace("@latin", "_Latn")
                              .replace("_CN", "_Hans_CN")
                              .replace("_TW", "_Hant_TW");
    QString language_name =
        QLocale::languageToString(QLocale(lookup_code).language());
#if QT_VERSION >= 0x040800
    QString native_name = QLocale(lookup_code).nativeLanguageName();
    if (!native_name.isEmpty()) {
      language_name = native_name;
    }
#endif
    QString name = QString("%1 (%2)").arg(language_name, code);

    language_map_[name] = code;
  }

  language_map_["English (en)"] = "en";

  // Sort the names and show them in the UI
  QStringList names = language_map_.keys();
  std::stable_sort(names.begin(), names.end(), LocaleAwareCompare);
  ui_->language->addItems(names);

#ifdef Q_OS_DARWIN
  ui_->b_show_tray_icon_->setEnabled(false);
  ui_->startup_group_->setEnabled(false);
#else
  if (QSystemTrayIcon::isSystemTrayAvailable()) {
    ui_->b_show_tray_icon_->setEnabled(true);
    ui_->startup_group_->setEnabled(true);
  } else {
    ui_->b_show_tray_icon_->setEnabled(false);
    ui_->startup_group_->setEnabled(false);
  }
#endif
}

BehaviourSettingsPage::~BehaviourSettingsPage() { delete ui_; }

void BehaviourSettingsPage::Load() {
  QSettings s;

  s.beginGroup(MainWindow::kSettingsGroup);
#ifdef Q_OS_DARWIN
  ui_->b_show_tray_icon_->setChecked(false);
  ui_->b_scroll_tray_icon_->setChecked(false);
  ui_->b_keep_running_->setChecked(false);
#else
  if (QSystemTrayIcon::isSystemTrayAvailable()) {
    ui_->b_show_tray_icon_->setChecked(s.value("showtray", true).toBool());
    ui_->b_scroll_tray_icon_->setChecked(
        s.value("scrolltrayicon", ui_->b_show_tray_icon_->isChecked())
            .toBool());
    ui_->b_keep_running_->setChecked(
        s.value("keeprunning", ui_->b_show_tray_icon_->isChecked()).toBool());
  } else {
    ui_->b_show_tray_icon_->setChecked(false);
    ui_->b_scroll_tray_icon_->setChecked(false);
    ui_->b_keep_running_->setChecked(false);
  }
#endif

  ui_->doubleclick_addmode->setCurrentIndex(ui_->doubleclick_addmode->findData(
      s.value("doubleclick_addmode", MainWindow::AddBehaviour_Append).toInt()));
  ui_->doubleclick_playmode->setCurrentIndex(
      ui_->doubleclick_playmode->findData(
          s.value("doubleclick_playmode", MainWindow::PlayBehaviour_IfStopped)
              .toInt()));
  ui_->doubleclick_playlist_addmode->setCurrentIndex(
      ui_->doubleclick_playlist_addmode->findData(
          s.value("doubleclick_playlist_addmode",
                  MainWindow::PlaylistAddBehaviour_Play).toInt()));
  ui_->menu_playmode->setCurrentIndex(ui_->menu_playmode->findData(
      s.value("menu_playmode", MainWindow::PlayBehaviour_IfStopped).toInt()));

  MainWindow::StartupBehaviour behaviour = MainWindow::StartupBehaviour(
      s.value("startupbehaviour", MainWindow::Startup_Remember).toInt());
  switch (behaviour) {
    case MainWindow::Startup_AlwaysHide:
      ui_->b_always_hide_->setChecked(true);
      break;
    case MainWindow::Startup_AlwaysShow:
      ui_->b_always_show_->setChecked(true);
      break;
    case MainWindow::Startup_Remember:
      ui_->b_remember_->setChecked(true);
      break;
  }
  ui_->resume_after_start_->setChecked(
      s.value("resume_playback_after_start", false).toBool());
  s.endGroup();

  s.beginGroup(Player::kSettingsGroup);
  ui_->stop_play_if_fail_->setChecked(
      s.value("stop_play_if_fail", false).toBool());
  ui_->menu_previousmode->setCurrentIndex(ui_->menu_previousmode->findData(
      s.value("menu_previousmode", Player::PreviousBehaviour_DontRestart)
          .toInt()));
  ui_->seek_step_sec->setValue(s.value("seek_step_sec", 10).toInt());

  if (s.value("play_count_short_duration", false).toBool()) {
    ui_->b_play_count_short_duration->setChecked(true);
    ui_->b_play_count_normal_duration->setChecked(false);
  } else {
    ui_->b_play_count_short_duration->setChecked(false);
    ui_->b_play_count_normal_duration->setChecked(true);
  }
  s.endGroup();

  s.beginGroup("General");
  QString name = language_map_.key(s.value("language").toString());
  if (name.isEmpty())
    ui_->language->setCurrentIndex(0);
  else
    ui_->language->setCurrentIndex(ui_->language->findText(name));
  s.endGroup();

  s.beginGroup(Playlist::kSettingsGroup);
  ui_->b_grey_out_deleted_->setChecked(
      s.value("greyoutdeleted", false).toBool());
  ui_->b_click_edit_inline_->setChecked(
      s.value("click_edit_inline", true).toBool());

  Playlist::Path path = Playlist::Path(
      s.value(Playlist::kPathType, Playlist::Path_Automatic).toInt());
  switch (path) {
    case Playlist::Path_Automatic:
      ui_->b_automatic_path->setChecked(true);
      break;
    case Playlist::Path_Absolute:
      ui_->b_absolute_path->setChecked(true);
      break;
    case Playlist::Path_Relative:
      ui_->b_relative_path->setChecked(true);
      break;
    case Playlist::Path_Ask_User:
      ui_->b_ask_path->setChecked(true);
  }
  ui_->b_write_metadata->setChecked(
      s.value(Playlist::kWriteMetadata, true).toBool());

  ui_->sort_ignore_prefix->setChecked(
      s.value(Playlist::kSortIgnorePrefix, true).toBool());
  ui_->sort_ignore_prefix_list->setText(
      s.value(Playlist::kSortIgnorePrefixList, QString("a, the")).toString());
  s.endGroup();

  s.beginGroup(PlaylistTabBar::kSettingsGroup);
  ui_->b_warn_close_playlist_->setChecked(
      s.value("warn_close_playlist", true).toBool());
  s.endGroup();
}

void BehaviourSettingsPage::Save() {
  QSettings s;

  MainWindow::StartupBehaviour behaviour = MainWindow::Startup_Remember;
  if (ui_->b_always_hide_->isChecked())
    behaviour = MainWindow::Startup_AlwaysHide;
  if (ui_->b_always_show_->isChecked())
    behaviour = MainWindow::Startup_AlwaysShow;
  if (ui_->b_remember_->isChecked()) behaviour = MainWindow::Startup_Remember;

  MainWindow::AddBehaviour doubleclick_addmode = MainWindow::AddBehaviour(
      ui_->doubleclick_addmode->itemData(
                                    ui_->doubleclick_addmode->currentIndex())
          .toInt());
  MainWindow::PlayBehaviour doubleclick_playmode = MainWindow::PlayBehaviour(
      ui_->doubleclick_playmode->itemData(
                                     ui_->doubleclick_playmode->currentIndex())
          .toInt());
  MainWindow::PlaylistAddBehaviour doubleclick_playlist_addmode =
      MainWindow::PlaylistAddBehaviour(
          ui_->doubleclick_playlist_addmode
              ->itemData(ui_->doubleclick_playlist_addmode->currentIndex())
              .toInt());
  MainWindow::PlayBehaviour menu_playmode = MainWindow::PlayBehaviour(
      ui_->menu_playmode->itemData(ui_->menu_playmode->currentIndex()).toInt());

  Player::PreviousBehaviour menu_previousmode = Player::PreviousBehaviour(
      ui_->menu_previousmode->itemData(ui_->menu_previousmode->currentIndex())
          .toInt());

  Playlist::Path path = Playlist::Path_Automatic;
  if (ui_->b_automatic_path->isChecked()) {
    path = Playlist::Path_Automatic;
  } else if (ui_->b_absolute_path->isChecked()) {
    path = Playlist::Path_Absolute;
  } else if (ui_->b_relative_path->isChecked()) {
    path = Playlist::Path_Relative;
  } else if (ui_->b_ask_path->isChecked()) {
    path = Playlist::Path_Ask_User;
  }

  s.beginGroup(MainWindow::kSettingsGroup);
  s.setValue("showtray", ui_->b_show_tray_icon_->isChecked());
  s.setValue("scrolltrayicon", ui_->b_scroll_tray_icon_->isChecked());
  s.setValue("keeprunning", ui_->b_keep_running_->isChecked());
  s.setValue("startupbehaviour", int(behaviour));
  s.setValue("doubleclick_addmode", doubleclick_addmode);
  s.setValue("doubleclick_playmode", doubleclick_playmode);
  s.setValue("doubleclick_playlist_addmode", doubleclick_playlist_addmode);
  s.setValue("menu_playmode", menu_playmode);
  s.setValue("resume_playback_after_start",
             ui_->resume_after_start_->isChecked());
  s.endGroup();

  s.beginGroup(Player::kSettingsGroup);
  s.setValue("stop_play_if_fail",
             ui_->stop_play_if_fail_->isChecked());
  s.setValue("menu_previousmode", menu_previousmode);
  s.setValue("seek_step_sec", ui_->seek_step_sec->value());

  if (ui_->b_play_count_short_duration->isChecked()) {
    s.setValue("play_count_short_duration", true);
  } else {
    s.setValue("play_count_short_duration", false);
  }

  s.endGroup();

  s.beginGroup("General");
  s.setValue("language", language_map_.contains(ui_->language->currentText())
                             ? language_map_[ui_->language->currentText()]
                             : QString());
  s.endGroup();

  s.beginGroup(Playlist::kSettingsGroup);
  s.setValue("greyoutdeleted", ui_->b_grey_out_deleted_->isChecked());
  s.setValue("click_edit_inline", ui_->b_click_edit_inline_->isChecked());
  s.setValue(Playlist::kPathType, static_cast<int>(path));
  s.setValue(Playlist::kWriteMetadata, ui_->b_write_metadata->isChecked());
  s.setValue(Playlist::kSortIgnorePrefix, ui_->sort_ignore_prefix->isChecked());
  s.setValue(Playlist::kSortIgnorePrefixList,
             ui_->sort_ignore_prefix_list->text());
  s.endGroup();

  s.beginGroup(PlaylistTabBar::kSettingsGroup);
  s.setValue("warn_close_playlist", ui_->b_warn_close_playlist_->isChecked());
  s.endGroup();
}

void BehaviourSettingsPage::ShowTrayIconToggled(bool on) {
  ui_->b_always_hide_->setEnabled(on);
  if (!on && ui_->b_always_hide_->isChecked())
    ui_->b_remember_->setChecked(true);
  ui_->b_keep_running_->setEnabled(on);
  ui_->b_keep_running_->setChecked(on);
  ui_->b_scroll_tray_icon_->setEnabled(on);
}
