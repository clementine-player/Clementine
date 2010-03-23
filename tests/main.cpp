#include <gmock/gmock.h>

#include <QApplication>

#include "resources_env.h"
#include "metatypes_env.h"

int main(int argc, char** argv) {
  testing::InitGoogleMock(&argc, argv);

  testing::AddGlobalTestEnvironment(new MetatypesEnvironment);
  QApplication a(argc, argv);
  testing::AddGlobalTestEnvironment(new ResourcesEnvironment);

  return RUN_ALL_TESTS();
}
