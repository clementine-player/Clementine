#include "portforwarder.h"

#include <QtConcurrentRun>

#include "core/boundfuturewatcher.h"

PortForwarder::PortForwarder(QObject* parent)
    : QObject(parent) {
}

void PortForwarder::Init() {
  QFuture<bool> future = QtConcurrent::run(this, &PortForwarder::InitSync);
  QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>(this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(InitFinished()));
}

bool PortForwarder::InitSync() {
  portfwd_.init(10000);
  return true;
}

void PortForwarder::InitFinished() {
  QFutureWatcher<bool>* watcher = dynamic_cast<QFutureWatcher<bool>*>(sender());
  Q_ASSERT(watcher);
  watcher->deleteLater();

  emit(InitFinished(watcher->result()));
}

void PortForwarder::AddPortMapping(quint16 port) {
  QFuture<bool> future = QtConcurrent::run(
      this, &PortForwarder::AddPortMappingSync, port);
  BoundFutureWatcher<bool, quint16>* watcher =
      new BoundFutureWatcher<bool, quint16>(port, this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(AddPortMappingFinished()));
}

bool PortForwarder::AddPortMappingSync(quint16 port) {
  return portfwd_.add(port);
}

void PortForwarder::AddPortMappingFinished() {
  BoundFutureWatcher<bool, quint16>* watcher =
      dynamic_cast<BoundFutureWatcher<bool, quint16>*>(sender());
  Q_ASSERT(watcher);
  watcher->deleteLater();

  if (watcher->result())
    emit(PortMappingAdded(watcher->data()));
}

void PortForwarder::RemovePortMapping(quint16 port) {
  QFuture<bool> future = QtConcurrent::run(
      this, &PortForwarder::RemovePortMappingSync, port);
  BoundFutureWatcher<bool, quint16>* watcher =
      new BoundFutureWatcher<bool, quint16>(port, this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(RemovePortMappingFinished()));
}

bool PortForwarder::RemovePortMappingSync(quint16 port) {
  return portfwd_.remove(port);
}

void PortForwarder::RemovePortMappingFinished() {
  BoundFutureWatcher<bool, quint16>* watcher =
      dynamic_cast<BoundFutureWatcher<bool, quint16>*>(sender());
  Q_ASSERT(watcher);
  watcher->deleteLater();

  if (watcher->result())
    emit(PortMappingRemoved(watcher->data()));
}
