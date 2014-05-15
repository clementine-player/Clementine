#include "clementinewebpage.h"

#include <QByteArray>
#include <QMetaObject>
#include <QThread>
#include <QWebFrame>

#include "core/closure.h"
#include "core/logging.h"
#include "remotecontrolmessages.pb.h"

namespace {

const char* kInitialPagePath = "/channel/clementine";
const char* kRemoteEndpointPath = "/channel/remote/%1";

}  // namespace

ClementineWebPage::ClementineWebPage(QObject* parent)
    : QWebPage(parent) {
  qLog(Debug) << Q_FUNC_INFO;
  NewClosure(this, SIGNAL(loadFinished(bool)), [=]() {
    qLog(Debug) << Q_FUNC_INFO << "load finished" << totalBytes();
    qLog(Debug) << mainFrame()->toHtml();
  });
}

void ClementineWebPage::Init(const QString& base_url) {
  RunOnMainThread([=]{
    Q_ASSERT(QThread::currentThread() == qApp->thread());
    base_url_ = base_url;
    QUrl url(base_url_);
    url.setPath(kInitialPagePath);
    mainFrame()->load(url);
  });
}

void ClementineWebPage::javaScriptConsoleMessage(
    const QString& message, int, const QString&) {
  qLog(Error) << message;
}

bool ClementineWebPage::javaScriptConfirm(QWebFrame*, const QString& message) {
  id_ = message;
  qLog(Debug) << "id:" << message;
  QUrl url(base_url_);
  url.setPath(QString(kRemoteEndpointPath).arg(message));
  qLog(Debug) << url;
  return true;
}

void ClementineWebPage::javaScriptAlert(QWebFrame*, const QString& message) {
  qLog(Debug) << message;

  QByteArray bytes = QByteArray::fromBase64(message.toAscii());
  pb::remote::Message msg;
  msg.ParseFromArray(bytes.constData(), bytes.size());
  qLog(Debug) << msg.DebugString().c_str();

  if (msg.type() == pb::remote::CONNECT) {
    emit Connected();
    return;
  }

  emit MessageReceived(msg);
}

bool ClementineWebPage::shouldInterruptJavaScript() {
  qLog(Debug) << Q_FUNC_INFO;
  // Make sure any long-running js does not get interrupted.
  return false;
}
