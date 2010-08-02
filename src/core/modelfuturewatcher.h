#ifndef MODELFUTUREWATCHER_H
#define MODELFUTUREWATCHER_H

#include <QFutureWatcher>
#include <QModelIndex>

template <typename T>
class ModelFutureWatcher : public QFutureWatcher<T> {
 public:
  ModelFutureWatcher(const QModelIndex& index, QObject* parent = 0)
      : QFutureWatcher<T>(parent),
        index_(index) {
  }

  ~ModelFutureWatcher() {
  }

  const QModelIndex& index() const { return index_; }

 private:
  QModelIndex index_;

};

#endif
