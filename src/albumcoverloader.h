#ifndef ALBUMCOVERLOADER_H
#define ALBUMCOVERLOADER_H

#include "backgroundthread.h"

#include <QObject>
#include <QImage>
#include <QMutex>
#include <QQueue>

class AlbumCoverLoader : public QObject {
  Q_OBJECT

 public:
  AlbumCoverLoader(QObject* parent = 0);

  static QString ImageCacheDir();

  void SetDesiredHeight(int height) { height_ = height; }
  quint64 LoadImageAsync(const QString& art_automatic, const QString& art_manual);

  void Clear();

  static QImage TryLoadImage(const QString& automatic, const QString& manual);
  static QPixmap TryLoadPixmap(const QString& automatic, const QString& manual);

  static const char* kManuallyUnsetCover;

 signals:
  void ImageLoaded(quint64 id, const QImage& image);

 private slots:
  void ProcessTasks();

 private:
  struct Task {
    quint64 id;
    QString art_automatic;
    QString art_manual;
  };

  int height_;

  QMutex mutex_;
  QQueue<Task> tasks_;
  quint64 next_id_;
};

#endif // ALBUMCOVERLOADER_H
