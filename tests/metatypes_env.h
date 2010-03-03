#ifndef METATYPES_ENV_H
#define METATYPES_ENV_H

#include <gtest/gtest.h>

#include <QMetaType>

#include "directory.h"
#include "song.h"

class MetatypesEnvironment : public ::testing::Environment {
public:
  void SetUp() {
    qRegisterMetaType<DirectoryList>("DirectoryList");
    qRegisterMetaType<SongList>("SongList");
  }
};

#endif // RESOURCES_ENV_H
