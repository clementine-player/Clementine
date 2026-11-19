/* This file is part of Clementine.

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

#include "core/crashreporting.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include "gtest/gtest.h"

namespace {

class CrashReportingTest : public ::testing::Test {
 protected:
  void SetUp() override { ASSERT_TRUE(dir_.isValid()); }

  // Saves a minidump where Crashpad would, in subdir of the database.
  void SaveMinidump(const QString& subdir, const QString& name) {
    ASSERT_TRUE(QDir(dir_.path()).mkpath(subdir));
    QFile file(dir_.path() + "/" + subdir + "/" + name);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
  }

  // The file names of the pending reports of the given type.
  QStringList Pending(CrashReporting::CrashType type) const {
    QStringList names;
    for (const CrashReporting::PendingReport& report :
         CrashReporting::PendingReports(dir_.path())) {
      if (report.type == type) {
        names << QFileInfo(report.minidump_path).fileName();
      }
    }
    return names;
  }

  QTemporaryDir dir_;
};

TEST_F(CrashReportingTest, CollectsUnexpectedCrashes) {
  SaveMinidump("pending", "a.dmp");
  SaveMinidump("completed", "b.dmp");
  SaveMinidump("reports", "c.dmp");

  CrashReporting::CollectNewReports(dir_.path());

  EXPECT_EQ(QStringList() << "a.dmp" << "b.dmp" << "c.dmp",
            Pending(CrashReporting::CrashType::kUnexpected));
  EXPECT_TRUE(Pending(CrashReporting::CrashType::kForced).isEmpty());
  EXPECT_FALSE(QFile::exists(dir_.path() + "/pending/a.dmp"));
}

TEST_F(CrashReportingTest, KeepsForcedCrashesApart) {
  SaveMinidump("pending", "unexpected.dmp");
  CrashReporting::CollectNewReports(dir_.path());

  CrashReporting::MarkNextCrashForced(dir_.path());
  SaveMinidump("pending", "forced.dmp");
  CrashReporting::CollectNewReports(dir_.path());

  EXPECT_EQ(QStringList() << "unexpected.dmp",
            Pending(CrashReporting::CrashType::kUnexpected));
  EXPECT_EQ(QStringList() << "forced.dmp",
            Pending(CrashReporting::CrashType::kForced));
}

TEST_F(CrashReportingTest, ForcedMarkLastsOneStart) {
  CrashReporting::MarkNextCrashForced(dir_.path());
  // The forced crash didn't save a report, for whatever reason.
  CrashReporting::CollectNewReports(dir_.path());

  SaveMinidump("pending", "later.dmp");
  CrashReporting::CollectNewReports(dir_.path());

  EXPECT_EQ(QStringList() << "later.dmp",
            Pending(CrashReporting::CrashType::kUnexpected));
  EXPECT_TRUE(Pending(CrashReporting::CrashType::kForced).isEmpty());
}

}  // namespace
