#include "multisortfilterproxy.h"
#include "core/logging.h"

#include <QDate>
#include <QDateTime>
#include <QTime>

MultiSortFilterProxy::MultiSortFilterProxy(QObject* parent)
    : QSortFilterProxyModel(parent) {}

void MultiSortFilterProxy::AddSortSpec(int role, Qt::SortOrder order) {
  sorting_ << SortSpec(role, order);
}

bool MultiSortFilterProxy::lessThan(const QModelIndex& left,
                                    const QModelIndex& right) const {
  for (const SortSpec& spec : sorting_) {
    const int ret = Compare(left.data(spec.first), right.data(spec.first));

    if (ret < 0) {
      return spec.second == Qt::AscendingOrder;
    } else if (ret > 0) {
      return spec.second != Qt::AscendingOrder;
    }
  }

  return left.row() < right.row();
}

template <typename T>
static inline int DoCompare(T left, T right) {
  if (left < right) return -1;
  if (left > right) return 1;
  return 0;
}

int MultiSortFilterProxy::Compare(const QVariant& left,
                                  const QVariant& right) const {
  // Copied from the QSortFilterProxyModel::lessThan implementation, but returns
  // -1, 0 or 1 instead of true or false.
  switch (left.userType()) {
    case QVariant::Invalid:
      return (right.type() != QVariant::Invalid) ? -1 : 0;
    case QVariant::Int:
      return DoCompare(left.toInt(), right.toInt());
    case QVariant::UInt:
      return DoCompare(left.toUInt(), right.toUInt());
    case QVariant::LongLong:
      return DoCompare(left.toLongLong(), right.toLongLong());
    case QVariant::ULongLong:
      return DoCompare(left.toULongLong(), right.toULongLong());
    case QMetaType::Float:
      return DoCompare(left.toFloat(), right.toFloat());
    case QVariant::Double:
      return DoCompare(left.toDouble(), right.toDouble());
    case QVariant::Char:
      return DoCompare(left.toChar(), right.toChar());
    case QVariant::Date:
      return DoCompare(left.toDate(), right.toDate());
    case QVariant::Time:
      return DoCompare(left.toTime(), right.toTime());
    case QVariant::DateTime:
      return DoCompare(left.toDateTime(), right.toDateTime());
    case QVariant::String:
    default:
      if (isSortLocaleAware())
        return left.toString().localeAwareCompare(right.toString());
      else
        return left.toString().compare(right.toString(), sortCaseSensitivity());
  }

  return 0;
}
