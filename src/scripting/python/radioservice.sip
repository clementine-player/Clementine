class RadioService : QObject {

%TypeHeaderCode
#include "radio/radioservice.h"
%End

public:
  RadioService(const QString& name, RadioModel* model);

  QString name() const;
  RadioModel* model() const;

  virtual QStandardItem* CreateRootItem() = 0 /TransferBack/;
  virtual void LazyPopulate(QStandardItem* parent) = 0;

  virtual void ShowContextMenu(const QModelIndex& index, const QPoint& global_pos);

  virtual PlaylistItem::SpecialLoadResult StartLoading(const QUrl& url);
  virtual PlaylistItem::SpecialLoadResult LoadNext(const QUrl& url);

  virtual PlaylistItem::Options playlistitem_options() const;

  virtual QWidget* HeaderWidget() const /Transfer/;

  virtual void ReloadSettings();

  virtual QString Icon();

signals:
  void AsyncLoadFinished(const PlaylistItem::SpecialLoadResult& result);
  void StreamError(const QString& message);
  void StreamMetadataFound(const QUrl& original_url, const Song& song);
  void OpenSettingsAtPage(SettingsDialog::Page page);

  void AddToPlaylistSignal(QMimeData* data);

protected:
  void AddItemToPlaylist(const QModelIndex& index, bool clear = false, bool enqueue = false);
  void AddItemsToPlaylist(const QModelIndexList& indexes, bool clear = false, bool enqueue = false);
};
