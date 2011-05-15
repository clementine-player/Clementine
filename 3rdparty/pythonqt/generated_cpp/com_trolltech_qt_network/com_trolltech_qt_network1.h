#include <PythonQt.h>
#include <QObject>
#include <QVariant>
#include <qauthenticator.h>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qdatetime.h>
#include <qhostaddress.h>
#include <qiodevice.h>
#include <qlist.h>
#include <qnetworkproxy.h>
#include <qobject.h>
#include <qsslcertificate.h>
#include <qsslcipher.h>
#include <qsslconfiguration.h>
#include <qsslerror.h>
#include <qsslkey.h>
#include <qsslsocket.h>
#include <qurl.h>



class PythonQtWrapper_QSslKey : public QObject
{ Q_OBJECT
public:
public slots:
QSslKey* new_QSslKey();
QSslKey* new_QSslKey(QIODevice*  device, QSsl::KeyAlgorithm  algorithm, QSsl::EncodingFormat  format = QSsl::Pem, QSsl::KeyType  type = QSsl::PrivateKey, const QByteArray&  passPhrase = QByteArray());
QSslKey* new_QSslKey(const QByteArray&  encoded, QSsl::KeyAlgorithm  algorithm, QSsl::EncodingFormat  format = QSsl::Pem, QSsl::KeyType  type = QSsl::PrivateKey, const QByteArray&  passPhrase = QByteArray());
QSslKey* new_QSslKey(const QSslKey&  other);
void delete_QSslKey(QSslKey* obj) { delete obj; } 
   QSsl::KeyAlgorithm  algorithm(QSslKey* theWrappedObject) const;
   QByteArray  toDer(QSslKey* theWrappedObject, const QByteArray&  passPhrase = QByteArray()) const;
   QSslKey*  operator_assign(QSslKey* theWrappedObject, const QSslKey&  other);
   void clear(QSslKey* theWrappedObject);
   int  length(QSslKey* theWrappedObject) const;
   QByteArray  toPem(QSslKey* theWrappedObject, const QByteArray&  passPhrase = QByteArray()) const;
   Qt::HANDLE  handle(QSslKey* theWrappedObject) const;
   QSsl::KeyType  type(QSslKey* theWrappedObject) const;
   bool  operator_equal(QSslKey* theWrappedObject, const QSslKey&  key) const;
   bool  isNull(QSslKey* theWrappedObject) const;
    QString py_toString(QSslKey*);
};





class PythonQtShell_QSslSocket : public QSslSocket
{
public:
    PythonQtShell_QSslSocket(QObject*  parent = 0):QSslSocket(parent),_wrapper(NULL) {};

virtual bool  canReadLine() const;
virtual qint64  bytesToWrite() const;
virtual qint64  readData(char*  data, qint64  maxlen);
virtual qint64  writeData(const char*  data, qint64  len);
virtual qint64  bytesAvailable() const;
virtual bool  waitForBytesWritten(int  msecs = 30000);
virtual bool  atEnd() const;
virtual void close();
virtual bool  waitForReadyRead(int  msecs = 30000);
virtual qint64  readLineData(char*  data, qint64  maxlen);
virtual bool  isSequential() const;
virtual qint64  size() const;
virtual qint64  pos() const;
virtual bool  reset();
virtual bool  open(QIODevice::OpenMode  mode);
virtual bool  seek(qint64  pos);
virtual void childEvent(QChildEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QSslSocket : public QSslSocket
{ public:
inline bool  promoted_canReadLine() const { return QSslSocket::canReadLine(); }
inline qint64  promoted_bytesToWrite() const { return QSslSocket::bytesToWrite(); }
inline qint64  promoted_readData(char*  data, qint64  maxlen) { return QSslSocket::readData(data, maxlen); }
inline qint64  promoted_writeData(const char*  data, qint64  len) { return QSslSocket::writeData(data, len); }
inline qint64  promoted_bytesAvailable() const { return QSslSocket::bytesAvailable(); }
inline bool  promoted_waitForBytesWritten(int  msecs = 30000) { return QSslSocket::waitForBytesWritten(msecs); }
inline bool  promoted_atEnd() const { return QSslSocket::atEnd(); }
inline void promoted_close() { QSslSocket::close(); }
inline bool  promoted_waitForReadyRead(int  msecs = 30000) { return QSslSocket::waitForReadyRead(msecs); }
};

class PythonQtWrapper_QSslSocket : public QObject
{ Q_OBJECT
public:
Q_ENUMS(SslMode PeerVerifyMode )
enum SslMode{
  UnencryptedMode = QSslSocket::UnencryptedMode,   SslClientMode = QSslSocket::SslClientMode,   SslServerMode = QSslSocket::SslServerMode};
enum PeerVerifyMode{
  VerifyNone = QSslSocket::VerifyNone,   QueryPeer = QSslSocket::QueryPeer,   VerifyPeer = QSslSocket::VerifyPeer,   AutoVerifyPeer = QSslSocket::AutoVerifyPeer};
public slots:
QSslSocket* new_QSslSocket(QObject*  parent = 0);
void delete_QSslSocket(QSslSocket* obj) { delete obj; } 
   QSslSocket::SslMode  mode(QSslSocket* theWrappedObject) const;
   void setProtocol(QSslSocket* theWrappedObject, QSsl::SslProtocol  protocol);
   void static_QSslSocket_setDefaultCiphers(const QList<QSslCipher >&  ciphers);
   void static_QSslSocket_setDefaultCaCertificates(const QList<QSslCertificate >&  certificates);
   bool  static_QSslSocket_supportsSsl();
   void setSocketOption(QSslSocket* theWrappedObject, QAbstractSocket::SocketOption  option, const QVariant&  value);
   bool  isEncrypted(QSslSocket* theWrappedObject) const;
   bool  canReadLine(QSslSocket* theWrappedObject) const;
   QSslKey  privateKey(QSslSocket* theWrappedObject) const;
   QList<QSslCertificate >  static_QSslSocket_defaultCaCertificates();
   QSslConfiguration  sslConfiguration(QSslSocket* theWrappedObject) const;
   qint64  bytesToWrite(QSslSocket* theWrappedObject) const;
   void setCaCertificates(QSslSocket* theWrappedObject, const QList<QSslCertificate >&  certificates);
   QList<QSslCipher >  static_QSslSocket_defaultCiphers();
   QList<QSslError >  sslErrors(QSslSocket* theWrappedObject) const;
   void connectToHostEncrypted(QSslSocket* theWrappedObject, const QString&  hostName, unsigned short  port, const QString&  sslPeerName, QIODevice::OpenMode  mode = QIODevice::ReadWrite);
   void connectToHostEncrypted(QSslSocket* theWrappedObject, const QString&  hostName, unsigned short  port, QIODevice::OpenMode  mode = QIODevice::ReadWrite);
   void ignoreSslErrors(QSslSocket* theWrappedObject, const QList<QSslError >&  errors);
   void setCiphers(QSslSocket* theWrappedObject, const QString&  ciphers);
   qint64  encryptedBytesAvailable(QSslSocket* theWrappedObject) const;
   void setCiphers(QSslSocket* theWrappedObject, const QList<QSslCipher >&  ciphers);
   void addCaCertificates(QSslSocket* theWrappedObject, const QList<QSslCertificate >&  certificates);
   bool  addCaCertificates(QSslSocket* theWrappedObject, const QString&  path, QSsl::EncodingFormat  format = QSsl::Pem, QRegExp::PatternSyntax  syntax = QRegExp::FixedString);
   bool  flush(QSslSocket* theWrappedObject);
   QSslSocket::PeerVerifyMode  peerVerifyMode(QSslSocket* theWrappedObject) const;
   QSslCertificate  peerCertificate(QSslSocket* theWrappedObject) const;
   bool  setSocketDescriptor(QSslSocket* theWrappedObject, int  socketDescriptor, QAbstractSocket::SocketState  state = QAbstractSocket::ConnectedState, QIODevice::OpenMode  openMode = QIODevice::ReadWrite);
   QList<QSslCertificate >  caCertificates(QSslSocket* theWrappedObject) const;
   void setPrivateKey(QSslSocket* theWrappedObject, const QString&  fileName, QSsl::KeyAlgorithm  algorithm = QSsl::Rsa, QSsl::EncodingFormat  format = QSsl::Pem, const QByteArray&  passPhrase = QByteArray());
   qint64  readData(QSslSocket* theWrappedObject, char*  data, qint64  maxlen);
   QSslCipher  sessionCipher(QSslSocket* theWrappedObject) const;
   void setPrivateKey(QSslSocket* theWrappedObject, const QSslKey&  key);
   void setReadBufferSize(QSslSocket* theWrappedObject, qint64  size);
   void addCaCertificate(QSslSocket* theWrappedObject, const QSslCertificate&  certificate);
   qint64  writeData(QSslSocket* theWrappedObject, const char*  data, qint64  len);
   void setSslConfiguration(QSslSocket* theWrappedObject, const QSslConfiguration&  config);
   void setPeerVerifyDepth(QSslSocket* theWrappedObject, int  depth);
   void abort(QSslSocket* theWrappedObject);
   qint64  bytesAvailable(QSslSocket* theWrappedObject) const;
   bool  waitForDisconnected(QSslSocket* theWrappedObject, int  msecs = 30000);
   bool  waitForBytesWritten(QSslSocket* theWrappedObject, int  msecs = 30000);
   bool  atEnd(QSslSocket* theWrappedObject) const;
   void static_QSslSocket_addDefaultCaCertificate(const QSslCertificate&  certificate);
   void setPeerVerifyMode(QSslSocket* theWrappedObject, QSslSocket::PeerVerifyMode  mode);
   void setLocalCertificate(QSslSocket* theWrappedObject, const QString&  fileName, QSsl::EncodingFormat  format = QSsl::Pem);
   void setLocalCertificate(QSslSocket* theWrappedObject, const QSslCertificate&  certificate);
   QList<QSslCipher >  static_QSslSocket_supportedCiphers();
   qint64  encryptedBytesToWrite(QSslSocket* theWrappedObject) const;
   void close(QSslSocket* theWrappedObject);
   bool  waitForConnected(QSslSocket* theWrappedObject, int  msecs = 30000);
   QList<QSslCertificate >  static_QSslSocket_systemCaCertificates();
   bool  waitForReadyRead(QSslSocket* theWrappedObject, int  msecs = 30000);
   QSslCertificate  localCertificate(QSslSocket* theWrappedObject) const;
   bool  waitForEncrypted(QSslSocket* theWrappedObject, int  msecs = 30000);
   QVariant  socketOption(QSslSocket* theWrappedObject, QAbstractSocket::SocketOption  option);
   QList<QSslCertificate >  peerCertificateChain(QSslSocket* theWrappedObject) const;
   QSsl::SslProtocol  protocol(QSslSocket* theWrappedObject) const;
   int  peerVerifyDepth(QSslSocket* theWrappedObject) const;
   void static_QSslSocket_addDefaultCaCertificates(const QList<QSslCertificate >&  certificates);
   bool  static_QSslSocket_addDefaultCaCertificates(const QString&  path, QSsl::EncodingFormat  format = QSsl::Pem, QRegExp::PatternSyntax  syntax = QRegExp::FixedString);
   QList<QSslCipher >  ciphers(QSslSocket* theWrappedObject) const;
};




