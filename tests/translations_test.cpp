#include <gtest/gtest.h>

#include <QFile>
#include <QTranslator>

#include "test_utils.h"

TEST(Translations, Basic) {
  ASSERT_TRUE(QFile::exists(":/translations"));
  ASSERT_TRUE(QFile::exists(":/translations/clementine_empty.qm"));
  ASSERT_TRUE(QFile::exists(":/translations/clementine_es.qm"));

  QTranslator t;
  t.load("clementine_es.qm", ":/translations");

  EXPECT_EQ(QString::fromUtf8("Colección"),
            t.translate("MainWindow", "Library"));
}
