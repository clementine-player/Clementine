/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "screenshottaker.h"

#include <gst/gst.h>

#include <QApplication>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFileInfo>
#include <QPixmap>
#include <QRegularExpression>
#include <QSettings>
#include <QTimer>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <algorithm>
#include <tuple>

#include "core/appearance.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "core/timeconstants.h"
#include "library/librarybackend.h"
#include "playlist/playlist.h"
#include "playlist/playlistmanager.h"
#include "ui/mainwindow.h"
#include "ui/settingsdialog.h"
#include "widgets/fancytabwidget.h"

namespace {

// The same size everywhere, so screenshots from different runs line up.
const QSize kWindowSize(1280, 800);
const QSize kSettingsSize(960, 680);

const int kLoadTimeoutMsec = 120000;
const int kPlayTimeoutMsec = 20000;
// How long a value has to stay put to count as settled.
const int kSettledMsec = 2000;
// Long enough for a tab or page to lay itself out and paint.
const int kPaintDelayMsec = 500;

}  // namespace

ScreenshotTaker::ScreenshotTaker(Application* app, MainWindow* window,
                                 const QString& dir, const QList<QUrl>& music,
                                 QObject* parent)
    : QObject(parent),
      app_(app),
      window_(window),
      dir_(dir),
      music_(music),
      failures_(0) {}

void ScreenshotTaker::UseSilentSink() {
  // fakesink plays as fast as it can, which would race through the playlist.
  // fakeaudiosink (GStreamer 1.20) plays in real time.
  gst_init(nullptr, nullptr);
  GstElementFactory* factory = gst_element_factory_find("fakeaudiosink");
  const char* sink = factory ? "fakeaudiosink" : "fakesink";
  if (factory) gst_object_unref(factory);

  QSettings s;
  s.beginGroup("GstEngine");
  s.setValue("sink", sink);
}

void ScreenshotTaker::Run() {
  if (!dir_.mkpath(".")) {
    qLog(Error) << "Couldn't create" << dir_.absolutePath();
    QCoreApplication::exit(1);
    return;
  }

  window_->showNormal();
  window_->resize(kWindowSize);

  if (!LoadMusic()) {
    QCoreApplication::exit(2);
    return;
  }
  PausePartWay();

  app_->appearance()->SetThemeMode(Appearance::ThemeMode_Light);
  TakeAll(QString());
  app_->appearance()->SetThemeMode(Appearance::ThemeMode_Dark);
  TakeAll("dark_");

  qLog(Info) << "Took the screenshots in" << dir_.absolutePath() << "with"
             << failures_ << "failures";
  QCoreApplication::exit(failures_ ? 1 : 0);
}

void ScreenshotTaker::Wait(int msec) {
  QEventLoop loop;
  QTimer::singleShot(msec, &loop, &QEventLoop::quit);
  loop.exec();
}

bool ScreenshotTaker::WaitFor(const char* what, int timeout_msec,
                              const std::function<bool()>& done) {
  QElapsedTimer timer;
  timer.start();
  while (!done()) {
    if (timer.elapsed() > timeout_msec) {
      qLog(Error) << "Timed out waiting for" << what;
      return false;
    }
    Wait(100);
  }
  return true;
}

bool ScreenshotTaker::WaitUntilSettled(const char* what, int timeout_msec,
                                       const std::function<int()>& value,
                                       const std::function<bool(int)>& ready) {
  int last = value();
  QElapsedTimer settled;
  settled.start();
  return WaitFor(what, timeout_msec, [&]() {
    const int current = value();
    if (current != last || !ready(current)) {
      last = current;
      settled.restart();
      return false;
    }
    return settled.elapsed() >= kSettledMsec;
  });
}

bool ScreenshotTaker::LoadMusic() {
  // MainWindow puts the music in the playlist itself; the library has to be
  // told about it.
  int directories = 0;
  for (const QUrl& url : music_) {
    if (url.isLocalFile() && QFileInfo(url.toLocalFile()).isDir()) {
      app_->library_backend()->AddDirectory(url.toLocalFile());
      ++directories;
    }
  }

  if (directories) {
    // Settled once the scan has finished and every song is in.
    const bool loaded = WaitUntilSettled(
        "the library scan", kLoadTimeoutMsec,
        [this]() { return app_->library_backend()->GetAllSongs().count(); },
        [this](int songs) {
          return songs > 0 && app_->task_manager()->GetTasks().isEmpty();
        });
    if (!loaded) {
      qLog(Error) << "The library has"
                  << app_->library_backend()->GetAllSongs().count() << "songs";
      for (const TaskManager::Task& task : app_->task_manager()->GetTasks()) {
        qLog(Error) << "Still running:" << task.name;
      }
      return false;
    }
  }

  Playlist* playlist = app_->playlist_manager()->active();
  if (!WaitUntilSettled(
          "the playlist", kLoadTimeoutMsec,
          [playlist]() { return playlist->rowCount(); },
          [](int rows) { return rows > 0; })) {
    return false;
  }
  if (!directories) return true;

  // Songs loaded from files don't have the covers the library found beside
  // them, so play the library's instead, in the order it lists them.
  SongList songs = app_->library_backend()->GetAllSongs();
  std::sort(songs.begin(), songs.end(), [](const Song& a, const Song& b) {
    return std::make_tuple(a.artist(), a.album(), a.disc(), a.track()) <
           std::make_tuple(b.artist(), b.album(), b.disc(), b.track());
  });
  playlist->Clear();
  playlist->InsertLibraryItems(songs);
  return true;
}

void ScreenshotTaker::PausePartWay() {
  // Playing matters less than the screenshots, so a song that won't play is
  // only a warning: the screenshots just won't show one playing.
  app_->player()->PlayAt(0, Engine::Manual, true);
  if (!WaitFor("the first song to play", kPlayTimeoutMsec, [this]() {
        return app_->player()->GetState() == Engine::Playing;
      })) {
    return;
  }

  PlaylistItemPtr item = app_->player()->GetCurrentItem();
  if (item)
    app_->player()->SeekTo(item->Metadata().length_nanosec() / kNsecPerSec / 3);
  Wait(1000);
  app_->player()->Pause();
  WaitFor("the song to pause", kPlayTimeoutMsec,
          [this]() { return app_->player()->GetState() == Engine::Paused; });
  // For the cover to load.
  Wait(1000);
}

void ScreenshotTaker::TakeAll(const QString& prefix) {
  Wait(kPaintDelayMsec);
  TakeMainWindow(prefix);
  TakeSettings(prefix);
}

void ScreenshotTaker::TakeMainWindow(const QString& prefix) {
  FancyTabWidget* tabs = window_->findChild<FancyTabWidget*>();
  if (!tabs) {
    qLog(Error) << "The main window has no tabs";
    ++failures_;
    return;
  }

  int n = 0;
  for (int i = 0; i < tabs->count(); ++i) {
    // The spacer between the groups of tabs has no name.
    if (!tabs->isTabVisible(i) || tabs->tabText(i).isEmpty()) continue;
    tabs->setCurrentIndex(i);
    Wait(kPaintDelayMsec);
    Save(window_, QString("%1main-%2-%3")
                      .arg(prefix)
                      .arg(++n, 2, 10, QChar('0'))
                      .arg(Slug(tabs->tabText(i))));
  }
  tabs->setCurrentIndex(0);
}

void ScreenshotTaker::TakeSettings(const QString& prefix) {
  // A private slot, but there's no public way to open the dialog.
  QMetaObject::invokeMethod(window_, "OpenSettingsDialog",
                            Qt::DirectConnection);
  SettingsDialog* dialog = nullptr;
  for (QWidget* widget : QApplication::topLevelWidgets()) {
    dialog = qobject_cast<SettingsDialog*>(widget);
    if (dialog) break;
  }
  QTreeWidget* list =
      dialog ? dialog->findChild<QTreeWidget*>("list") : nullptr;
  if (!list) {
    qLog(Error) << "Couldn't find the settings dialog's pages";
    ++failures_;
    return;
  }
  dialog->resize(kSettingsSize);

  int n = 0;
  for (QTreeWidgetItemIterator it(list); *it; ++it) {
    QTreeWidgetItem* item = *it;
    // The section headings aren't pages.
    if (item->isHidden() || !(item->flags() & Qt::ItemIsSelectable)) continue;
    list->setCurrentItem(item);
    list->scrollToItem(item);
    Wait(kPaintDelayMsec);
    Save(dialog, QString("%1settings-%2-%3")
                     .arg(prefix)
                     .arg(++n, 2, 10, QChar('0'))
                     .arg(Slug(item->text(0))));
  }
  dialog->reject();
}

void ScreenshotTaker::Save(QWidget* widget, const QString& name) {
  const QString path = dir_.filePath(name + ".png");
  if (widget->grab().save(path, "PNG")) {
    qLog(Info) << "Saved" << path;
  } else {
    qLog(Error) << "Couldn't save" << path;
    ++failures_;
  }
}

QString ScreenshotTaker::Slug(const QString& text) {
  QString slug = text.toLower();
  slug.remove('&');  // Mnemonics
  slug.replace(QRegularExpression("[^a-z0-9]+"), "-");
  return slug.remove(QRegularExpression("^-+|-+$"));
}
