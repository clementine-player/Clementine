#include <QWebPage>

#include "remotecontrolmessages.pb.h"

class ClementineWebPage : public QWebPage {
  Q_OBJECT

 public:
  explicit ClementineWebPage(QObject* parent = nullptr);

 public slots:
  bool shouldInterruptJavaScript();

 signals:
  void Connected();
  void MessageReceived(const pb::remote::Message& msg);

 protected:
  virtual void javaScriptConsoleMessage(
      const QString& message, int line, const QString& source);

  virtual void javaScriptAlert(QWebFrame*, const QString&);
};
