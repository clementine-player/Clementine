#ifndef SHUFFLEREPEATWIDGET_H
#define SHUFFLEREPEATWIDGET_H

#include <QWidget>

class QMenu;

#include "ui_shufflerepeatwidget.h"

class ShuffleRepeatWidget : public QWidget {
  Q_OBJECT

 public:
  ShuffleRepeatWidget(QWidget *parent = 0);

  enum RepeatMode {
    Repeat_Off = 0,
    Repeat_Track = 1,
    Repeat_Album = 2,
    Repeat_Playlist = 3,
  };
  enum ShuffleMode {
    Shuffle_Off = 0,
    Shuffle_All = 1,
    Shuffle_Album = 2,
  };

  static const char* kSettingsGroup;

  void Load();
  void Save();

 public slots:
  void SetRepeatMode(RepeatMode mode);
  void SetShuffleMode(ShuffleMode mode);

 signals:
  void RepeatModeChanged(RepeatMode mode);
  void ShuffleModeChanged(ShuffleMode mode);

 private slots:
  void RepeatActionTriggered(QAction*);
  void ShuffleActionTriggered(QAction*);

 private:
  Ui::ShuffleRepeatWidget ui_;

  QMenu* repeat_menu_;
  QMenu* shuffle_menu_;

  bool loading_;
  RepeatMode repeat_mode_;
  ShuffleMode shuffle_mode_;
};

#endif // SHUFFLEREPEATWIDGET_H
