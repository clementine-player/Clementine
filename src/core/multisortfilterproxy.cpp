#include "logging.h"
#include "multisortfilterproxy.h"

#include <QDate>
#include <QDateTime>
#include <QTime>

MultiSortFilterProxy::MultiSortFilterProxy(QObject* parent)
  : QSortFilterProxyModel(parent)
{
}

void MultiSortFilterProxy::AddSortSpec(int role, Qt::SortOrder order) {
  sorting_ << SortSpec(role, order);
}

bool MultiSortFilterProxy::lessThan(const QModelIndex& left,
                                    const QModelIndex& right) const {
  foreach (const SortSpec& spec, sorting_) {
    const int ret = compare(left.data(spec.first), right.data(spec.first));

    if (ret < 0) {
      return spec.second == Qt::AscendingOrder;
    } else if (ret > 0) {
      return spec.second != Qt::AscendingOrder;
    }
  }

  return left.row() < right.row();
}

int MultiSortFilterProxy::compare(const QVariant& left, const QVariant& right) const {
  // Copied from the QSortFilterProxyModel::lessThan implementation, but returns
  // -1, 0 or 1 instead of true or false.

  #define docompare(left, right) \
    if (left < right) return -1; \
    if (left > right) return 1; \
    return 0;

  switch (left.userType()) {
    case QVariant::Invalid:
      return (right.type() != QVariant::Invalid) ? -1 : 0;
    case QVariant::Int:       docompare(left.toInt(), right.toInt());
    case QVariant::UInt:      docompare(left.toUInt(), right.toUInt());
    case QVariant::LongLong:  docompare(left.toLongLong(), right.toLongLong());
    case QVariant::ULongLong: docompare(left.toULongLong(), right.toULongLong());
    case QMetaType::Float:    docompare(left.toFloat(), right.toFloat());
    case QVariant::Double:    docompare(left.toDouble(), right.toDouble());
    case QVariant::Char:      docompare(left.toChar(), right.toChar());
    case QVariant::Date:      docompare(left.toDate(), right.toDate());
    case QVariant::Time:      docompare(left.toTime(), right.toTime());
    case QVariant::DateTime:  docompare(left.toDateTime(), right.toDateTime());
    case QVariant::String:
    default:
      if (isSortLocaleAware())
        return left.toString().localeAwareCompare(right.toString());
      else
        return left.toString().compare(right.toString(), sortCaseSensitivity());
  }

  #undef docompare

  return 0;
}
