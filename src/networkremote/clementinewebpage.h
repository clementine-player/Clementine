#include <QWebPage>

#include "remotecontrolmessages.pb.h"

class ClementineWebPage : public QWebPage {
  Q_OBJECT

 public:
  explicit ClementineWebPage(QObject* parent = nullptr);
  void Init(const QString& base_url);
  const QString id() const { return id_; }

 public slots:
  bool shouldInterruptJavaScript();

 signals:
  void Connected();
  void MessageReceived(const pb::remote::Message& msg);

 protected:
  // For OOB communication for controlling the appengine channel.
  virtual bool javaScriptConfirm(QWebFrame*, const QString& message);
  // For generic error messages from the javascript.
  virtual void javaScriptConsoleMessage(
      const QString& message, int line, const QString& source);
  // For receiving base64 proto messages; the actual remote control messages.
  virtual void javaScriptAlert(QWebFrame*, const QString& message);

 private:
  QString id_;
  QString base_url_;
};
