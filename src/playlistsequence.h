#ifndef PLAYLISTSEQUENCE_H
#define PLAYLISTSEQUENCE_H

#include <QWidget>

#include "ui_playlistsequence.h"

class QMenu;

class PlaylistSequence : public QWidget {
  Q_OBJECT

 public:
  PlaylistSequence(QWidget *parent = 0);

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

  RepeatMode repeat_mode() const { return repeat_mode_; }
  ShuffleMode shuffle_mode() const { return shuffle_mode_; }

 public slots:
  void SetRepeatMode(PlaylistSequence::RepeatMode mode);
  void SetShuffleMode(PlaylistSequence::ShuffleMode mode);

 signals:
  void RepeatModeChanged(PlaylistSequence::RepeatMode mode);
  void ShuffleModeChanged(PlaylistSequence::ShuffleMode mode);

 private slots:
  void RepeatActionTriggered(QAction*);
  void ShuffleActionTriggered(QAction*);

 private:
  void Load();
  void Save();

 private:
  Ui::PlaylistSequence ui_;

  QMenu* repeat_menu_;
  QMenu* shuffle_menu_;

  bool loading_;
  RepeatMode repeat_mode_;
  ShuffleMode shuffle_mode_;
};

#endif // PLAYLISTSEQUENCE_H
