#ifndef LIBRARYVIEW_H
#define LIBRARYVIEW_H

#include <QStyledItemDelegate>
#include <QTreeView>

class Library;

class LibraryItemDelegate : public QStyledItemDelegate {
 public:
  LibraryItemDelegate(QObject* parent);
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

class LibraryView : public QTreeView {
  Q_OBJECT

 public:
  LibraryView(QWidget* parent = 0);

  void SetLibrary(Library* library);

 public slots:
  void TotalSongCountUpdated(int count);

 signals:
  void ShowConfigDialog();

 protected:
  // QAbstractItemView
  void reset();

  // QWidget
  void paintEvent(QPaintEvent* event);
  void mouseReleaseEvent(QMouseEvent* e);
  void contextMenuEvent(QContextMenuEvent* e);

 private slots:
  void ItemExpanded(const QModelIndex& index);
  void ShowInVarious();
  void NoShowInVarious();

 private:
  void RecheckIsEmpty();
  bool RecursivelyExpand(const QModelIndex& index, int* count);
  void ShowInVarious(bool on);

 private:
  static const int kRowsToShow;

  Library* library_;
  int total_song_count_;

  QPixmap nomusic_;

  QMenu* context_menu_;
  QModelIndex context_menu_index_;
  QAction* show_in_various_;
  QAction* no_show_in_various_;
};

#endif // LIBRARYVIEW_H
