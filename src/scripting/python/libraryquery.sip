class QueryOptions {

%TypeHeaderCode
#include "library/libraryquery.h"
%End

public:
  enum QueryMode {
    QueryMode_All,
    QueryMode_Duplicates,
    QueryMode_Untagged
  };

  QueryOptions();

  bool Matches(const Song& song) const;

  QString filter() const;
  void set_filter(const QString& filter);

  int max_age() const;
  void set_max_age(int max_age);

  QueryMode query_mode() const;
  void set_query_mode(QueryMode view);
};

class LibraryQuery {

%TypeHeaderCode
#include "library/libraryquery.h"
%End

public:
  LibraryQuery(); // Workaround SIP Mercurial 3e647ed0f2a2
  LibraryQuery(const QueryOptions& options);

  void SetColumnSpec(const QString& spec);
  void SetOrderBy(const QString& order_by);

  void AddWhere(const QString& column, const QVariant& value, const QString& op = "=");

  void AddCompilationRequirement(bool compilation);
  void SetLimit(int limit);

  bool Next();
  QVariant Value(int column) const;
};
