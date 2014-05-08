#include "clementinewebpage.h"

#include <QByteArray>
#include <QMetaObject>
#include <QThread>
#include <QWebFrame>

#include "core/closure.h"
#include "core/logging.h"
#include "remotecontrolmessages.pb.h"

namespace {

//const char* kRemoteEndpoint = "http://localhost:8080/channel/remote/%1";
const char* kRemoteEndpoint = "https://remote-dot-clementine-data.appspot.com/channel/remote/%1";

const char* kInitialPage = "https://remote-dot-clementine-data.appspot.com/channel/clementine";

}  // namespace

ClementineWebPage::ClementineWebPage(QObject* parent)
    : QWebPage(parent) {
  qLog(Debug) << Q_FUNC_INFO;
  NewClosure(this, SIGNAL(loadFinished(bool)), [&]() {
    qLog(Debug) << Q_FUNC_INFO << "load finished" << totalBytes();
    qLog(Debug) << mainFrame()->toHtml();
    mainFrame()->evaluateJavaScript("window.setTimeout");
  });
}

void ClementineWebPage::Init() {
  QMetaObject::invokeMethod(this, "InitOnMainThread", Qt::QueuedConnection);
}

void ClementineWebPage::InitOnMainThread() {
  Q_ASSERT(QThread::currentThread() == qApp->thread());
  mainFrame()->load(QUrl(kInitialPage));
}

void ClementineWebPage::javaScriptConsoleMessage(
    const QString& message, int, const QString&) {
  qLog(Error) << message;
}

bool ClementineWebPage::javaScriptConfirm(QWebFrame*, const QString& message) {
  id_ = message;
  qLog(Debug) << "id:" << message;
  qLog(Debug) << QString(kRemoteEndpoint).arg(message);
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
