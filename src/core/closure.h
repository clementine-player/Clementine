#ifndef CLOSURE_H
#define CLOSURE_H

#include <QMetaMethod>
#include <QObject>

#include "core/logging.h"

class ClosureArgumentWrapper {
 public:
  virtual ~ClosureArgumentWrapper() {}

  virtual QGenericArgument arg() const {
    qLog(Debug) << Q_FUNC_INFO;
    return QGenericArgument();
  }
};

template<typename T>
class ClosureArgument : public ClosureArgumentWrapper {
 public:
  ClosureArgument(const T& data) : data_(data) {}

  virtual QGenericArgument arg() const {
    qLog(Debug) << Q_FUNC_INFO;
    return Q_ARG(T, data_);
  }

 private:
  T data_;
};

class Closure : public QObject {
  Q_OBJECT

 public:
  Closure(QObject* sender, const char* signal,
          QObject* receiver, const char* slot,
          const ClosureArgumentWrapper* val1,
          const ClosureArgumentWrapper* val2);

 private slots:
  void Invoked();

 private:
  QMetaMethod slot_;

  const ClosureArgumentWrapper* val1_;
  const ClosureArgumentWrapper* val2_;
};

#define C_ARG(type, data) new ClosureArgument<type>(data)

#endif  // CLOSURE_H
