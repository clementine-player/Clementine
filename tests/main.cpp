#include <gmock/gmock.h>

#include "resources_env.h"

int main(int argc, char** argv) {
  testing::InitGoogleMock(&argc, argv);

  testing::AddGlobalTestEnvironment(new ResourcesEnvironment);

  return RUN_ALL_TESTS();
}
