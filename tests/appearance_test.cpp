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

#include "core/appearance.h"

#include <QApplication>
#include <QTemporaryFile>
#include <QTest>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>

#include "analyzers/blockanalyzer.h"
#include "core/stylesheetloader.h"
#include "gtest/gtest.h"

namespace {

// Switching the theme while Clementine runs has to repaint what it started
// with, as starting in that theme would.
class AppearanceTest : public ::testing::Test {
 protected:
  void SetUp() override {
    appearance_.SetThemeMode(Appearance::ThemeMode_Light);
    window_.resize(400, 300);
  }

  void TearDown() override {
    appearance_.SetThemeMode(Appearance::ThemeMode_Light);
  }

  void SwitchTo(Appearance::ThemeMode mode) {
    appearance_.SetThemeMode(mode);
    QCoreApplication::processEvents();
  }

  Appearance appearance_;
  QWidget window_;
};

TEST_F(AppearanceTest, StyleSheetFollowsTheTheme) {
  // The playlist's colours come from mainwindow.css like this.
  QTemporaryFile css;
  ASSERT_TRUE(css.open());
  css.write("#playlist { background-color: %palette-base; }");
  css.flush();

  QWidget* playlist = new QWidget(&window_);
  playlist->setObjectName("playlist");
  StyleSheetLoader loader;
  loader.SetStyleSheet(&window_, css.fileName());
  window_.show();

  const QString dark_base =
      Appearance::DarkPalette().color(QPalette::Base).name();
  EXPECT_FALSE(window_.styleSheet().contains(dark_base));

  SwitchTo(Appearance::ThemeMode_Dark);
  EXPECT_TRUE(window_.styleSheet().contains(dark_base))
      << window_.styleSheet().toStdString();

  SwitchTo(Appearance::ThemeMode_Light);
  EXPECT_FALSE(window_.styleSheet().contains(dark_base))
      << window_.styleSheet().toStdString();
}

TEST_F(AppearanceTest, AnalyzerFollowsTheTheme) {
  QVBoxLayout* layout = new QVBoxLayout(&window_);
  // With no engine, it draws its demo.
  BlockAnalyzer* analyzer = new BlockAnalyzer(&window_);
  layout->addWidget(analyzer);
  window_.show();
  ASSERT_TRUE(QTest::qWaitForWindowExposed(&window_));

  // The gaps between the blocks are the window's colour.
  const std::string dark_window =
      Appearance::DarkPalette().color(QPalette::Window).name().toStdString();
  auto gap = [analyzer]() {
    return analyzer->grab().toImage().pixelColor(0, 0).name().toStdString();
  };
  EXPECT_NE(dark_window, gap());

  SwitchTo(Appearance::ThemeMode_Dark);
  EXPECT_EQ(dark_window, gap());
}

}  // namespace
