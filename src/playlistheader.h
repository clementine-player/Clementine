#ifndef PLAYLISTHEADER_H
#define PLAYLISTHEADER_H

#include <QHeaderView>

class QMenu;
class QSignalMapper;

class PlaylistHeader : public QHeaderView {
  Q_OBJECT

 public:
  PlaylistHeader(Qt::Orientation orientation, QWidget* parent = 0);

  // QWidget
  void contextMenuEvent(QContextMenuEvent* e);

 private slots:
  void HideCurrent();
  void ToggleVisible(int section);

 private:
  void AddColumnAction(int index);

 private:
  int menu_section_;
  QMenu* menu_;
  QAction* hide_action_;
  QMenu* show_menu_;

  QSignalMapper* show_mapper_;
};

#endif // PLAYLISTHEADER_H
