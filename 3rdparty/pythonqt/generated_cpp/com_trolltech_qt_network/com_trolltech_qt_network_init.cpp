#include <PythonQt.h>
#include "com_trolltech_qt_network_init.h"
#include "com_trolltech_qt_network0.h"


void PythonQt_init_QtNetwork(PyObject* module) {
PythonQt::priv()->registerClass(&QAbstractNetworkCache::staticMetaObject, "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QAbstractNetworkCache>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QAbstractNetworkCache>, module, 0);
PythonQt::priv()->registerClass(&QAbstractSocket::staticMetaObject, "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QAbstractSocket>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QAbstractSocket>, module, 0);
PythonQt::priv()->registerCPPClass("QAuthenticator", "", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QAuthenticator>, NULL, module, PythonQt::Type_NonZero|PythonQt::Type_RichCompare);
PythonQt::priv()->registerClass(&QFtp::staticMetaObject, "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QFtp>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QFtp>, module, 0);
PythonQt::priv()->registerCPPClass("QHostAddress", "", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QHostAddress>, NULL, module, PythonQt::Type_NonZero|PythonQt::Type_RichCompare);
PythonQt::priv()->registerCPPClass("QHostInfo", "", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QHostInfo>, NULL, module, 0);
PythonQt::priv()->registerClass(&QHttp::staticMetaObject, "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QHttp>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QHttp>, module, 0);
PythonQt::priv()->registerCPPClass("QHttpHeader", "", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QHttpHeader>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QHttpHeader>, module, 0);
PythonQt::priv()->registerCPPClass("QHttpRequestHeader", "QHttpHeader", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QHttpRequestHeader>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QHttpRequestHeader>, module, 0);
PythonQt::priv()->registerCPPClass("QHttpResponseHeader", "QHttpHeader", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QHttpResponseHeader>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QHttpResponseHeader>, module, 0);
PythonQt::priv()->registerCPPClass("QIPv6Address", "", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QIPv6Address>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QIPv6Address>, module, 0);
PythonQt::priv()->registerClass(&QLocalServer::staticMetaObject, "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QLocalServer>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QLocalServer>, module, 0);
PythonQt::priv()->registerClass(&QLocalSocket::staticMetaObject, "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QLocalSocket>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QLocalSocket>, module, 0);
PythonQt::priv()->registerClass(&QNetworkAccessManager::staticMetaObject, "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QNetworkAccessManager>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QNetworkAccessManager>, module, 0);
PythonQt::priv()->registerCPPClass("QNetworkAddressEntry", "", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QNetworkAddressEntry>, NULL, module, PythonQt::Type_RichCompare);
PythonQt::priv()->registerCPPClass("QNetworkCacheMetaData", "", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QNetworkCacheMetaData>, NULL, module, PythonQt::Type_RichCompare);
PythonQt::priv()->registerCPPClass("QNetworkCookie", "", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QNetworkCookie>, NULL, module, PythonQt::Type_RichCompare);
PythonQt::priv()->registerClass(&QNetworkCookieJar::staticMetaObject, "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QNetworkCookieJar>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QNetworkCookieJar>, module, 0);
PythonQt::priv()->registerClass(&QNetworkDiskCache::staticMetaObject, "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QNetworkDiskCache>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QNetworkDiskCache>, module, 0);
PythonQt::priv()->registerCPPClass("QNetworkInterface", "", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QNetworkInterface>, NULL, module, 0);
PythonQt::priv()->registerCPPClass("QNetworkProxy", "", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QNetworkProxy>, NULL, module, PythonQt::Type_RichCompare);
PythonQt::priv()->registerCPPClass("QNetworkProxyFactory", "", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QNetworkProxyFactory>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QNetworkProxyFactory>, module, 0);
PythonQt::priv()->registerCPPClass("QNetworkProxyQuery", "", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QNetworkProxyQuery>, NULL, module, PythonQt::Type_RichCompare);
PythonQt::priv()->registerClass(&QNetworkReply::staticMetaObject, "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QNetworkReply>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QNetworkReply>, module, 0);
PythonQt::priv()->registerCPPClass("QNetworkRequest", "", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QNetworkRequest>, NULL, module, PythonQt::Type_RichCompare);
PythonQt::priv()->registerCPPClass("QSsl", "", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QSsl>, NULL, module, 0);
PythonQt::priv()->registerClass(&QTcpServer::staticMetaObject, "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QTcpServer>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QTcpServer>, module, 0);
PythonQt::priv()->registerClass(&QTcpSocket::staticMetaObject, "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QTcpSocket>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QTcpSocket>, module, 0);
PythonQt::priv()->registerClass(&QUdpSocket::staticMetaObject, "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QUdpSocket>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QUdpSocket>, module, 0);
PythonQt::priv()->registerCPPClass("QUrlInfo", "", "QtNetwork", PythonQtCreateObject<PythonQtWrapper_QUrlInfo>, PythonQtSetInstanceWrapperOnShell<PythonQtShell_QUrlInfo>, module, PythonQt::Type_RichCompare);

}
