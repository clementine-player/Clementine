#include "mock_librarybackend.h"

using ::testing::_;
using ::testing::Return;

void MockLibraryBackend::ExpectSetup(bool has_compilations,
                                     const QStringList& artists) {
  EXPECT_CALL(*this, LoadDirectoriesAsync());
  EXPECT_CALL(*this, UpdateTotalSongCountAsync());
  EXPECT_CALL(*this, HasCompilations(_))
      .WillOnce(Return(has_compilations));
  EXPECT_CALL(*this, GetAllArtists(_))
      .WillOnce(Return(artists));
}
