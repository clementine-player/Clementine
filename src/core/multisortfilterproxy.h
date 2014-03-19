#ifndef MULTISORTFILTERPROXY_H
#define MULTISORTFILTERPROXY_H

#include <QSortFilterProxyModel>

class MultiSortFilterProxy : public QSortFilterProxyModel {
 public:
  MultiSortFilterProxy(QObject* parent = nullptr);

  void AddSortSpec(int role, Qt::SortOrder order = Qt::AscendingOrder);

 protected:
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

 private:
  int Compare(const QVariant& left, const QVariant& right) const;

  typedef QPair<int, Qt::SortOrder> SortSpec;
  QList<SortSpec> sorting_;
};

#endif  // MULTISORTFILTERPROXY_H
