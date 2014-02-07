#ifndef BOUNDFUTUREWATCHER_H
#define BOUNDFUTUREWATCHER_H

#include <QFutureWatcher>

#include <boost/noncopyable.hpp>

template <typename T, typename D>
class BoundFutureWatcher : public QFutureWatcher<T>, boost::noncopyable {
 public:
  BoundFutureWatcher(const D& data, QObject* parent = 0)
      : QFutureWatcher<T>(parent), data_(data) {}

  ~BoundFutureWatcher() {}

  const D& data() const { return data_; }

 private:
  D data_;
};

#endif
