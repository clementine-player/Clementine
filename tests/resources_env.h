#ifndef RESOURCES_ENV_H
#define RESOURCES_ENV_H

#include <gtest/gtest.h>

#include <QResource>

class ResourcesEnvironment : public ::testing::Environment {
public:
  void SetUp() {
    Q_INIT_RESOURCE(data);
    Q_INIT_RESOURCE(translations);
  }
};

#endif // RESOURCES_ENV_H
