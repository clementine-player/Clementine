#include <PythonQt.h>
#include <QDateTime>
#include <QObject>
#include <QVariant>
#include <qabstractnetworkcache.h>
#include <qabstractsocket.h>
#include <qauthenticator.h>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qdatastream.h>
#include <qdatetime.h>
#include <qftp.h>
#include <qhostaddress.h>
#include <qhostinfo.h>
#include <qhttp.h>
#include <qiodevice.h>
#include <qlist.h>
#include <qlocalserver.h>
#include <qlocalsocket.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkcookie.h>
#include <qnetworkcookiejar.h>
#include <qnetworkdiskcache.h>
#include <qnetworkinterface.h>
#include <qnetworkproxy.h>
#include <qnetworkreply.h>
#include <qnetworkrequest.h>
#include <qobject.h>
#include <qpair.h>
#include <qssl.h>
#include <qstringlist.h>
#include <qtcpserver.h>
#include <qtcpsocket.h>
#include <qudpsocket.h>
#include <qurl.h>
#include <qurlinfo.h>



class PythonQtShell_QAbstractNetworkCache : public QAbstractNetworkCache
{
public:
    PythonQtShell_QAbstractNetworkCache(QObject*  parent = 0):QAbstractNetworkCache(parent),_wrapper(NULL) {};

virtual qint64  cacheSize() const;
virtual void childEvent(QChildEvent*  arg__1);
virtual void clear();
virtual void customEvent(QEvent*  arg__1);
virtual QIODevice*  data(const QUrl&  url);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void insert(QIODevice*  device);
virtual QNetworkCacheMetaData  metaData(const QUrl&  url);
virtual QIODevice*  prepare(const QNetworkCacheMetaData&  metaData);
virtual bool  remove(const QUrl&  url);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void updateMetaData(const QNetworkCacheMetaData&  metaData);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QAbstractNetworkCache : public QObject
{ Q_OBJECT
public:
public slots:
void delete_QAbstractNetworkCache(QAbstractNetworkCache* obj) { delete obj; } 
};





class PythonQtShell_QAbstractSocket : public QAbstractSocket
{
public:
    PythonQtShell_QAbstractSocket(QAbstractSocket::SocketType  socketType, QObject*  parent):QAbstractSocket(socketType, parent),_wrapper(NULL) {};

virtual bool  atEnd() const;
virtual qint64  bytesAvailable() const;
virtual qint64  bytesToWrite() const;
virtual bool  canReadLine() const;
virtual void childEvent(QChildEvent*  arg__1);
virtual void close();
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual bool  isSequential() const;
virtual bool  open(QIODevice::OpenMode  mode);
virtual qint64  pos() const;
virtual qint64  readData(char*  data, qint64  maxlen);
virtual qint64  readLineData(char*  data, qint64  maxlen);
virtual bool  reset();
virtual bool  seek(qint64  pos);
virtual qint64  size() const;
virtual void timerEvent(QTimerEvent*  arg__1);
virtual bool  waitForBytesWritten(int  msecs = 30000);
virtual bool  waitForReadyRead(int  msecs = 30000);
virtual qint64  writeData(const char*  data, qint64  len);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QAbstractSocket : public QAbstractSocket
{ public:
inline bool  promoted_atEnd() const { return QAbstractSocket::atEnd(); }
inline qint64  promoted_bytesAvailable() const { return QAbstractSocket::bytesAvailable(); }
inline qint64  promoted_bytesToWrite() const { return QAbstractSocket::bytesToWrite(); }
inline bool  promoted_canReadLine() const { return QAbstractSocket::canReadLine(); }
inline void promoted_close() { QAbstractSocket::close(); }
inline bool  promoted_isSequential() const { return QAbstractSocket::isSequential(); }
inline qint64  promoted_readData(char*  data, qint64  maxlen) { return QAbstractSocket::readData(data, maxlen); }
inline qint64  promoted_readLineData(char*  data, qint64  maxlen) { return QAbstractSocket::readLineData(data, maxlen); }
inline bool  promoted_waitForBytesWritten(int  msecs = 30000) { return QAbstractSocket::waitForBytesWritten(msecs); }
inline bool  promoted_waitForReadyRead(int  msecs = 30000) { return QAbstractSocket::waitForReadyRead(msecs); }
inline qint64  promoted_writeData(const char*  data, qint64  len) { return QAbstractSocket::writeData(data, len); }
};

class PythonQtWrapper_QAbstractSocket : public QObject
{ Q_OBJECT
public:
Q_ENUMS(SocketType SocketError NetworkLayerProtocol SocketOption SocketState )
enum SocketType{
  TcpSocket = QAbstractSocket::TcpSocket,   UdpSocket = QAbstractSocket::UdpSocket,   UnknownSocketType = QAbstractSocket::UnknownSocketType};
enum SocketError{
  ConnectionRefusedError = QAbstractSocket::ConnectionRefusedError,   RemoteHostClosedError = QAbstractSocket::RemoteHostClosedError,   HostNotFoundError = QAbstractSocket::HostNotFoundError,   SocketAccessError = QAbstractSocket::SocketAccessError,   SocketResourceError = QAbstractSocket::SocketResourceError,   SocketTimeoutError = QAbstractSocket::SocketTimeoutError,   DatagramTooLargeError = QAbstractSocket::DatagramTooLargeError,   NetworkError = QAbstractSocket::NetworkError,   AddressInUseError = QAbstractSocket::AddressInUseError,   SocketAddressNotAvailableError = QAbstractSocket::SocketAddressNotAvailableError,   UnsupportedSocketOperationError = QAbstractSocket::UnsupportedSocketOperationError,   UnfinishedSocketOperationError = QAbstractSocket::UnfinishedSocketOperationError,   ProxyAuthenticationRequiredError = QAbstractSocket::ProxyAuthenticationRequiredError,   SslHandshakeFailedError = QAbstractSocket::SslHandshakeFailedError,   ProxyConnectionRefusedError = QAbstractSocket::ProxyConnectionRefusedError,   ProxyConnectionClosedError = QAbstractSocket::ProxyConnectionClosedError,   ProxyConnectionTimeoutError = QAbstractSocket::ProxyConnectionTimeoutError,   ProxyNotFoundError = QAbstractSocket::ProxyNotFoundError,   ProxyProtocolError = QAbstractSocket::ProxyProtocolError,   UnknownSocketError = QAbstractSocket::UnknownSocketError};
enum NetworkLayerProtocol{
  IPv4Protocol = QAbstractSocket::IPv4Protocol,   IPv6Protocol = QAbstractSocket::IPv6Protocol,   UnknownNetworkLayerProtocol = QAbstractSocket::UnknownNetworkLayerProtocol};
enum SocketOption{
  LowDelayOption = QAbstractSocket::LowDelayOption,   KeepAliveOption = QAbstractSocket::KeepAliveOption};
enum SocketState{
  UnconnectedState = QAbstractSocket::UnconnectedState,   HostLookupState = QAbstractSocket::HostLookupState,   ConnectingState = QAbstractSocket::ConnectingState,   ConnectedState = QAbstractSocket::ConnectedState,   BoundState = QAbstractSocket::BoundState,   ListeningState = QAbstractSocket::ListeningState,   ClosingState = QAbstractSocket::ClosingState};
public slots:
QAbstractSocket* new_QAbstractSocket(QAbstractSocket::SocketType  socketType, QObject*  parent);
void delete_QAbstractSocket(QAbstractSocket* obj) { delete obj; } 
   void abort(QAbstractSocket* theWrappedObject);
   bool  atEnd(QAbstractSocket* theWrappedObject) const;
   qint64  bytesAvailable(QAbstractSocket* theWrappedObject) const;
   qint64  bytesToWrite(QAbstractSocket* theWrappedObject) const;
   bool  canReadLine(QAbstractSocket* theWrappedObject) const;
   void close(QAbstractSocket* theWrappedObject);
   void connectToHost(QAbstractSocket* theWrappedObject, const QHostAddress&  address, unsigned short  port, QIODevice::OpenMode  mode = QIODevice::ReadWrite);
   void connectToHost(QAbstractSocket* theWrappedObject, const QString&  hostName, unsigned short  port, QIODevice::OpenMode  mode = QIODevice::ReadWrite);
   void disconnectFromHost(QAbstractSocket* theWrappedObject);
   QAbstractSocket::SocketError  error(QAbstractSocket* theWrappedObject) const;
   bool  flush(QAbstractSocket* theWrappedObject);
   bool  isSequential(QAbstractSocket* theWrappedObject) const;
   bool  isValid(QAbstractSocket* theWrappedObject) const;
   QHostAddress  localAddress(QAbstractSocket* theWrappedObject) const;
   unsigned short  localPort(QAbstractSocket* theWrappedObject) const;
   QHostAddress  peerAddress(QAbstractSocket* theWrappedObject) const;
   QString  peerName(QAbstractSocket* theWrappedObject) const;
   unsigned short  peerPort(QAbstractSocket* theWrappedObject) const;
   QNetworkProxy  proxy(QAbstractSocket* theWrappedObject) const;
   qint64  readBufferSize(QAbstractSocket* theWrappedObject) const;
   qint64  readData(QAbstractSocket* theWrappedObject, char*  data, qint64  maxlen);
   qint64  readLineData(QAbstractSocket* theWrappedObject, char*  data, qint64  maxlen);
   void setProxy(QAbstractSocket* theWrappedObject, const QNetworkProxy&  networkProxy);
   void setReadBufferSize(QAbstractSocket* theWrappedObject, qint64  size);
   bool  setSocketDescriptor(QAbstractSocket* theWrappedObject, int  socketDescriptor, QAbstractSocket::SocketState  state = QAbstractSocket::ConnectedState, QIODevice::OpenMode  openMode = QIODevice::ReadWrite);
   int  socketDescriptor(QAbstractSocket* theWrappedObject) const;
   QAbstractSocket::SocketType  socketType(QAbstractSocket* theWrappedObject) const;
   QAbstractSocket::SocketState  state(QAbstractSocket* theWrappedObject) const;
   bool  waitForBytesWritten(QAbstractSocket* theWrappedObject, int  msecs = 30000);
   bool  waitForConnected(QAbstractSocket* theWrappedObject, int  msecs = 30000);
   bool  waitForDisconnected(QAbstractSocket* theWrappedObject, int  msecs = 30000);
   bool  waitForReadyRead(QAbstractSocket* theWrappedObject, int  msecs = 30000);
   qint64  writeData(QAbstractSocket* theWrappedObject, const char*  data, qint64  len);
};





class PythonQtWrapper_QAuthenticator : public QObject
{ Q_OBJECT
public:
public slots:
QAuthenticator* new_QAuthenticator();
QAuthenticator* new_QAuthenticator(const QAuthenticator&  other);
void delete_QAuthenticator(QAuthenticator* obj) { delete obj; } 
   bool  isNull(QAuthenticator* theWrappedObject) const;
   bool  __ne__(QAuthenticator* theWrappedObject, const QAuthenticator&  other) const;
   bool  __eq__(QAuthenticator* theWrappedObject, const QAuthenticator&  other) const;
   QString  password(QAuthenticator* theWrappedObject) const;
   QString  realm(QAuthenticator* theWrappedObject) const;
   void setPassword(QAuthenticator* theWrappedObject, const QString&  password);
   void setUser(QAuthenticator* theWrappedObject, const QString&  user);
   QString  user(QAuthenticator* theWrappedObject) const;
    bool __nonzero__(QAuthenticator* obj) { return !obj->isNull(); }
};





class PythonQtShell_QFtp : public QFtp
{
public:
    PythonQtShell_QFtp(QObject*  parent = 0):QFtp(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QFtp : public QObject
{ Q_OBJECT
public:
Q_ENUMS(TransferType Error Command TransferMode State )
enum TransferType{
  Binary = QFtp::Binary,   Ascii = QFtp::Ascii};
enum Error{
  NoError = QFtp::NoError,   UnknownError = QFtp::UnknownError,   HostNotFound = QFtp::HostNotFound,   ConnectionRefused = QFtp::ConnectionRefused,   NotConnected = QFtp::NotConnected};
enum Command{
  None = QFtp::None,   SetTransferMode = QFtp::SetTransferMode,   SetProxy = QFtp::SetProxy,   ConnectToHost = QFtp::ConnectToHost,   Login = QFtp::Login,   Close = QFtp::Close,   List = QFtp::List,   Cd = QFtp::Cd,   Get = QFtp::Get,   Put = QFtp::Put,   Remove = QFtp::Remove,   Mkdir = QFtp::Mkdir,   Rmdir = QFtp::Rmdir,   Rename = QFtp::Rename,   RawCommand = QFtp::RawCommand};
enum TransferMode{
  Active = QFtp::Active,   Passive = QFtp::Passive};
enum State{
  Unconnected = QFtp::Unconnected,   HostLookup = QFtp::HostLookup,   Connecting = QFtp::Connecting,   Connected = QFtp::Connected,   LoggedIn = QFtp::LoggedIn,   Closing = QFtp::Closing};
public slots:
QFtp* new_QFtp(QObject*  parent = 0);
void delete_QFtp(QFtp* obj) { delete obj; } 
   qint64  bytesAvailable(QFtp* theWrappedObject) const;
   int  cd(QFtp* theWrappedObject, const QString&  dir);
   void clearPendingCommands(QFtp* theWrappedObject);
   int  close(QFtp* theWrappedObject);
   int  connectToHost(QFtp* theWrappedObject, const QString&  host, unsigned short  port = 21);
   QFtp::Command  currentCommand(QFtp* theWrappedObject) const;
   QIODevice*  currentDevice(QFtp* theWrappedObject) const;
   int  currentId(QFtp* theWrappedObject) const;
   QFtp::Error  error(QFtp* theWrappedObject) const;
   QString  errorString(QFtp* theWrappedObject) const;
   int  get(QFtp* theWrappedObject, const QString&  file, QIODevice*  dev = 0, QFtp::TransferType  type = QFtp::Binary);
   bool  hasPendingCommands(QFtp* theWrappedObject) const;
   int  list(QFtp* theWrappedObject, const QString&  dir = QString());
   int  login(QFtp* theWrappedObject, const QString&  user = QString(), const QString&  password = QString());
   int  mkdir(QFtp* theWrappedObject, const QString&  dir);
   int  put(QFtp* theWrappedObject, QIODevice*  dev, const QString&  file, QFtp::TransferType  type = QFtp::Binary);
   int  put(QFtp* theWrappedObject, const QByteArray&  data, const QString&  file, QFtp::TransferType  type = QFtp::Binary);
   int  rawCommand(QFtp* theWrappedObject, const QString&  command);
   qint64  read(QFtp* theWrappedObject, char*  data, qint64  maxlen);
   QByteArray  readAll(QFtp* theWrappedObject);
   int  remove(QFtp* theWrappedObject, const QString&  file);
   int  rename(QFtp* theWrappedObject, const QString&  oldname, const QString&  newname);
   int  rmdir(QFtp* theWrappedObject, const QString&  dir);
   int  setProxy(QFtp* theWrappedObject, const QString&  host, unsigned short  port);
   int  setTransferMode(QFtp* theWrappedObject, QFtp::TransferMode  mode);
   QFtp::State  state(QFtp* theWrappedObject) const;
};





class PythonQtWrapper_QHostAddress : public QObject
{ Q_OBJECT
public:
Q_ENUMS(SpecialAddress )
enum SpecialAddress{
  Null = QHostAddress::Null,   Broadcast = QHostAddress::Broadcast,   LocalHost = QHostAddress::LocalHost,   LocalHostIPv6 = QHostAddress::LocalHostIPv6,   Any = QHostAddress::Any,   AnyIPv6 = QHostAddress::AnyIPv6};
public slots:
QHostAddress* new_QHostAddress();
QHostAddress* new_QHostAddress(QHostAddress::SpecialAddress  address);
QHostAddress* new_QHostAddress(const QHostAddress&  copy);
QHostAddress* new_QHostAddress(const QIPv6Address&  ip6Addr);
QHostAddress* new_QHostAddress(const QString&  address);
QHostAddress* new_QHostAddress(unsigned int  ip4Addr);
void delete_QHostAddress(QHostAddress* obj) { delete obj; } 
   void clear(QHostAddress* theWrappedObject);
   bool  isInSubnet(QHostAddress* theWrappedObject, const QHostAddress&  subnet, int  netmask) const;
   bool  isInSubnet(QHostAddress* theWrappedObject, const QPair<QHostAddress , int >&  subnet) const;
   bool  isNull(QHostAddress* theWrappedObject) const;
   bool  __ne__(QHostAddress* theWrappedObject, QHostAddress::SpecialAddress  address) const;
   bool  __ne__(QHostAddress* theWrappedObject, const QHostAddress&  address) const;
   void writeTo(QHostAddress* theWrappedObject, QDataStream&  arg__1);
   bool  __eq__(QHostAddress* theWrappedObject, QHostAddress::SpecialAddress  address) const;
   bool  __eq__(QHostAddress* theWrappedObject, const QHostAddress&  address) const;
   void readFrom(QHostAddress* theWrappedObject, QDataStream&  arg__1);
   QPair<QHostAddress , int >  static_QHostAddress_parseSubnet(const QString&  subnet);
   QAbstractSocket::NetworkLayerProtocol  protocol(QHostAddress* theWrappedObject) const;
   QString  scopeId(QHostAddress* theWrappedObject) const;
   void setAddress(QHostAddress* theWrappedObject, const QIPv6Address&  ip6Addr);
   bool  setAddress(QHostAddress* theWrappedObject, const QString&  address);
   void setAddress(QHostAddress* theWrappedObject, unsigned int  ip4Addr);
   void setScopeId(QHostAddress* theWrappedObject, const QString&  id);
   unsigned int  toIPv4Address(QHostAddress* theWrappedObject) const;
   QIPv6Address  toIPv6Address(QHostAddress* theWrappedObject) const;
   QString  toString(QHostAddress* theWrappedObject) const;
    QString py_toString(QHostAddress*);
    bool __nonzero__(QHostAddress* obj) { return !obj->isNull(); }
};





class PythonQtWrapper_QHostInfo : public QObject
{ Q_OBJECT
public:
Q_ENUMS(HostInfoError )
enum HostInfoError{
  NoError = QHostInfo::NoError,   HostNotFound = QHostInfo::HostNotFound,   UnknownError = QHostInfo::UnknownError};
public slots:
QHostInfo* new_QHostInfo(const QHostInfo&  d);
QHostInfo* new_QHostInfo(int  lookupId = -1);
void delete_QHostInfo(QHostInfo* obj) { delete obj; } 
   void static_QHostInfo_abortHostLookup(int  lookupId);
   QList<QHostAddress >  addresses(QHostInfo* theWrappedObject) const;
   QHostInfo::HostInfoError  error(QHostInfo* theWrappedObject) const;
   QString  errorString(QHostInfo* theWrappedObject) const;
   QHostInfo  static_QHostInfo_fromName(const QString&  name);
   QString  hostName(QHostInfo* theWrappedObject) const;
   QString  static_QHostInfo_localDomainName();
   QString  static_QHostInfo_localHostName();
   int  static_QHostInfo_lookupHost(const QString&  name, QObject*  receiver, const char*  member);
   int  lookupId(QHostInfo* theWrappedObject) const;
   void setAddresses(QHostInfo* theWrappedObject, const QList<QHostAddress >&  addresses);
   void setError(QHostInfo* theWrappedObject, QHostInfo::HostInfoError  error);
   void setErrorString(QHostInfo* theWrappedObject, const QString&  errorString);
   void setHostName(QHostInfo* theWrappedObject, const QString&  name);
   void setLookupId(QHostInfo* theWrappedObject, int  id);
};





class PythonQtShell_QHttp : public QHttp
{
public:
    PythonQtShell_QHttp(QObject*  parent = 0):QHttp(parent),_wrapper(NULL) {};
    PythonQtShell_QHttp(const QString&  hostname, QHttp::ConnectionMode  mode, unsigned short  port = 0, QObject*  parent = 0):QHttp(hostname, mode, port, parent),_wrapper(NULL) {};
    PythonQtShell_QHttp(const QString&  hostname, unsigned short  port = 80, QObject*  parent = 0):QHttp(hostname, port, parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QHttp : public QObject
{ Q_OBJECT
public:
Q_ENUMS(Error ConnectionMode State )
enum Error{
  NoError = QHttp::NoError,   UnknownError = QHttp::UnknownError,   HostNotFound = QHttp::HostNotFound,   ConnectionRefused = QHttp::ConnectionRefused,   UnexpectedClose = QHttp::UnexpectedClose,   InvalidResponseHeader = QHttp::InvalidResponseHeader,   WrongContentLength = QHttp::WrongContentLength,   Aborted = QHttp::Aborted,   AuthenticationRequiredError = QHttp::AuthenticationRequiredError,   ProxyAuthenticationRequiredError = QHttp::ProxyAuthenticationRequiredError};
enum ConnectionMode{
  ConnectionModeHttp = QHttp::ConnectionModeHttp,   ConnectionModeHttps = QHttp::ConnectionModeHttps};
enum State{
  Unconnected = QHttp::Unconnected,   HostLookup = QHttp::HostLookup,   Connecting = QHttp::Connecting,   Sending = QHttp::Sending,   Reading = QHttp::Reading,   Connected = QHttp::Connected,   Closing = QHttp::Closing};
public slots:
QHttp* new_QHttp(QObject*  parent = 0);
QHttp* new_QHttp(const QString&  hostname, QHttp::ConnectionMode  mode, unsigned short  port = 0, QObject*  parent = 0);
QHttp* new_QHttp(const QString&  hostname, unsigned short  port = 80, QObject*  parent = 0);
void delete_QHttp(QHttp* obj) { delete obj; } 
   qint64  bytesAvailable(QHttp* theWrappedObject) const;
   void clearPendingRequests(QHttp* theWrappedObject);
   int  close(QHttp* theWrappedObject);
   QIODevice*  currentDestinationDevice(QHttp* theWrappedObject) const;
   int  currentId(QHttp* theWrappedObject) const;
   QHttpRequestHeader  currentRequest(QHttp* theWrappedObject) const;
   QIODevice*  currentSourceDevice(QHttp* theWrappedObject) const;
   QHttp::Error  error(QHttp* theWrappedObject) const;
   QString  errorString(QHttp* theWrappedObject) const;
   int  get(QHttp* theWrappedObject, const QString&  path, QIODevice*  to = 0);
   bool  hasPendingRequests(QHttp* theWrappedObject) const;
   int  head(QHttp* theWrappedObject, const QString&  path);
   QHttpResponseHeader  lastResponse(QHttp* theWrappedObject) const;
   int  post(QHttp* theWrappedObject, const QString&  path, QIODevice*  data, QIODevice*  to = 0);
   int  post(QHttp* theWrappedObject, const QString&  path, const QByteArray&  data, QIODevice*  to = 0);
   qint64  read(QHttp* theWrappedObject, char*  data, qint64  maxlen);
   QByteArray  readAll(QHttp* theWrappedObject);
   int  request(QHttp* theWrappedObject, const QHttpRequestHeader&  header, QIODevice*  device = 0, QIODevice*  to = 0);
   int  request(QHttp* theWrappedObject, const QHttpRequestHeader&  header, const QByteArray&  data, QIODevice*  to = 0);
   int  setHost(QHttp* theWrappedObject, const QString&  hostname, QHttp::ConnectionMode  mode, unsigned short  port = 0);
   int  setHost(QHttp* theWrappedObject, const QString&  hostname, unsigned short  port = 80);
   int  setProxy(QHttp* theWrappedObject, const QNetworkProxy&  proxy);
   int  setProxy(QHttp* theWrappedObject, const QString&  host, int  port, const QString&  username = QString(), const QString&  password = QString());
   int  setSocket(QHttp* theWrappedObject, QTcpSocket*  socket);
   int  setUser(QHttp* theWrappedObject, const QString&  username, const QString&  password = QString());
   QHttp::State  state(QHttp* theWrappedObject) const;
};





class PythonQtShell_QHttpHeader : public QHttpHeader
{
public:
    PythonQtShell_QHttpHeader():QHttpHeader(),_wrapper(NULL) {};
    PythonQtShell_QHttpHeader(const QString&  str):QHttpHeader(str),_wrapper(NULL) {};

virtual int  majorVersion() const;
virtual int  minorVersion() const;
virtual bool  parseLine(const QString&  line, int  number);
virtual QString  toString() const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QHttpHeader : public QHttpHeader
{ public:
inline bool  promoted_parseLine(const QString&  line, int  number) { return QHttpHeader::parseLine(line, number); }
inline QString  promoted_toString() const { return QHttpHeader::toString(); }
};

class PythonQtWrapper_QHttpHeader : public QObject
{ Q_OBJECT
public:
public slots:
QHttpHeader* new_QHttpHeader();
QHttpHeader* new_QHttpHeader(const QString&  str);
void delete_QHttpHeader(QHttpHeader* obj) { delete obj; } 
   void addValue(QHttpHeader* theWrappedObject, const QString&  key, const QString&  value);
   QStringList  allValues(QHttpHeader* theWrappedObject, const QString&  key) const;
   uint  contentLength(QHttpHeader* theWrappedObject) const;
   QString  contentType(QHttpHeader* theWrappedObject) const;
   bool  hasContentLength(QHttpHeader* theWrappedObject) const;
   bool  hasContentType(QHttpHeader* theWrappedObject) const;
   bool  hasKey(QHttpHeader* theWrappedObject, const QString&  key) const;
   bool  isValid(QHttpHeader* theWrappedObject) const;
   QStringList  keys(QHttpHeader* theWrappedObject) const;
   bool  parseLine(QHttpHeader* theWrappedObject, const QString&  line, int  number);
   void removeAllValues(QHttpHeader* theWrappedObject, const QString&  key);
   void removeValue(QHttpHeader* theWrappedObject, const QString&  key);
   void setContentLength(QHttpHeader* theWrappedObject, int  len);
   void setContentType(QHttpHeader* theWrappedObject, const QString&  type);
   void setValue(QHttpHeader* theWrappedObject, const QString&  key, const QString&  value);
   void setValues(QHttpHeader* theWrappedObject, const QList<QPair<QString , QString >  >&  values);
   QString  toString(QHttpHeader* theWrappedObject) const;
   QString  value(QHttpHeader* theWrappedObject, const QString&  key) const;
   QList<QPair<QString , QString >  >  values(QHttpHeader* theWrappedObject) const;
    QString py_toString(QHttpHeader*);
};





class PythonQtShell_QHttpRequestHeader : public QHttpRequestHeader
{
public:
    PythonQtShell_QHttpRequestHeader():QHttpRequestHeader(),_wrapper(NULL) {};
    PythonQtShell_QHttpRequestHeader(const QHttpRequestHeader&  header):QHttpRequestHeader(header),_wrapper(NULL) {};
    PythonQtShell_QHttpRequestHeader(const QString&  method, const QString&  path, int  majorVer = 1, int  minorVer = 1):QHttpRequestHeader(method, path, majorVer, minorVer),_wrapper(NULL) {};
    PythonQtShell_QHttpRequestHeader(const QString&  str):QHttpRequestHeader(str),_wrapper(NULL) {};

virtual int  majorVersion() const;
virtual int  minorVersion() const;
virtual bool  parseLine(const QString&  line, int  number);
virtual QString  toString() const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QHttpRequestHeader : public QHttpRequestHeader
{ public:
inline int  promoted_majorVersion() const { return QHttpRequestHeader::majorVersion(); }
inline int  promoted_minorVersion() const { return QHttpRequestHeader::minorVersion(); }
inline bool  promoted_parseLine(const QString&  line, int  number) { return QHttpRequestHeader::parseLine(line, number); }
inline QString  promoted_toString() const { return QHttpRequestHeader::toString(); }
};

class PythonQtWrapper_QHttpRequestHeader : public QObject
{ Q_OBJECT
public:
public slots:
QHttpRequestHeader* new_QHttpRequestHeader();
QHttpRequestHeader* new_QHttpRequestHeader(const QHttpRequestHeader&  header);
QHttpRequestHeader* new_QHttpRequestHeader(const QString&  method, const QString&  path, int  majorVer = 1, int  minorVer = 1);
QHttpRequestHeader* new_QHttpRequestHeader(const QString&  str);
void delete_QHttpRequestHeader(QHttpRequestHeader* obj) { delete obj; } 
   int  majorVersion(QHttpRequestHeader* theWrappedObject) const;
   QString  method(QHttpRequestHeader* theWrappedObject) const;
   int  minorVersion(QHttpRequestHeader* theWrappedObject) const;
   bool  parseLine(QHttpRequestHeader* theWrappedObject, const QString&  line, int  number);
   QString  path(QHttpRequestHeader* theWrappedObject) const;
   void setRequest(QHttpRequestHeader* theWrappedObject, const QString&  method, const QString&  path, int  majorVer = 1, int  minorVer = 1);
   QString  toString(QHttpRequestHeader* theWrappedObject) const;
    QString py_toString(QHttpRequestHeader*);
};





class PythonQtShell_QHttpResponseHeader : public QHttpResponseHeader
{
public:
    PythonQtShell_QHttpResponseHeader():QHttpResponseHeader(),_wrapper(NULL) {};
    PythonQtShell_QHttpResponseHeader(const QHttpResponseHeader&  header):QHttpResponseHeader(header),_wrapper(NULL) {};
    PythonQtShell_QHttpResponseHeader(const QString&  str):QHttpResponseHeader(str),_wrapper(NULL) {};
    PythonQtShell_QHttpResponseHeader(int  code, const QString&  text = QString(), int  majorVer = 1, int  minorVer = 1):QHttpResponseHeader(code, text, majorVer, minorVer),_wrapper(NULL) {};

virtual int  majorVersion() const;
virtual int  minorVersion() const;
virtual bool  parseLine(const QString&  line, int  number);
virtual QString  toString() const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QHttpResponseHeader : public QHttpResponseHeader
{ public:
inline int  promoted_majorVersion() const { return QHttpResponseHeader::majorVersion(); }
inline int  promoted_minorVersion() const { return QHttpResponseHeader::minorVersion(); }
inline bool  promoted_parseLine(const QString&  line, int  number) { return QHttpResponseHeader::parseLine(line, number); }
inline QString  promoted_toString() const { return QHttpResponseHeader::toString(); }
};

class PythonQtWrapper_QHttpResponseHeader : public QObject
{ Q_OBJECT
public:
public slots:
QHttpResponseHeader* new_QHttpResponseHeader();
QHttpResponseHeader* new_QHttpResponseHeader(const QHttpResponseHeader&  header);
QHttpResponseHeader* new_QHttpResponseHeader(const QString&  str);
QHttpResponseHeader* new_QHttpResponseHeader(int  code, const QString&  text = QString(), int  majorVer = 1, int  minorVer = 1);
void delete_QHttpResponseHeader(QHttpResponseHeader* obj) { delete obj; } 
   int  majorVersion(QHttpResponseHeader* theWrappedObject) const;
   int  minorVersion(QHttpResponseHeader* theWrappedObject) const;
   bool  parseLine(QHttpResponseHeader* theWrappedObject, const QString&  line, int  number);
   QString  reasonPhrase(QHttpResponseHeader* theWrappedObject) const;
   void setStatusLine(QHttpResponseHeader* theWrappedObject, int  code, const QString&  text = QString(), int  majorVer = 1, int  minorVer = 1);
   int  statusCode(QHttpResponseHeader* theWrappedObject) const;
   QString  toString(QHttpResponseHeader* theWrappedObject) const;
    QString py_toString(QHttpResponseHeader*);
};





class PythonQtShell_QIPv6Address : public QIPv6Address
{
public:
    PythonQtShell_QIPv6Address():QIPv6Address(),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QIPv6Address : public QObject
{ Q_OBJECT
public:
public slots:
QIPv6Address* new_QIPv6Address();
QIPv6Address* new_QIPv6Address(const QIPv6Address& other) {
PythonQtShell_QIPv6Address* a = new PythonQtShell_QIPv6Address();
*((QIPv6Address*)a) = other;
return a; }
void delete_QIPv6Address(QIPv6Address* obj) { delete obj; } 
};





class PythonQtShell_QLocalServer : public QLocalServer
{
public:
    PythonQtShell_QLocalServer(QObject*  parent = 0):QLocalServer(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual bool  hasPendingConnections() const;
virtual void incomingConnection(quintptr  socketDescriptor);
virtual QLocalSocket*  nextPendingConnection();
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QLocalServer : public QLocalServer
{ public:
inline bool  promoted_hasPendingConnections() const { return QLocalServer::hasPendingConnections(); }
inline void promoted_incomingConnection(quintptr  socketDescriptor) { QLocalServer::incomingConnection(socketDescriptor); }
inline QLocalSocket*  promoted_nextPendingConnection() { return QLocalServer::nextPendingConnection(); }
};

class PythonQtWrapper_QLocalServer : public QObject
{ Q_OBJECT
public:
public slots:
QLocalServer* new_QLocalServer(QObject*  parent = 0);
void delete_QLocalServer(QLocalServer* obj) { delete obj; } 
   void close(QLocalServer* theWrappedObject);
   QString  errorString(QLocalServer* theWrappedObject) const;
   QString  fullServerName(QLocalServer* theWrappedObject) const;
   bool  hasPendingConnections(QLocalServer* theWrappedObject) const;
   void incomingConnection(QLocalServer* theWrappedObject, quintptr  socketDescriptor);
   bool  isListening(QLocalServer* theWrappedObject) const;
   bool  listen(QLocalServer* theWrappedObject, const QString&  name);
   int  maxPendingConnections(QLocalServer* theWrappedObject) const;
   QLocalSocket*  nextPendingConnection(QLocalServer* theWrappedObject);
   bool  static_QLocalServer_removeServer(const QString&  name);
   QAbstractSocket::SocketError  serverError(QLocalServer* theWrappedObject) const;
   QString  serverName(QLocalServer* theWrappedObject) const;
   void setMaxPendingConnections(QLocalServer* theWrappedObject, int  numConnections);
   bool  waitForNewConnection(QLocalServer* theWrappedObject, int  msec = 0, bool*  timedOut = 0);
};





class PythonQtShell_QLocalSocket : public QLocalSocket
{
public:
    PythonQtShell_QLocalSocket(QObject*  parent = 0):QLocalSocket(parent),_wrapper(NULL) {};

virtual bool  atEnd() const;
virtual qint64  bytesAvailable() const;
virtual qint64  bytesToWrite() const;
virtual bool  canReadLine() const;
virtual void childEvent(QChildEvent*  arg__1);
virtual void close();
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual bool  isSequential() const;
virtual bool  open(QIODevice::OpenMode  mode);
virtual qint64  pos() const;
virtual qint64  readData(char*  arg__1, qint64  arg__2);
virtual qint64  readLineData(char*  data, qint64  maxlen);
virtual bool  reset();
virtual bool  seek(qint64  pos);
virtual qint64  size() const;
virtual void timerEvent(QTimerEvent*  arg__1);
virtual bool  waitForBytesWritten(int  msecs = 30000);
virtual bool  waitForReadyRead(int  msecs = 30000);
virtual qint64  writeData(const char*  arg__1, qint64  arg__2);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QLocalSocket : public QLocalSocket
{ public:
inline qint64  promoted_bytesAvailable() const { return QLocalSocket::bytesAvailable(); }
inline qint64  promoted_bytesToWrite() const { return QLocalSocket::bytesToWrite(); }
inline bool  promoted_canReadLine() const { return QLocalSocket::canReadLine(); }
inline void promoted_close() { QLocalSocket::close(); }
inline bool  promoted_isSequential() const { return QLocalSocket::isSequential(); }
inline qint64  promoted_readData(char*  arg__1, qint64  arg__2) { return QLocalSocket::readData(arg__1, arg__2); }
inline bool  promoted_waitForBytesWritten(int  msecs = 30000) { return QLocalSocket::waitForBytesWritten(msecs); }
inline bool  promoted_waitForReadyRead(int  msecs = 30000) { return QLocalSocket::waitForReadyRead(msecs); }
inline qint64  promoted_writeData(const char*  arg__1, qint64  arg__2) { return QLocalSocket::writeData(arg__1, arg__2); }
};

class PythonQtWrapper_QLocalSocket : public QObject
{ Q_OBJECT
public:
Q_ENUMS(LocalSocketState LocalSocketError )
enum LocalSocketState{
  UnconnectedState = QLocalSocket::UnconnectedState,   ConnectingState = QLocalSocket::ConnectingState,   ConnectedState = QLocalSocket::ConnectedState,   ClosingState = QLocalSocket::ClosingState};
enum LocalSocketError{
  ConnectionRefusedError = QLocalSocket::ConnectionRefusedError,   PeerClosedError = QLocalSocket::PeerClosedError,   ServerNotFoundError = QLocalSocket::ServerNotFoundError,   SocketAccessError = QLocalSocket::SocketAccessError,   SocketResourceError = QLocalSocket::SocketResourceError,   SocketTimeoutError = QLocalSocket::SocketTimeoutError,   DatagramTooLargeError = QLocalSocket::DatagramTooLargeError,   ConnectionError = QLocalSocket::ConnectionError,   UnsupportedSocketOperationError = QLocalSocket::UnsupportedSocketOperationError,   UnknownSocketError = QLocalSocket::UnknownSocketError};
public slots:
QLocalSocket* new_QLocalSocket(QObject*  parent = 0);
void delete_QLocalSocket(QLocalSocket* obj) { delete obj; } 
   void abort(QLocalSocket* theWrappedObject);
   qint64  bytesAvailable(QLocalSocket* theWrappedObject) const;
   qint64  bytesToWrite(QLocalSocket* theWrappedObject) const;
   bool  canReadLine(QLocalSocket* theWrappedObject) const;
   void close(QLocalSocket* theWrappedObject);
   void connectToServer(QLocalSocket* theWrappedObject, const QString&  name, QIODevice::OpenMode  openMode = QIODevice::ReadWrite);
   void disconnectFromServer(QLocalSocket* theWrappedObject);
   QLocalSocket::LocalSocketError  error(QLocalSocket* theWrappedObject) const;
   bool  flush(QLocalSocket* theWrappedObject);
   QString  fullServerName(QLocalSocket* theWrappedObject) const;
   bool  isSequential(QLocalSocket* theWrappedObject) const;
   bool  isValid(QLocalSocket* theWrappedObject) const;
   qint64  readBufferSize(QLocalSocket* theWrappedObject) const;
   qint64  readData(QLocalSocket* theWrappedObject, char*  arg__1, qint64  arg__2);
   QString  serverName(QLocalSocket* theWrappedObject) const;
   void setReadBufferSize(QLocalSocket* theWrappedObject, qint64  size);
   bool  setSocketDescriptor(QLocalSocket* theWrappedObject, quintptr  socketDescriptor, QLocalSocket::LocalSocketState  socketState = QLocalSocket::ConnectedState, QIODevice::OpenMode  openMode = QIODevice::ReadWrite);
   quintptr  socketDescriptor(QLocalSocket* theWrappedObject) const;
   QLocalSocket::LocalSocketState  state(QLocalSocket* theWrappedObject) const;
   bool  waitForBytesWritten(QLocalSocket* theWrappedObject, int  msecs = 30000);
   bool  waitForConnected(QLocalSocket* theWrappedObject, int  msecs = 30000);
   bool  waitForDisconnected(QLocalSocket* theWrappedObject, int  msecs = 30000);
   bool  waitForReadyRead(QLocalSocket* theWrappedObject, int  msecs = 30000);
   qint64  writeData(QLocalSocket* theWrappedObject, const char*  arg__1, qint64  arg__2);
};





class PythonQtShell_QNetworkAccessManager : public QNetworkAccessManager
{
public:
    PythonQtShell_QNetworkAccessManager(QObject*  parent = 0):QNetworkAccessManager(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual QNetworkReply*  createRequest(QNetworkAccessManager::Operation  op, const QNetworkRequest&  request, QIODevice*  outgoingData = 0);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QNetworkAccessManager : public QNetworkAccessManager
{ public:
inline QNetworkReply*  promoted_createRequest(QNetworkAccessManager::Operation  op, const QNetworkRequest&  request, QIODevice*  outgoingData = 0) { return QNetworkAccessManager::createRequest(op, request, outgoingData); }
};

class PythonQtWrapper_QNetworkAccessManager : public QObject
{ Q_OBJECT
public:
Q_ENUMS(Operation )
enum Operation{
  HeadOperation = QNetworkAccessManager::HeadOperation,   GetOperation = QNetworkAccessManager::GetOperation,   PutOperation = QNetworkAccessManager::PutOperation,   PostOperation = QNetworkAccessManager::PostOperation,   DeleteOperation = QNetworkAccessManager::DeleteOperation,   UnknownOperation = QNetworkAccessManager::UnknownOperation};
public slots:
QNetworkAccessManager* new_QNetworkAccessManager(QObject*  parent = 0);
void delete_QNetworkAccessManager(QNetworkAccessManager* obj) { delete obj; } 
   QAbstractNetworkCache*  cache(QNetworkAccessManager* theWrappedObject) const;
   QNetworkCookieJar*  cookieJar(QNetworkAccessManager* theWrappedObject) const;
   QNetworkReply*  createRequest(QNetworkAccessManager* theWrappedObject, QNetworkAccessManager::Operation  op, const QNetworkRequest&  request, QIODevice*  outgoingData = 0);
   QNetworkReply*  deleteResource(QNetworkAccessManager* theWrappedObject, const QNetworkRequest&  request);
   QNetworkReply*  get(QNetworkAccessManager* theWrappedObject, const QNetworkRequest&  request);
   QNetworkReply*  head(QNetworkAccessManager* theWrappedObject, const QNetworkRequest&  request);
   QNetworkReply*  post(QNetworkAccessManager* theWrappedObject, const QNetworkRequest&  request, QIODevice*  data);
   QNetworkReply*  post(QNetworkAccessManager* theWrappedObject, const QNetworkRequest&  request, const QByteArray&  data);
   QNetworkProxy  proxy(QNetworkAccessManager* theWrappedObject) const;
   QNetworkProxyFactory*  proxyFactory(QNetworkAccessManager* theWrappedObject) const;
   QNetworkReply*  put(QNetworkAccessManager* theWrappedObject, const QNetworkRequest&  request, QIODevice*  data);
   QNetworkReply*  put(QNetworkAccessManager* theWrappedObject, const QNetworkRequest&  request, const QByteArray&  data);
   void setCache(QNetworkAccessManager* theWrappedObject, QAbstractNetworkCache*  cache);
   void setCookieJar(QNetworkAccessManager* theWrappedObject, QNetworkCookieJar*  cookieJar);
   void setProxy(QNetworkAccessManager* theWrappedObject, const QNetworkProxy&  proxy);
   void setProxyFactory(QNetworkAccessManager* theWrappedObject, QNetworkProxyFactory*  factory);
};





class PythonQtWrapper_QNetworkAddressEntry : public QObject
{ Q_OBJECT
public:
public slots:
QNetworkAddressEntry* new_QNetworkAddressEntry();
QNetworkAddressEntry* new_QNetworkAddressEntry(const QNetworkAddressEntry&  other);
void delete_QNetworkAddressEntry(QNetworkAddressEntry* obj) { delete obj; } 
   QHostAddress  broadcast(QNetworkAddressEntry* theWrappedObject) const;
   QHostAddress  ip(QNetworkAddressEntry* theWrappedObject) const;
   QHostAddress  netmask(QNetworkAddressEntry* theWrappedObject) const;
   bool  __ne__(QNetworkAddressEntry* theWrappedObject, const QNetworkAddressEntry&  other) const;
   bool  __eq__(QNetworkAddressEntry* theWrappedObject, const QNetworkAddressEntry&  other) const;
   int  prefixLength(QNetworkAddressEntry* theWrappedObject) const;
   void setBroadcast(QNetworkAddressEntry* theWrappedObject, const QHostAddress&  newBroadcast);
   void setIp(QNetworkAddressEntry* theWrappedObject, const QHostAddress&  newIp);
   void setNetmask(QNetworkAddressEntry* theWrappedObject, const QHostAddress&  newNetmask);
   void setPrefixLength(QNetworkAddressEntry* theWrappedObject, int  length);
};





class PythonQtWrapper_QNetworkCacheMetaData : public QObject
{ Q_OBJECT
public:
public slots:
QNetworkCacheMetaData* new_QNetworkCacheMetaData();
QNetworkCacheMetaData* new_QNetworkCacheMetaData(const QNetworkCacheMetaData&  other);
void delete_QNetworkCacheMetaData(QNetworkCacheMetaData* obj) { delete obj; } 
   QHash<QNetworkRequest::Attribute , QVariant >  attributes(QNetworkCacheMetaData* theWrappedObject) const;
   QDateTime  expirationDate(QNetworkCacheMetaData* theWrappedObject) const;
   bool  isValid(QNetworkCacheMetaData* theWrappedObject) const;
   QDateTime  lastModified(QNetworkCacheMetaData* theWrappedObject) const;
   bool  __ne__(QNetworkCacheMetaData* theWrappedObject, const QNetworkCacheMetaData&  other) const;
   void writeTo(QNetworkCacheMetaData* theWrappedObject, QDataStream&  arg__1);
   bool  __eq__(QNetworkCacheMetaData* theWrappedObject, const QNetworkCacheMetaData&  other) const;
   void readFrom(QNetworkCacheMetaData* theWrappedObject, QDataStream&  arg__1);
   QList<QPair<QByteArray , QByteArray >  >  rawHeaders(QNetworkCacheMetaData* theWrappedObject) const;
   bool  saveToDisk(QNetworkCacheMetaData* theWrappedObject) const;
   void setAttributes(QNetworkCacheMetaData* theWrappedObject, const QHash<QNetworkRequest::Attribute , QVariant >&  attributes);
   void setExpirationDate(QNetworkCacheMetaData* theWrappedObject, const QDateTime&  dateTime);
   void setLastModified(QNetworkCacheMetaData* theWrappedObject, const QDateTime&  dateTime);
   void setRawHeaders(QNetworkCacheMetaData* theWrappedObject, const QList<QPair<QByteArray , QByteArray >  >&  headers);
   void setSaveToDisk(QNetworkCacheMetaData* theWrappedObject, bool  allow);
   void setUrl(QNetworkCacheMetaData* theWrappedObject, const QUrl&  url);
   QUrl  url(QNetworkCacheMetaData* theWrappedObject) const;
};





class PythonQtWrapper_QNetworkCookie : public QObject
{ Q_OBJECT
public:
Q_ENUMS(RawForm )
enum RawForm{
  NameAndValueOnly = QNetworkCookie::NameAndValueOnly,   Full = QNetworkCookie::Full};
public slots:
QNetworkCookie* new_QNetworkCookie(const QByteArray&  name = QByteArray(), const QByteArray&  value = QByteArray());
QNetworkCookie* new_QNetworkCookie(const QNetworkCookie&  other);
void delete_QNetworkCookie(QNetworkCookie* obj) { delete obj; } 
   QString  domain(QNetworkCookie* theWrappedObject) const;
   QDateTime  expirationDate(QNetworkCookie* theWrappedObject) const;
   bool  isHttpOnly(QNetworkCookie* theWrappedObject) const;
   bool  isSecure(QNetworkCookie* theWrappedObject) const;
   bool  isSessionCookie(QNetworkCookie* theWrappedObject) const;
   QByteArray  name(QNetworkCookie* theWrappedObject) const;
   bool  __ne__(QNetworkCookie* theWrappedObject, const QNetworkCookie&  other) const;
   bool  __eq__(QNetworkCookie* theWrappedObject, const QNetworkCookie&  other) const;
   QList<QNetworkCookie >  static_QNetworkCookie_parseCookies(const QByteArray&  cookieString);
   QString  path(QNetworkCookie* theWrappedObject) const;
   void setDomain(QNetworkCookie* theWrappedObject, const QString&  domain);
   void setExpirationDate(QNetworkCookie* theWrappedObject, const QDateTime&  date);
   void setHttpOnly(QNetworkCookie* theWrappedObject, bool  enable);
   void setName(QNetworkCookie* theWrappedObject, const QByteArray&  cookieName);
   void setPath(QNetworkCookie* theWrappedObject, const QString&  path);
   void setSecure(QNetworkCookie* theWrappedObject, bool  enable);
   void setValue(QNetworkCookie* theWrappedObject, const QByteArray&  value);
   QByteArray  toRawForm(QNetworkCookie* theWrappedObject, QNetworkCookie::RawForm  form = QNetworkCookie::Full) const;
   QByteArray  value(QNetworkCookie* theWrappedObject) const;
    QString py_toString(QNetworkCookie*);
};





class PythonQtShell_QNetworkCookieJar : public QNetworkCookieJar
{
public:
    PythonQtShell_QNetworkCookieJar(QObject*  parent = 0):QNetworkCookieJar(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual QList<QNetworkCookie >  cookiesForUrl(const QUrl&  url) const;
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual bool  setCookiesFromUrl(const QList<QNetworkCookie >&  cookieList, const QUrl&  url);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QNetworkCookieJar : public QNetworkCookieJar
{ public:
inline QList<QNetworkCookie >  promoted_cookiesForUrl(const QUrl&  url) const { return QNetworkCookieJar::cookiesForUrl(url); }
inline bool  promoted_setCookiesFromUrl(const QList<QNetworkCookie >&  cookieList, const QUrl&  url) { return QNetworkCookieJar::setCookiesFromUrl(cookieList, url); }
};

class PythonQtWrapper_QNetworkCookieJar : public QObject
{ Q_OBJECT
public:
public slots:
QNetworkCookieJar* new_QNetworkCookieJar(QObject*  parent = 0);
void delete_QNetworkCookieJar(QNetworkCookieJar* obj) { delete obj; } 
   QList<QNetworkCookie >  cookiesForUrl(QNetworkCookieJar* theWrappedObject, const QUrl&  url) const;
   bool  setCookiesFromUrl(QNetworkCookieJar* theWrappedObject, const QList<QNetworkCookie >&  cookieList, const QUrl&  url);
};





class PythonQtShell_QNetworkDiskCache : public QNetworkDiskCache
{
public:
    PythonQtShell_QNetworkDiskCache(QObject*  parent = 0):QNetworkDiskCache(parent),_wrapper(NULL) {};

virtual qint64  cacheSize() const;
virtual void childEvent(QChildEvent*  arg__1);
virtual void clear();
virtual void customEvent(QEvent*  arg__1);
virtual QIODevice*  data(const QUrl&  url);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual qint64  expire();
virtual void insert(QIODevice*  device);
virtual QNetworkCacheMetaData  metaData(const QUrl&  url);
virtual QIODevice*  prepare(const QNetworkCacheMetaData&  metaData);
virtual bool  remove(const QUrl&  url);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void updateMetaData(const QNetworkCacheMetaData&  metaData);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QNetworkDiskCache : public QNetworkDiskCache
{ public:
inline qint64  promoted_cacheSize() const { return QNetworkDiskCache::cacheSize(); }
inline void promoted_clear() { QNetworkDiskCache::clear(); }
inline QIODevice*  promoted_data(const QUrl&  url) { return QNetworkDiskCache::data(url); }
inline qint64  promoted_expire() { return QNetworkDiskCache::expire(); }
inline void promoted_insert(QIODevice*  device) { QNetworkDiskCache::insert(device); }
inline QNetworkCacheMetaData  promoted_metaData(const QUrl&  url) { return QNetworkDiskCache::metaData(url); }
inline QIODevice*  promoted_prepare(const QNetworkCacheMetaData&  metaData) { return QNetworkDiskCache::prepare(metaData); }
inline bool  promoted_remove(const QUrl&  url) { return QNetworkDiskCache::remove(url); }
inline void promoted_updateMetaData(const QNetworkCacheMetaData&  metaData) { QNetworkDiskCache::updateMetaData(metaData); }
};

class PythonQtWrapper_QNetworkDiskCache : public QObject
{ Q_OBJECT
public:
public slots:
QNetworkDiskCache* new_QNetworkDiskCache(QObject*  parent = 0);
void delete_QNetworkDiskCache(QNetworkDiskCache* obj) { delete obj; } 
   QString  cacheDirectory(QNetworkDiskCache* theWrappedObject) const;
   qint64  cacheSize(QNetworkDiskCache* theWrappedObject) const;
   QIODevice*  data(QNetworkDiskCache* theWrappedObject, const QUrl&  url);
   qint64  expire(QNetworkDiskCache* theWrappedObject);
   QNetworkCacheMetaData  fileMetaData(QNetworkDiskCache* theWrappedObject, const QString&  fileName) const;
   void insert(QNetworkDiskCache* theWrappedObject, QIODevice*  device);
   qint64  maximumCacheSize(QNetworkDiskCache* theWrappedObject) const;
   QNetworkCacheMetaData  metaData(QNetworkDiskCache* theWrappedObject, const QUrl&  url);
   QIODevice*  prepare(QNetworkDiskCache* theWrappedObject, const QNetworkCacheMetaData&  metaData);
   bool  remove(QNetworkDiskCache* theWrappedObject, const QUrl&  url);
   void setCacheDirectory(QNetworkDiskCache* theWrappedObject, const QString&  cacheDir);
   void setMaximumCacheSize(QNetworkDiskCache* theWrappedObject, qint64  size);
   void updateMetaData(QNetworkDiskCache* theWrappedObject, const QNetworkCacheMetaData&  metaData);
};





class PythonQtWrapper_QNetworkInterface : public QObject
{ Q_OBJECT
public:
Q_ENUMS(InterfaceFlag )
Q_FLAGS(InterfaceFlags )
enum InterfaceFlag{
  IsUp = QNetworkInterface::IsUp,   IsRunning = QNetworkInterface::IsRunning,   CanBroadcast = QNetworkInterface::CanBroadcast,   IsLoopBack = QNetworkInterface::IsLoopBack,   IsPointToPoint = QNetworkInterface::IsPointToPoint,   CanMulticast = QNetworkInterface::CanMulticast};
Q_DECLARE_FLAGS(InterfaceFlags, InterfaceFlag)
public slots:
QNetworkInterface* new_QNetworkInterface();
QNetworkInterface* new_QNetworkInterface(const QNetworkInterface&  other);
void delete_QNetworkInterface(QNetworkInterface* obj) { delete obj; } 
   QList<QNetworkAddressEntry >  addressEntries(QNetworkInterface* theWrappedObject) const;
   QList<QHostAddress >  static_QNetworkInterface_allAddresses();
   QList<QNetworkInterface >  static_QNetworkInterface_allInterfaces();
   QNetworkInterface::InterfaceFlags  flags(QNetworkInterface* theWrappedObject) const;
   QString  hardwareAddress(QNetworkInterface* theWrappedObject) const;
   QString  humanReadableName(QNetworkInterface* theWrappedObject) const;
   int  index(QNetworkInterface* theWrappedObject) const;
   QNetworkInterface  static_QNetworkInterface_interfaceFromIndex(int  index);
   QNetworkInterface  static_QNetworkInterface_interfaceFromName(const QString&  name);
   bool  isValid(QNetworkInterface* theWrappedObject) const;
   QString  name(QNetworkInterface* theWrappedObject) const;
    QString py_toString(QNetworkInterface*);
};





class PythonQtWrapper_QNetworkProxy : public QObject
{ Q_OBJECT
public:
Q_ENUMS(ProxyType Capability )
Q_FLAGS(Capabilities )
enum ProxyType{
  DefaultProxy = QNetworkProxy::DefaultProxy,   Socks5Proxy = QNetworkProxy::Socks5Proxy,   NoProxy = QNetworkProxy::NoProxy,   HttpProxy = QNetworkProxy::HttpProxy,   HttpCachingProxy = QNetworkProxy::HttpCachingProxy,   FtpCachingProxy = QNetworkProxy::FtpCachingProxy};
enum Capability{
  TunnelingCapability = QNetworkProxy::TunnelingCapability,   ListeningCapability = QNetworkProxy::ListeningCapability,   UdpTunnelingCapability = QNetworkProxy::UdpTunnelingCapability,   CachingCapability = QNetworkProxy::CachingCapability,   HostNameLookupCapability = QNetworkProxy::HostNameLookupCapability};
Q_DECLARE_FLAGS(Capabilities, Capability)
public slots:
QNetworkProxy* new_QNetworkProxy();
QNetworkProxy* new_QNetworkProxy(QNetworkProxy::ProxyType  type, const QString&  hostName = QString(), unsigned short  port = 0, const QString&  user = QString(), const QString&  password = QString());
QNetworkProxy* new_QNetworkProxy(const QNetworkProxy&  other);
void delete_QNetworkProxy(QNetworkProxy* obj) { delete obj; } 
   QNetworkProxy  static_QNetworkProxy_applicationProxy();
   QNetworkProxy::Capabilities  capabilities(QNetworkProxy* theWrappedObject) const;
   QString  hostName(QNetworkProxy* theWrappedObject) const;
   bool  isCachingProxy(QNetworkProxy* theWrappedObject) const;
   bool  isTransparentProxy(QNetworkProxy* theWrappedObject) const;
   bool  __ne__(QNetworkProxy* theWrappedObject, const QNetworkProxy&  other) const;
   bool  __eq__(QNetworkProxy* theWrappedObject, const QNetworkProxy&  other) const;
   QString  password(QNetworkProxy* theWrappedObject) const;
   unsigned short  port(QNetworkProxy* theWrappedObject) const;
   void static_QNetworkProxy_setApplicationProxy(const QNetworkProxy&  proxy);
   void setCapabilities(QNetworkProxy* theWrappedObject, QNetworkProxy::Capabilities  capab);
   void setHostName(QNetworkProxy* theWrappedObject, const QString&  hostName);
   void setPassword(QNetworkProxy* theWrappedObject, const QString&  password);
   void setPort(QNetworkProxy* theWrappedObject, unsigned short  port);
   void setType(QNetworkProxy* theWrappedObject, QNetworkProxy::ProxyType  type);
   void setUser(QNetworkProxy* theWrappedObject, const QString&  userName);
   QNetworkProxy::ProxyType  type(QNetworkProxy* theWrappedObject) const;
   QString  user(QNetworkProxy* theWrappedObject) const;
};





class PythonQtShell_QNetworkProxyFactory : public QNetworkProxyFactory
{
public:
    PythonQtShell_QNetworkProxyFactory():QNetworkProxyFactory(),_wrapper(NULL) {};

virtual QList<QNetworkProxy >  queryProxy(const QNetworkProxyQuery&  query = QNetworkProxyQuery());

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QNetworkProxyFactory : public QObject
{ Q_OBJECT
public:
public slots:
QNetworkProxyFactory* new_QNetworkProxyFactory();
void delete_QNetworkProxyFactory(QNetworkProxyFactory* obj) { delete obj; } 
   QList<QNetworkProxy >  static_QNetworkProxyFactory_proxyForQuery(const QNetworkProxyQuery&  query);
   void static_QNetworkProxyFactory_setApplicationProxyFactory(QNetworkProxyFactory*  factory);
   void static_QNetworkProxyFactory_setUseSystemConfiguration(bool  enable);
   QList<QNetworkProxy >  static_QNetworkProxyFactory_systemProxyForQuery(const QNetworkProxyQuery&  query = QNetworkProxyQuery());
};





class PythonQtWrapper_QNetworkProxyQuery : public QObject
{ Q_OBJECT
public:
Q_ENUMS(QueryType )
enum QueryType{
  TcpSocket = QNetworkProxyQuery::TcpSocket,   UdpSocket = QNetworkProxyQuery::UdpSocket,   TcpServer = QNetworkProxyQuery::TcpServer,   UrlRequest = QNetworkProxyQuery::UrlRequest};
public slots:
QNetworkProxyQuery* new_QNetworkProxyQuery();
QNetworkProxyQuery* new_QNetworkProxyQuery(const QNetworkProxyQuery&  other);
QNetworkProxyQuery* new_QNetworkProxyQuery(const QString&  hostname, int  port, const QString&  protocolTag = QString(), QNetworkProxyQuery::QueryType  queryType = QNetworkProxyQuery::TcpSocket);
QNetworkProxyQuery* new_QNetworkProxyQuery(const QUrl&  requestUrl, QNetworkProxyQuery::QueryType  queryType = QNetworkProxyQuery::UrlRequest);
QNetworkProxyQuery* new_QNetworkProxyQuery(unsigned short  bindPort, const QString&  protocolTag = QString(), QNetworkProxyQuery::QueryType  queryType = QNetworkProxyQuery::TcpServer);
void delete_QNetworkProxyQuery(QNetworkProxyQuery* obj) { delete obj; } 
   int  localPort(QNetworkProxyQuery* theWrappedObject) const;
   bool  __ne__(QNetworkProxyQuery* theWrappedObject, const QNetworkProxyQuery&  other) const;
   bool  __eq__(QNetworkProxyQuery* theWrappedObject, const QNetworkProxyQuery&  other) const;
   QString  peerHostName(QNetworkProxyQuery* theWrappedObject) const;
   int  peerPort(QNetworkProxyQuery* theWrappedObject) const;
   QString  protocolTag(QNetworkProxyQuery* theWrappedObject) const;
   QNetworkProxyQuery::QueryType  queryType(QNetworkProxyQuery* theWrappedObject) const;
   void setLocalPort(QNetworkProxyQuery* theWrappedObject, int  port);
   void setPeerHostName(QNetworkProxyQuery* theWrappedObject, const QString&  hostname);
   void setPeerPort(QNetworkProxyQuery* theWrappedObject, int  port);
   void setProtocolTag(QNetworkProxyQuery* theWrappedObject, const QString&  protocolTag);
   void setQueryType(QNetworkProxyQuery* theWrappedObject, QNetworkProxyQuery::QueryType  type);
   void setUrl(QNetworkProxyQuery* theWrappedObject, const QUrl&  url);
   QUrl  url(QNetworkProxyQuery* theWrappedObject) const;
};





class PythonQtShell_QNetworkReply : public QNetworkReply
{
public:
    PythonQtShell_QNetworkReply(QObject*  parent = 0):QNetworkReply(parent),_wrapper(NULL) {};

virtual void abort();
virtual bool  atEnd() const;
virtual qint64  bytesAvailable() const;
virtual qint64  bytesToWrite() const;
virtual bool  canReadLine() const;
virtual void childEvent(QChildEvent*  arg__1);
virtual void close();
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void ignoreSslErrors();
virtual bool  isSequential() const;
virtual bool  open(QIODevice::OpenMode  mode);
virtual qint64  pos() const;
virtual qint64  readData(char*  data, qint64  maxlen);
virtual qint64  readLineData(char*  data, qint64  maxlen);
virtual bool  reset();
virtual bool  seek(qint64  pos);
virtual void setReadBufferSize(qint64  size);
virtual qint64  size() const;
virtual void timerEvent(QTimerEvent*  arg__1);
virtual bool  waitForBytesWritten(int  msecs);
virtual bool  waitForReadyRead(int  msecs);
virtual qint64  writeData(const char*  data, qint64  len);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QNetworkReply : public QNetworkReply
{ public:
inline void promoted_close() { QNetworkReply::close(); }
inline void promoted_ignoreSslErrors() { QNetworkReply::ignoreSslErrors(); }
inline bool  promoted_isSequential() const { return QNetworkReply::isSequential(); }
inline void promoted_setReadBufferSize(qint64  size) { QNetworkReply::setReadBufferSize(size); }
inline qint64  promoted_writeData(const char*  data, qint64  len) { return QNetworkReply::writeData(data, len); }
};

class PythonQtWrapper_QNetworkReply : public QObject
{ Q_OBJECT
public:
public slots:
void delete_QNetworkReply(QNetworkReply* obj) { delete obj; } 
   QVariant  attribute(QNetworkReply* theWrappedObject, QNetworkRequest::Attribute  code) const;
   void close(QNetworkReply* theWrappedObject);
   QNetworkReply::NetworkError  error(QNetworkReply* theWrappedObject) const;
   bool  hasRawHeader(QNetworkReply* theWrappedObject, const QByteArray&  headerName) const;
   QVariant  header(QNetworkReply* theWrappedObject, QNetworkRequest::KnownHeaders  header) const;
   bool  isFinished(QNetworkReply* theWrappedObject) const;
   bool  isRunning(QNetworkReply* theWrappedObject) const;
   bool  isSequential(QNetworkReply* theWrappedObject) const;
   QNetworkAccessManager*  manager(QNetworkReply* theWrappedObject) const;
   QNetworkAccessManager::Operation  operation(QNetworkReply* theWrappedObject) const;
   QByteArray  rawHeader(QNetworkReply* theWrappedObject, const QByteArray&  headerName) const;
   QList<QByteArray >  rawHeaderList(QNetworkReply* theWrappedObject) const;
   qint64  readBufferSize(QNetworkReply* theWrappedObject) const;
   QNetworkRequest  request(QNetworkReply* theWrappedObject) const;
   void setReadBufferSize(QNetworkReply* theWrappedObject, qint64  size);
   QUrl  url(QNetworkReply* theWrappedObject) const;
   qint64  writeData(QNetworkReply* theWrappedObject, const char*  data, qint64  len);
};





class PythonQtWrapper_QNetworkRequest : public QObject
{ Q_OBJECT
public:
Q_ENUMS(CacheLoadControl KnownHeaders Attribute )
enum CacheLoadControl{
  AlwaysNetwork = QNetworkRequest::AlwaysNetwork,   PreferNetwork = QNetworkRequest::PreferNetwork,   PreferCache = QNetworkRequest::PreferCache,   AlwaysCache = QNetworkRequest::AlwaysCache};
enum KnownHeaders{
  ContentTypeHeader = QNetworkRequest::ContentTypeHeader,   ContentLengthHeader = QNetworkRequest::ContentLengthHeader,   LocationHeader = QNetworkRequest::LocationHeader,   LastModifiedHeader = QNetworkRequest::LastModifiedHeader,   CookieHeader = QNetworkRequest::CookieHeader,   SetCookieHeader = QNetworkRequest::SetCookieHeader};
enum Attribute{
  HttpStatusCodeAttribute = QNetworkRequest::HttpStatusCodeAttribute,   HttpReasonPhraseAttribute = QNetworkRequest::HttpReasonPhraseAttribute,   RedirectionTargetAttribute = QNetworkRequest::RedirectionTargetAttribute,   ConnectionEncryptedAttribute = QNetworkRequest::ConnectionEncryptedAttribute,   CacheLoadControlAttribute = QNetworkRequest::CacheLoadControlAttribute,   CacheSaveControlAttribute = QNetworkRequest::CacheSaveControlAttribute,   SourceIsFromCacheAttribute = QNetworkRequest::SourceIsFromCacheAttribute,   DoNotBufferUploadDataAttribute = QNetworkRequest::DoNotBufferUploadDataAttribute,   HttpPipeliningAllowedAttribute = QNetworkRequest::HttpPipeliningAllowedAttribute,   HttpPipeliningWasUsedAttribute = QNetworkRequest::HttpPipeliningWasUsedAttribute,   User = QNetworkRequest::User,   UserMax = QNetworkRequest::UserMax};
public slots:
QNetworkRequest* new_QNetworkRequest(const QNetworkRequest&  other);
QNetworkRequest* new_QNetworkRequest(const QUrl&  url = QUrl());
void delete_QNetworkRequest(QNetworkRequest* obj) { delete obj; } 
   QVariant  attribute(QNetworkRequest* theWrappedObject, QNetworkRequest::Attribute  code, const QVariant&  defaultValue = QVariant()) const;
   bool  hasRawHeader(QNetworkRequest* theWrappedObject, const QByteArray&  headerName) const;
   QVariant  header(QNetworkRequest* theWrappedObject, QNetworkRequest::KnownHeaders  header) const;
   bool  __ne__(QNetworkRequest* theWrappedObject, const QNetworkRequest&  other) const;
   bool  __eq__(QNetworkRequest* theWrappedObject, const QNetworkRequest&  other) const;
   QObject*  originatingObject(QNetworkRequest* theWrappedObject) const;
   QByteArray  rawHeader(QNetworkRequest* theWrappedObject, const QByteArray&  headerName) const;
   QList<QByteArray >  rawHeaderList(QNetworkRequest* theWrappedObject) const;
   void setAttribute(QNetworkRequest* theWrappedObject, QNetworkRequest::Attribute  code, const QVariant&  value);
   void setHeader(QNetworkRequest* theWrappedObject, QNetworkRequest::KnownHeaders  header, const QVariant&  value);
   void setOriginatingObject(QNetworkRequest* theWrappedObject, QObject*  object);
   void setRawHeader(QNetworkRequest* theWrappedObject, const QByteArray&  headerName, const QByteArray&  value);
   void setUrl(QNetworkRequest* theWrappedObject, const QUrl&  url);
   QUrl  url(QNetworkRequest* theWrappedObject) const;
};





class PythonQtWrapper_QSsl : public QObject
{ Q_OBJECT
public:
Q_ENUMS(KeyType KeyAlgorithm SslProtocol AlternateNameEntryType EncodingFormat )
enum KeyType{
  PrivateKey = QSsl::PrivateKey,   PublicKey = QSsl::PublicKey};
enum KeyAlgorithm{
  Rsa = QSsl::Rsa,   Dsa = QSsl::Dsa};
enum SslProtocol{
  SslV3 = QSsl::SslV3,   SslV2 = QSsl::SslV2,   TlsV1 = QSsl::TlsV1,   AnyProtocol = QSsl::AnyProtocol,   UnknownProtocol = QSsl::UnknownProtocol};
enum AlternateNameEntryType{
  EmailEntry = QSsl::EmailEntry,   DnsEntry = QSsl::DnsEntry};
enum EncodingFormat{
  Pem = QSsl::Pem,   Der = QSsl::Der};
public slots:
};





class PythonQtShell_QTcpServer : public QTcpServer
{
public:
    PythonQtShell_QTcpServer(QObject*  parent = 0):QTcpServer(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual bool  hasPendingConnections() const;
virtual void incomingConnection(int  handle);
virtual QTcpSocket*  nextPendingConnection();
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QTcpServer : public QTcpServer
{ public:
inline bool  promoted_hasPendingConnections() const { return QTcpServer::hasPendingConnections(); }
inline void promoted_incomingConnection(int  handle) { QTcpServer::incomingConnection(handle); }
inline QTcpSocket*  promoted_nextPendingConnection() { return QTcpServer::nextPendingConnection(); }
};

class PythonQtWrapper_QTcpServer : public QObject
{ Q_OBJECT
public:
public slots:
QTcpServer* new_QTcpServer(QObject*  parent = 0);
void delete_QTcpServer(QTcpServer* obj) { delete obj; } 
   void close(QTcpServer* theWrappedObject);
   QString  errorString(QTcpServer* theWrappedObject) const;
   bool  hasPendingConnections(QTcpServer* theWrappedObject) const;
   void incomingConnection(QTcpServer* theWrappedObject, int  handle);
   bool  isListening(QTcpServer* theWrappedObject) const;
   bool  listen(QTcpServer* theWrappedObject, const QHostAddress&  address = QHostAddress::Any, unsigned short  port = 0);
   int  maxPendingConnections(QTcpServer* theWrappedObject) const;
   QTcpSocket*  nextPendingConnection(QTcpServer* theWrappedObject);
   QNetworkProxy  proxy(QTcpServer* theWrappedObject) const;
   QHostAddress  serverAddress(QTcpServer* theWrappedObject) const;
   QAbstractSocket::SocketError  serverError(QTcpServer* theWrappedObject) const;
   unsigned short  serverPort(QTcpServer* theWrappedObject) const;
   void setMaxPendingConnections(QTcpServer* theWrappedObject, int  numConnections);
   void setProxy(QTcpServer* theWrappedObject, const QNetworkProxy&  networkProxy);
   bool  setSocketDescriptor(QTcpServer* theWrappedObject, int  socketDescriptor);
   int  socketDescriptor(QTcpServer* theWrappedObject) const;
   bool  waitForNewConnection(QTcpServer* theWrappedObject, int  msec = 0, bool*  timedOut = 0);
};





class PythonQtShell_QTcpSocket : public QTcpSocket
{
public:
    PythonQtShell_QTcpSocket(QObject*  parent = 0):QTcpSocket(parent),_wrapper(NULL) {};

virtual bool  atEnd() const;
virtual qint64  bytesAvailable() const;
virtual qint64  bytesToWrite() const;
virtual bool  canReadLine() const;
virtual void childEvent(QChildEvent*  arg__1);
virtual void close();
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual bool  isSequential() const;
virtual bool  open(QIODevice::OpenMode  mode);
virtual qint64  pos() const;
virtual qint64  readData(char*  data, qint64  maxlen);
virtual qint64  readLineData(char*  data, qint64  maxlen);
virtual bool  reset();
virtual bool  seek(qint64  pos);
virtual qint64  size() const;
virtual void timerEvent(QTimerEvent*  arg__1);
virtual bool  waitForBytesWritten(int  msecs);
virtual bool  waitForReadyRead(int  msecs);
virtual qint64  writeData(const char*  data, qint64  len);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QTcpSocket : public QObject
{ Q_OBJECT
public:
public slots:
QTcpSocket* new_QTcpSocket(QObject*  parent = 0);
void delete_QTcpSocket(QTcpSocket* obj) { delete obj; } 
};





class PythonQtShell_QUdpSocket : public QUdpSocket
{
public:
    PythonQtShell_QUdpSocket(QObject*  parent = 0):QUdpSocket(parent),_wrapper(NULL) {};

virtual bool  atEnd() const;
virtual qint64  bytesAvailable() const;
virtual qint64  bytesToWrite() const;
virtual bool  canReadLine() const;
virtual void childEvent(QChildEvent*  arg__1);
virtual void close();
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual bool  isSequential() const;
virtual bool  open(QIODevice::OpenMode  mode);
virtual qint64  pos() const;
virtual qint64  readData(char*  data, qint64  maxlen);
virtual qint64  readLineData(char*  data, qint64  maxlen);
virtual bool  reset();
virtual bool  seek(qint64  pos);
virtual qint64  size() const;
virtual void timerEvent(QTimerEvent*  arg__1);
virtual bool  waitForBytesWritten(int  msecs);
virtual bool  waitForReadyRead(int  msecs);
virtual qint64  writeData(const char*  data, qint64  len);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QUdpSocket : public QObject
{ Q_OBJECT
public:
Q_ENUMS(BindFlag )
Q_FLAGS(BindMode )
enum BindFlag{
  DefaultForPlatform = QUdpSocket::DefaultForPlatform,   ShareAddress = QUdpSocket::ShareAddress,   DontShareAddress = QUdpSocket::DontShareAddress,   ReuseAddressHint = QUdpSocket::ReuseAddressHint};
Q_DECLARE_FLAGS(BindMode, BindFlag)
public slots:
QUdpSocket* new_QUdpSocket(QObject*  parent = 0);
void delete_QUdpSocket(QUdpSocket* obj) { delete obj; } 
   bool  bind(QUdpSocket* theWrappedObject, const QHostAddress&  address, unsigned short  port);
   bool  bind(QUdpSocket* theWrappedObject, const QHostAddress&  address, unsigned short  port, QUdpSocket::BindMode  mode);
   bool  bind(QUdpSocket* theWrappedObject, unsigned short  port = 0);
   bool  bind(QUdpSocket* theWrappedObject, unsigned short  port, QUdpSocket::BindMode  mode);
   bool  hasPendingDatagrams(QUdpSocket* theWrappedObject) const;
   qint64  pendingDatagramSize(QUdpSocket* theWrappedObject) const;
   qint64  readDatagram(QUdpSocket* theWrappedObject, char*  data, qint64  maxlen, QHostAddress*  host = 0, unsigned short*  port = 0);
   qint64  writeDatagram(QUdpSocket* theWrappedObject, const QByteArray&  datagram, const QHostAddress&  host, unsigned short  port);
};





class PythonQtShell_QUrlInfo : public QUrlInfo
{
public:
    PythonQtShell_QUrlInfo():QUrlInfo(),_wrapper(NULL) {};
    PythonQtShell_QUrlInfo(const QString&  name, int  permissions, const QString&  owner, const QString&  group, qint64  size, const QDateTime&  lastModified, const QDateTime&  lastRead, bool  isDir, bool  isFile, bool  isSymLink, bool  isWritable, bool  isReadable, bool  isExecutable):QUrlInfo(name, permissions, owner, group, size, lastModified, lastRead, isDir, isFile, isSymLink, isWritable, isReadable, isExecutable),_wrapper(NULL) {};
    PythonQtShell_QUrlInfo(const QUrl&  url, int  permissions, const QString&  owner, const QString&  group, qint64  size, const QDateTime&  lastModified, const QDateTime&  lastRead, bool  isDir, bool  isFile, bool  isSymLink, bool  isWritable, bool  isReadable, bool  isExecutable):QUrlInfo(url, permissions, owner, group, size, lastModified, lastRead, isDir, isFile, isSymLink, isWritable, isReadable, isExecutable),_wrapper(NULL) {};
    PythonQtShell_QUrlInfo(const QUrlInfo&  ui):QUrlInfo(ui),_wrapper(NULL) {};

virtual void setDir(bool  b);
virtual void setFile(bool  b);
virtual void setGroup(const QString&  s);
virtual void setLastModified(const QDateTime&  dt);
virtual void setName(const QString&  name);
virtual void setOwner(const QString&  s);
virtual void setPermissions(int  p);
virtual void setReadable(bool  b);
virtual void setSize(qint64  size);
virtual void setSymLink(bool  b);
virtual void setWritable(bool  b);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QUrlInfo : public QUrlInfo
{ public:
inline void promoted_setDir(bool  b) { QUrlInfo::setDir(b); }
inline void promoted_setFile(bool  b) { QUrlInfo::setFile(b); }
inline void promoted_setGroup(const QString&  s) { QUrlInfo::setGroup(s); }
inline void promoted_setLastModified(const QDateTime&  dt) { QUrlInfo::setLastModified(dt); }
inline void promoted_setName(const QString&  name) { QUrlInfo::setName(name); }
inline void promoted_setOwner(const QString&  s) { QUrlInfo::setOwner(s); }
inline void promoted_setPermissions(int  p) { QUrlInfo::setPermissions(p); }
inline void promoted_setReadable(bool  b) { QUrlInfo::setReadable(b); }
inline void promoted_setSize(qint64  size) { QUrlInfo::setSize(size); }
inline void promoted_setSymLink(bool  b) { QUrlInfo::setSymLink(b); }
inline void promoted_setWritable(bool  b) { QUrlInfo::setWritable(b); }
};

class PythonQtWrapper_QUrlInfo : public QObject
{ Q_OBJECT
public:
Q_ENUMS(PermissionSpec )
enum PermissionSpec{
  ReadOwner = QUrlInfo::ReadOwner,   WriteOwner = QUrlInfo::WriteOwner,   ExeOwner = QUrlInfo::ExeOwner,   ReadGroup = QUrlInfo::ReadGroup,   WriteGroup = QUrlInfo::WriteGroup,   ExeGroup = QUrlInfo::ExeGroup,   ReadOther = QUrlInfo::ReadOther,   WriteOther = QUrlInfo::WriteOther,   ExeOther = QUrlInfo::ExeOther};
public slots:
QUrlInfo* new_QUrlInfo();
QUrlInfo* new_QUrlInfo(const QString&  name, int  permissions, const QString&  owner, const QString&  group, qint64  size, const QDateTime&  lastModified, const QDateTime&  lastRead, bool  isDir, bool  isFile, bool  isSymLink, bool  isWritable, bool  isReadable, bool  isExecutable);
QUrlInfo* new_QUrlInfo(const QUrl&  url, int  permissions, const QString&  owner, const QString&  group, qint64  size, const QDateTime&  lastModified, const QDateTime&  lastRead, bool  isDir, bool  isFile, bool  isSymLink, bool  isWritable, bool  isReadable, bool  isExecutable);
QUrlInfo* new_QUrlInfo(const QUrlInfo&  ui);
void delete_QUrlInfo(QUrlInfo* obj) { delete obj; } 
   bool  static_QUrlInfo_equal(const QUrlInfo&  i1, const QUrlInfo&  i2, int  sortBy);
   bool  static_QUrlInfo_greaterThan(const QUrlInfo&  i1, const QUrlInfo&  i2, int  sortBy);
   QString  group(QUrlInfo* theWrappedObject) const;
   bool  isDir(QUrlInfo* theWrappedObject) const;
   bool  isExecutable(QUrlInfo* theWrappedObject) const;
   bool  isFile(QUrlInfo* theWrappedObject) const;
   bool  isReadable(QUrlInfo* theWrappedObject) const;
   bool  isSymLink(QUrlInfo* theWrappedObject) const;
   bool  isValid(QUrlInfo* theWrappedObject) const;
   bool  isWritable(QUrlInfo* theWrappedObject) const;
   QDateTime  lastModified(QUrlInfo* theWrappedObject) const;
   QDateTime  lastRead(QUrlInfo* theWrappedObject) const;
   bool  static_QUrlInfo_lessThan(const QUrlInfo&  i1, const QUrlInfo&  i2, int  sortBy);
   QString  name(QUrlInfo* theWrappedObject) const;
   bool  __ne__(QUrlInfo* theWrappedObject, const QUrlInfo&  i) const;
   bool  __eq__(QUrlInfo* theWrappedObject, const QUrlInfo&  i) const;
   QString  owner(QUrlInfo* theWrappedObject) const;
   int  permissions(QUrlInfo* theWrappedObject) const;
   void setDir(QUrlInfo* theWrappedObject, bool  b);
   void setFile(QUrlInfo* theWrappedObject, bool  b);
   void setGroup(QUrlInfo* theWrappedObject, const QString&  s);
   void setLastModified(QUrlInfo* theWrappedObject, const QDateTime&  dt);
   void setLastRead(QUrlInfo* theWrappedObject, const QDateTime&  dt);
   void setName(QUrlInfo* theWrappedObject, const QString&  name);
   void setOwner(QUrlInfo* theWrappedObject, const QString&  s);
   void setPermissions(QUrlInfo* theWrappedObject, int  p);
   void setReadable(QUrlInfo* theWrappedObject, bool  b);
   void setSize(QUrlInfo* theWrappedObject, qint64  size);
   void setSymLink(QUrlInfo* theWrappedObject, bool  b);
   void setWritable(QUrlInfo* theWrappedObject, bool  b);
   qint64  size(QUrlInfo* theWrappedObject) const;
};


