#ifndef FILEVIEWLIST_H
#define FILEVIEWLIST_H

#include <QListView>
#include <QUrl>

class FileViewList : public QListView {
  Q_OBJECT

 public:
  FileViewList(QWidget* parent = 0);

 signals:
  void AddToPlaylist(const QList<QUrl>& urls);
  void CopyToLibrary(const QList<QUrl>& urls);
  void MoveToLibrary(const QList<QUrl>& urls);

 protected:
  void contextMenuEvent(QContextMenuEvent* e);

 private slots:
  void AddToPlaylistSlot();
  void CopyToLibrarySlot();
  void MoveToLibrarySlot();

  QList<QUrl> UrlListFromSelection() const;

 private:
  QMenu* menu_;
  QItemSelection menu_selection_;
};

#endif // FILEVIEWLIST_H
