#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QList>
#include <QString>

class QSqlQuery;

struct Directory {
  QString path;
  int id;
};

typedef QList<Directory> DirectoryList;

#endif // DIRECTORY_H
