#ifndef RADIOSERVICE_H
#define RADIOSERVICE_H

#include <QObject>

class RadioItem;

class RadioService : public QObject {
  Q_OBJECT

 public:
  RadioService(const QString& name, QObject* parent = 0);

  QString name() const { return name_; }

  virtual RadioItem* CreateRootItem(RadioItem* parent) = 0;
  virtual void LazyPopulate(RadioItem* item) = 0;

 private:
  QString name_;
};

#endif // RADIOSERVICE_H
