#ifndef PLAYLISTLISTMODEL_H
#define PLAYLISTLISTMODEL_H

#include <QStandardItemModel>

class PlaylistListModel : public QStandardItemModel {
  Q_OBJECT

public:
  PlaylistListModel(QObject* parent = 0);

  enum Types {
    Type_Folder,
    Type_Playlist
  };

  enum Roles {
    Role_Type = Qt::UserRole,
    Role_PlaylistId
  };

  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
                    int column, const QModelIndex& parent);

  // These icons will be used for newly created playlists and folders.
  // The caller will need to set these icons on existing items if there are any.
  void SetIcons(const QIcon& playlist_icon, const QIcon& folder_icon);
  const QIcon& playlist_icon() const { return playlist_icon_; }
  const QIcon& folder_icon() const { return folder_icon_; }

  // Walks from the given item to the root, returning the / separated path of
  // all the parent folders.  The path includes this item if it is a folder.
  QString ItemPath(const QStandardItem* item) const;

  // Finds the playlist with the given ID, returns 0 if it doesn't exist.
  QStandardItem* PlaylistById(int id) const;

  // Finds the folder with the given path, creating it (and its parents) if they
  // do not exist.  Returns invisibleRootItem() if path is empty.
  QStandardItem* FolderByPath(const QString& path);

  // Returns a new folder item with the given name.  The item isn't added to
  // the model yet.
  QStandardItem* NewFolder(const QString& name) const;

  // Returns a new playlist item with the given name and ID.  The item isn't
  // added to the model yet.
  QStandardItem* NewPlaylist(const QString& name, int id) const;

  // QStandardItemModel
  bool setData(const QModelIndex& index, const QVariant& value, int role);

signals:
  void PlaylistPathChanged(int id, const QString& new_path);
  void PlaylistRenamed(int id, const QString& new_name);

private slots:
  void RowsChanged(const QModelIndex& begin, const QModelIndex& end);
  void RowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
  void RowsInserted(const QModelIndex& parent, int start, int end);

private:
  void AddRowMappings(const QModelIndex& begin, const QModelIndex& end);
  void AddRowItem(QStandardItem* item, const QString& parent_path);
  void UpdatePathsRecursive(const QModelIndex& parent);

private:
  bool dropping_rows_;

  QIcon playlist_icon_;
  QIcon folder_icon_;

  QMap<int, QStandardItem*> playlists_by_id_;
  QMap<QString, QStandardItem*> folders_by_path_;
};

#endif // PLAYLISTLISTMODEL_H
