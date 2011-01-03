struct Directory {

%TypeHeaderCode
#include "library/directory.h"
%End

  Directory();

  QString path;
  int id;
};

struct Subdirectory {

%TypeHeaderCode
#include "library/directory.h"
%End

  Subdirectory();

  int directory_id;
  QString path;
  uint mtime;
};

typedef QList<Directory> DirectoryList;
typedef QList<Subdirectory> SubdirectoryList;

