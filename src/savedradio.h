#ifndef SAVEDRADIO_H
#define SAVEDRADIO_H

#include "radioservice.h"

class QMenu;

class SavedRadio : public RadioService {
  Q_OBJECT

 public:
  SavedRadio(QObject* parent = 0);
  ~SavedRadio();

  enum ItemType {
    Type_Stream = 2000,
  };

  static const char* kServiceName;
  static const char* kSettingsGroup;

  RadioItem* CreateRootItem(RadioItem* parent);
  void LazyPopulate(RadioItem* item);

  void ShowContextMenu(RadioItem* item, const QPoint& global_pos);

  void StartLoading(const QUrl& url);

  void Add(const QUrl& url);

 signals:
  void ShowAddStreamDialog();

 private slots:
  void AddToPlaylist();
  void Remove();

 private:
  void LoadStreams();
  void SaveStreams();
  RadioItem* ItemForStream(const QUrl& url, RadioItem* parent);

 private:
  RadioItem* root_;
  QMenu* context_menu_;
  RadioItem* context_item_;

  QAction* add_action_;
  QAction* remove_action_;

  QStringList streams_;
};

#endif // SAVEDRADIO_H
