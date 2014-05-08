#include "clementinewebpage.h"

#include <QByteArray>
#include <QWebFrame>

#include "core/closure.h"
#include "core/logging.h"
#include "remotecontrolmessages.pb.h"

ClementineWebPage::ClementineWebPage(QObject* parent)
    : QWebPage(parent) {
  qLog(Debug) << Q_FUNC_INFO;
  NewClosure(this, SIGNAL(loadFinished(bool)), [&]() {
    qLog(Debug) << Q_FUNC_INFO << "load finished" << totalBytes();
    qLog(Debug) << mainFrame()->toHtml();
    mainFrame()->evaluateJavaScript("window.setTimeout");
  });
}

void ClementineWebPage::javaScriptConsoleMessage(
    const QString& message, int, const QString&) {
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

void ClementineWebPage::javaScriptAlert(QWebFrame*, const QString& message) {
  qLog(Debug) << message;
}

bool ClementineWebPage::shouldInterruptJavaScript() {
  qLog(Debug) << Q_FUNC_INFO;
  return false;
}
