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

 private slots:
  void ItemExpanded(const QModelIndex& index);

 private:
  void RecheckIsEmpty();
  bool RecursivelyExpand(const QModelIndex& index, int* count);

 private:
  static const int kRowsToShow;

  Library* library_;
  int total_song_count_;

  QPixmap nomusic_;
};

#endif // LIBRARYVIEW_H
