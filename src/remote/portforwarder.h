#ifndef PORTFORWARDER_H
#define PORTFORWARDER_H

#include <QObject>

#include "portfwd/portfwd.h"

class PortForwarder : public QObject {
  Q_OBJECT
 public:
  PortForwarder(QObject* parent = 0);
  void Init();
  void AddPortMapping(quint16 port);
  void RemovePortMapping(quint16 port);

 private:
  bool InitSync();
  bool AddPortMappingSync(quint16 port);
  bool RemovePortMappingSync(quint16 port);

 private slots:
  void InitFinished();
  void AddPortMappingFinished();
  void RemovePortMappingFinished();

 signals:
  void InitFinished(bool success);
  void PortMappingAdded(quint16 port);
  void PortMappingRemoved(quint16 port);

 private:
  Portfwd portfwd_;
};

#endif  // PORTFORWARDER_H
