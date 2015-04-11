#ifndef SINGLE_APPLICATION_H
#define SINGLE_APPLICATION_H

#include <QApplication>
#include <QLocalSocket>
#include <QLocalServer>

/**
 * @brief The SingleApplication class handles multipe instances of the same Application
 * @see QApplication
 */
class SingleApplication : public QApplication
{
  Q_OBJECT
public:
  explicit SingleApplication(int &, char **);
  ~SingleApplication();

signals:
  void showUp();

private slots:
  void slotConnectionEstablished();

private:
  QLocalSocket *socket;
  QLocalServer *server;

};

#endif // SINGLE_APPLICATION_H

