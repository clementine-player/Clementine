#include "closure.h"

#include "core/logging.h"

Closure::Closure(QObject* sender,
                 const char* signal,
                 QObject* receiver,
                 const char* slot,
                 const ClosureArgumentWrapper* val1,
                 const ClosureArgumentWrapper* val2)
    : QObject(receiver),
      val1_(val1),
      val2_(val2) {
  const QMetaObject* meta_receiver = receiver->metaObject();

  QByteArray normalised_slot = QMetaObject::normalizedSignature(slot + 1);
  slot_ = meta_receiver->method(
      meta_receiver->indexOfSlot(normalised_slot.constData()));

  connect(sender, signal, this, SLOT(Invoked()), Qt::AutoConnection);
}

void Closure::Invoked() {
  qLog(Debug) << "Invoking:" << slot_.signature() << val1_->arg().name();
  slot_.invoke(parent(), val1_->arg(), val2_->arg());

  delete val1_;
  delete val2_;
}
