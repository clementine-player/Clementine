class MergedProxyModel : QAbstractProxyModel {

%TypeHeaderCode
#include "core/mergedproxymodel.h"
%End

public:
  MergedProxyModel(QObject* parent = 0);

  void AddSubModel(const QModelIndex& source_parent, QAbstractItemModel* submodel);
  void RemoveSubModel(const QModelIndex& source_parent);

  QModelIndex FindSourceParent(const QModelIndex& proxy_index) const;

signals:
  void SubModelReset(const QModelIndex& root, QAbstractItemModel* model);
};
