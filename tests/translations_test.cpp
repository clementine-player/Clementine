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

#include <gtest/gtest.h>

#include <QFile>

#include "core/potranslator.h"
#include "test_utils.h"

TEST(Translations, Basic) {
  ASSERT_TRUE(QFile::exists(":/translations"));
  ASSERT_TRUE(QFile::exists(":/translations/clementine_es.qm"));

  PoTranslator t;
  t.load("clementine_es.qm", ":/translations");

  EXPECT_EQ(QString::fromUtf8("Fonoteca"),
            t.translate("MainWindow", "Library"));
  EXPECT_EQ(QString::fromUtf8("Fonoteca"),
            t.translate("", "Library"));
}
