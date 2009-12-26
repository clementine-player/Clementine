#ifndef RADIOMODEL_H
#define RADIOMODEL_H

#include "radioitem.h"
#include "simpletreemodel.h"

class RadioService;

class RadioModel : public SimpleTreeModel<RadioItem> {
  Q_OBJECT

 public:
  RadioModel(QObject* parent = 0);

  enum {
    Role_Type = Qt::UserRole + 1,
    Role_SortText,
    Role_Key,
  };

  // Needs to be static for RadioPlaylistItem::restore
  static RadioService* ServiceByName(const QString& name);

  // QAbstractItemModel
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  QStringList mimeTypes() const;
  QMimeData* mimeData(const QModelIndexList& indexes) const;

 signals:
  void LoadingStarted();
  void LoadingFinished();
  void StreamReady(const QUrl& original_url, const QUrl& media_url);
  void StreamFinished();
  void StreamError(const QString& message);

 protected:
  void LazyPopulate(RadioItem* parent);

 private:
  QVariant data(const RadioItem* item, int role) const;
  void AddService(RadioService* service);

 private:
  static QMap<QString, RadioService*> sServices;
};

#endif // RADIOMODEL_H
