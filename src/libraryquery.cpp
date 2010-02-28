#include "libraryquery.h"
#include "song.h"

#include <QtDebug>
#include <QDateTime>

LibraryQuery::LibraryQuery()
{
}

LibraryQuery::LibraryQuery(const QueryOptions& options)
{
  if (!options.filter.isEmpty()) {
    where_clauses_ << "("
        "artist LIKE ? OR "
        "album LIKE ? OR "
        "title LIKE ?)";
    bound_values_ << "%" + options.filter + "%";
    bound_values_ << "%" + options.filter + "%";
    bound_values_ << "%" + options.filter + "%";
  }

  if (options.max_age != -1) {
    int cutoff = QDateTime::currentDateTime().toTime_t() - options.max_age;

    where_clauses_ << "ctime > ?";
    bound_values_ << cutoff;
  }
}

void LibraryQuery::AddWhere(const QString& column, const QVariant& value) {
  // Do integers inline - sqlite seems to get confused when you pass integers
  // to bound parameters

  if (value.type() == QVariant::Int)
    where_clauses_ << QString("%1 = %2").arg(column, value.toString());
  else {
    where_clauses_ << QString("%1 = ?").arg(column);
    bound_values_ << value;
  }
}

void LibraryQuery::AddCompilationRequirement(bool compilation) {
  if (compilation)
    where_clauses_ << "(compilation = 1 OR sampler = 1)";
  else
    where_clauses_ << "(compilation = 0 AND sampler = 0)";
}

QSqlQuery LibraryQuery::Query(QSqlDatabase db) const {
  QString sql = QString("SELECT %1 FROM songs").arg(column_spec_);

  if (!where_clauses_.isEmpty())
    sql += " WHERE " + where_clauses_.join(" AND ");

  if (!order_by_.isEmpty())
    sql += " ORDER BY " + order_by_;

  QSqlQuery q(sql, db);

  // Bind values
  foreach (const QVariant& value, bound_values_) {
    q.addBindValue(value);
  }

  return q;
}

bool QueryOptions::Matches(const Song& song) const {
  if (max_age != -1) {
    const uint cutoff = QDateTime::currentDateTime().toTime_t() - max_age;
    if (song.ctime() <= cutoff)
      return false;
  }

  if (!filter.isNull()) {
    return song.artist().contains(filter, Qt::CaseInsensitive) ||
           song.album().contains(filter, Qt::CaseInsensitive) ||
           song.title().contains(filter, Qt::CaseInsensitive);
  }

  return true;
}
