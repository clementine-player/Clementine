#ifndef GLOBALSHORTCUTS_H
#define GLOBALSHORTCUTS_H

#include <QObject>

class GlobalShortcuts : public QObject {
  Q_OBJECT

public:
  GlobalShortcuts(QObject* parent = 0);

  static const char* kGsdService;
  static const char* kGsdPath;
  static const char* kGsdInterface;

signals:
  void PlayPause();
  void Stop();
  void Next();
  void Previous();

private:
  void Init();
  bool RegisterGnome();
  bool RegisterX11();

private slots:
  void GnomeMediaKeyPressed(const QString& application, const QString& key);
};

#endif
