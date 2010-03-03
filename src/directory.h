#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QList>
#include <QString>
#include <QMetaType>

class QSqlQuery;

struct Directory {
  QString path;
  int id;
};
Q_DECLARE_METATYPE(Directory);

typedef QList<Directory> DirectoryList;
Q_DECLARE_METATYPE(DirectoryList);

#endif // DIRECTORY_H
