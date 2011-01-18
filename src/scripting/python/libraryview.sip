class LibraryView : AutoExpandingTreeView {

%TypeHeaderCode
#include "library/libraryview.h"
%End

public:
  SongList GetSelectedSongs() const;

};
