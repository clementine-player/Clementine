#include <gmock/gmock.h>

#include "resources_env.h"
#include "metatypes_env.h"

int main(int argc, char** argv) {
  testing::InitGoogleMock(&argc, argv);

  testing::AddGlobalTestEnvironment(new ResourcesEnvironment);
  testing::AddGlobalTestEnvironment(new MetatypesEnvironment);

  return RUN_ALL_TESTS();
}
