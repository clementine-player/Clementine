#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <QStyledItemDelegate>
#include <QTreeView>

class RadioLoadingIndicator;

class PlaylistDelegateBase : public QStyledItemDelegate {
 public:
  PlaylistDelegateBase(QTreeView* view);
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  QString displayText(const QVariant& value, const QLocale& locale) const;

  QStyleOptionViewItemV4 Adjusted(const QStyleOptionViewItem& option, const QModelIndex& index) const;

 protected:
  QTreeView* view_;
};

class LengthItemDelegate : public PlaylistDelegateBase {
 public:
  LengthItemDelegate(QTreeView* view) : PlaylistDelegateBase(view) {}
  QString displayText(const QVariant& value, const QLocale& locale) const;
};

class SizeItemDelegate : public PlaylistDelegateBase {
 public:
  SizeItemDelegate(QTreeView* view) : PlaylistDelegateBase(view) {}
  QString displayText(const QVariant& value, const QLocale& locale) const;
};


class PlaylistView : public QTreeView {
  Q_OBJECT

 public:
  PlaylistView(QWidget* parent = 0);

  // QTreeView
  void setModel(QAbstractItemModel *model);
  void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  void keyPressEvent(QKeyEvent* event);

  // QAbstractScrollArea
  void contextMenuEvent(QContextMenuEvent* e);

 public slots:
  void StopGlowing();
  void StartGlowing();

 signals:
  void PlayPauseItem(const QModelIndex& index);
  void RightClicked(const QPoint& global_pos, const QModelIndex& index);

 protected:
  void hideEvent(QHideEvent* event);
  void showEvent(QShowEvent* event);

 private slots:
  void LoadGeometry();
  void SaveGeometry();
  void GlowIntensityChanged();

 private:
  void ReloadBarPixmaps();
  QList<QPixmap> LoadBarPixmap(const QString& filename);

 private:
  static const char* kSettingsGroup;
  static const int kGlowIntensitySteps;

  bool glow_enabled_;
  QTimer* glow_timer_;
  int glow_intensity_step_;
  QModelIndex last_current_item_;
  QRect last_glow_rect_;

  int row_height_; // Used to invalidate the currenttrack_bar pixmaps
  QList<QPixmap> currenttrack_bar_left_;
  QList<QPixmap> currenttrack_bar_mid_;
  QList<QPixmap> currenttrack_bar_right_;
  QPixmap currenttrack_play_;
  QPixmap currenttrack_pause_;
};

#endif // PLAYLISTVIEW_H
