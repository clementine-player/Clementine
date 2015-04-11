#ifndef SINGLE_CORE_APPLICATION_H
#define SINGLE_CORE_APPLICATION_H

#include <QCoreApplication>
#include <QLocalSocket>
#include <QLocalServer>

/**
 * @brief The SingleCoreApplication class handles multipe instances of the same Application
 * @see QApplication
 */
class SingleCoreApplication : public QCoreApplication
{
  Q_OBJECT
public:
  explicit SingleCoreApplication(int &, char **);
  ~SingleCoreApplication();

signals:
  void showUp();

private slots:
  void slotConnectionEstablished();

private:
  QLocalSocket *socket;
  QLocalServer *server;

};

#endif // SINGLE_CORE_APPLICATION_H

