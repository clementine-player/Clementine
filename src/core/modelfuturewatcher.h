#ifndef MODELFUTUREWATCHER_H
#define MODELFUTUREWATCHER_H

#include <QFutureWatcher>
#include <QPersistentModelIndex>

template <typename T>
class ModelFutureWatcher : public QFutureWatcher<T> {
 public:
  ModelFutureWatcher(const QModelIndex& index, QObject* parent = 0)
      : QFutureWatcher<T>(parent), index_(index) {}

  ~ModelFutureWatcher() {}

  const QPersistentModelIndex& index() const { return index_; }

 private:
  QPersistentModelIndex index_;
};

#endif
