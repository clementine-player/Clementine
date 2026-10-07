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

#include "widgets/trackslider.h"

#include <QEnterEvent>
#include <QHBoxLayout>
#include <QSignalSpy>
#include <QSlider>
#include <QApplication>
#include <QTest>
#include <QWindow>

#include "gtest/gtest.h"
#include "test_utils.h"
#include "widgets/tracksliderpopup.h"

namespace {

class TrackSliderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    slider_.resize(400, slider_.sizeHint().height());
    slider_.show();
    ASSERT_TRUE(QTest::qWaitForWindowExposed(&slider_));

    // Two minutes in to a ten minute song.
    slider_.SetValue(2 * 60 * 1000, 10 * 60 * 1000);
    bar_ = slider_.findChild<QSlider*>();
    ASSERT_NE(nullptr, bar_);
  }

  QPoint AtFraction(double fraction) const {
    return QPoint(int(bar_->width() * fraction), bar_->height() / 2);
  }

  TrackSlider slider_;
  QSlider* bar_ = nullptr;
};

TEST_F(TrackSliderTest, ClickSeeks) {
  QSignalSpy spy(&slider_, SIGNAL(ValueChangedSeconds(int)));

  QTest::mouseClick(bar_, Qt::LeftButton, Qt::NoModifier, AtFraction(0.75));

  ASSERT_FALSE(spy.isEmpty());
  EXPECT_GT(spy.last().at(0).toInt(), 2 * 60);
}

TEST_F(TrackSliderTest, DragSeeks) {
  QSignalSpy spy(&slider_, SIGNAL(ValueChangedSeconds(int)));

  QTest::mousePress(bar_, Qt::LeftButton, Qt::NoModifier, AtFraction(0.2));
  QTest::mouseMove(bar_, AtFraction(0.5));
  QTest::mouseMove(bar_, AtFraction(0.8));
  QTest::mouseRelease(bar_, Qt::LeftButton, Qt::NoModifier, AtFraction(0.8));

  ASSERT_FALSE(spy.isEmpty());
  EXPECT_GT(spy.last().at(0).toInt(), 5 * 60);
}

TEST_F(TrackSliderTest, KeepsPositionWhileDragging) {
  QSignalSpy spy(&slider_, SIGNAL(ValueChangedSeconds(int)));

  QTest::mousePress(bar_, Qt::LeftButton, Qt::NoModifier, AtFraction(0.2));
  QTest::mouseMove(bar_, AtFraction(0.8));
  // The player's position updates mustn't move the slider under the mouse.
  slider_.SetValue(2 * 60 * 1000, 10 * 60 * 1000);
  QTest::mouseRelease(bar_, Qt::LeftButton, Qt::NoModifier, AtFraction(0.8));

  ASSERT_FALSE(spy.isEmpty());
  EXPECT_GT(spy.last().at(0).toInt(), 5 * 60);
}

// On macOS a Cocoa search field gives its siblings, up to the window's
// children, native windows - the hover popup's too. The popup sits over the
// slider, so its window has to let clicks through.
TEST(TrackSliderPopupTest, NativePopupLetsClicksThrough) {
  QWidget window;
  QHBoxLayout* layout = new QHBoxLayout(&window);
  QWidget* native = new QWidget;
  native->setAttribute(Qt::WA_NativeWindow);
  layout->addWidget(native);
  TrackSlider* slider = new TrackSlider(&window);
  layout->addWidget(slider);
  window.resize(400, 60);
  window.show();
  ASSERT_TRUE(QTest::qWaitForWindowExposed(&window));
  slider->SetValue(2 * 60 * 1000, 10 * 60 * 1000);

  // Hover over the slider to show the popup.
  QSlider* bar = slider->findChild<QSlider*>();
  ASSERT_NE(nullptr, bar);
  const QPoint at = bar->rect().center();
  QEnterEvent enter(at, bar->mapTo(&window, at), bar->mapToGlobal(at));
  QApplication::sendEvent(bar, &enter);

  TrackSliderPopup* popup = window.findChild<TrackSliderPopup*>();
  ASSERT_NE(nullptr, popup);
  ASSERT_TRUE(popup->isVisible());
  ASSERT_NE(nullptr, popup->windowHandle());
  EXPECT_TRUE(popup->windowHandle()->flags() & Qt::WindowTransparentForInput);
}

}  // namespace
