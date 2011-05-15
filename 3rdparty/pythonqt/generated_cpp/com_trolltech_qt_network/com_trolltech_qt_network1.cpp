#include "com_trolltech_qt_network1.h"
#include <PythonQtConversion.h>
#include <PythonQtMethodInfo.h>
#include <PythonQtSignalReceiver.h>
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

QSslKey* PythonQtWrapper_QSslKey::new_QSslKey()
{ 
return new QSslKey(); }

QSslKey* PythonQtWrapper_QSslKey::new_QSslKey(QIODevice*  device, QSsl::KeyAlgorithm  algorithm, QSsl::EncodingFormat  format, QSsl::KeyType  type, const QByteArray&  passPhrase)
{ 
return new QSslKey(device, algorithm, format, type, passPhrase); }

QSslKey* PythonQtWrapper_QSslKey::new_QSslKey(const QByteArray&  encoded, QSsl::KeyAlgorithm  algorithm, QSsl::EncodingFormat  format, QSsl::KeyType  type, const QByteArray&  passPhrase)
{ 
return new QSslKey(encoded, algorithm, format, type, passPhrase); }

QSslKey* PythonQtWrapper_QSslKey::new_QSslKey(const QSslKey&  other)
{ 
return new QSslKey(other); }

QSsl::KeyAlgorithm  PythonQtWrapper_QSslKey::algorithm(QSslKey* theWrappedObject) const
{
  return ( theWrappedObject->algorithm());
}

QByteArray  PythonQtWrapper_QSslKey::toDer(QSslKey* theWrappedObject, const QByteArray&  passPhrase) const
{
  return ( theWrappedObject->toDer(passPhrase));
}

QSslKey*  PythonQtWrapper_QSslKey::operator_assign(QSslKey* theWrappedObject, const QSslKey&  other)
{
  return &( (*theWrappedObject)= other);
}

void PythonQtWrapper_QSslKey::clear(QSslKey* theWrappedObject)
{
  ( theWrappedObject->clear());
}

int  PythonQtWrapper_QSslKey::length(QSslKey* theWrappedObject) const
{
  return ( theWrappedObject->length());
}

QByteArray  PythonQtWrapper_QSslKey::toPem(QSslKey* theWrappedObject, const QByteArray&  passPhrase) const
{
  return ( theWrappedObject->toPem(passPhrase));
}

Qt::HANDLE  PythonQtWrapper_QSslKey::handle(QSslKey* theWrappedObject) const
{
  return ( theWrappedObject->handle());
}

QSsl::KeyType  PythonQtWrapper_QSslKey::type(QSslKey* theWrappedObject) const
{
  return ( theWrappedObject->type());
}

bool  PythonQtWrapper_QSslKey::operator_equal(QSslKey* theWrappedObject, const QSslKey&  key) const
{
  return ( (*theWrappedObject)== key);
}

bool  PythonQtWrapper_QSslKey::isNull(QSslKey* theWrappedObject) const
{
  return ( theWrappedObject->isNull());
}

QString PythonQtWrapper_QSslKey::py_toString(QSslKey* obj) {
  QString result;
  QDebug d(&result);
  d << *obj;
  return result;
}



bool  PythonQtShell_QSslSocket::canReadLine() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "canReadLine");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      bool returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("canReadLine", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::canReadLine();
}
qint64  PythonQtShell_QSslSocket::bytesToWrite() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "bytesToWrite");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"qint64"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      qint64 returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("bytesToWrite", methodInfo, result);
        } else {
          returnValue = *((qint64*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::bytesToWrite();
}
qint64  PythonQtShell_QSslSocket::readData(char*  data, qint64  maxlen)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "readData");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"qint64" , "char*" , "qint64"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      qint64 returnValue;
    void* args[3] = {NULL, (void*)&data, (void*)&maxlen};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("readData", methodInfo, result);
        } else {
          returnValue = *((qint64*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::readData(data, maxlen);
}
qint64  PythonQtShell_QSslSocket::writeData(const char*  data, qint64  len)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "writeData");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"qint64" , "const char*" , "qint64"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      qint64 returnValue;
    void* args[3] = {NULL, (void*)&data, (void*)&len};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("writeData", methodInfo, result);
        } else {
          returnValue = *((qint64*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::writeData(data, len);
}
qint64  PythonQtShell_QSslSocket::bytesAvailable() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "bytesAvailable");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"qint64"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      qint64 returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("bytesAvailable", methodInfo, result);
        } else {
          returnValue = *((qint64*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::bytesAvailable();
}
bool  PythonQtShell_QSslSocket::waitForBytesWritten(int  msecs)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "waitForBytesWritten");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&msecs};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("waitForBytesWritten", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::waitForBytesWritten(msecs);
}
bool  PythonQtShell_QSslSocket::atEnd() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "atEnd");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      bool returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("atEnd", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::atEnd();
}
void PythonQtShell_QSslSocket::close()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "close");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={""};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  QSslSocket::close();
}
bool  PythonQtShell_QSslSocket::waitForReadyRead(int  msecs)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "waitForReadyRead");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&msecs};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("waitForReadyRead", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::waitForReadyRead(msecs);
}
qint64  PythonQtShell_QSslSocket::readLineData(char*  data, qint64  maxlen)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "readLineData");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"qint64" , "char*" , "qint64"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      qint64 returnValue;
    void* args[3] = {NULL, (void*)&data, (void*)&maxlen};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("readLineData", methodInfo, result);
        } else {
          returnValue = *((qint64*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::readLineData(data, maxlen);
}
bool  PythonQtShell_QSslSocket::isSequential() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "isSequential");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      bool returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("isSequential", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::isSequential();
}
qint64  PythonQtShell_QSslSocket::size() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "size");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"qint64"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      qint64 returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("size", methodInfo, result);
        } else {
          returnValue = *((qint64*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::size();
}
qint64  PythonQtShell_QSslSocket::pos() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "pos");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"qint64"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      qint64 returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("pos", methodInfo, result);
        } else {
          returnValue = *((qint64*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::pos();
}
bool  PythonQtShell_QSslSocket::reset()
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "reset");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      bool returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("reset", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::reset();
}
bool  PythonQtShell_QSslSocket::open(QIODevice::OpenMode  mode)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "open");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QIODevice::OpenMode"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&mode};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("open", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::open(mode);
}
bool  PythonQtShell_QSslSocket::seek(qint64  pos)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "seek");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "qint64"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&pos};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("seek", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::seek(pos);
}
void PythonQtShell_QSslSocket::childEvent(QChildEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "childEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QChildEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  QSslSocket::childEvent(arg__1);
}
void PythonQtShell_QSslSocket::timerEvent(QTimerEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "timerEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QTimerEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  QSslSocket::timerEvent(arg__1);
}
void PythonQtShell_QSslSocket::customEvent(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "customEvent");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  QSslSocket::customEvent(arg__1);
}
bool  PythonQtShell_QSslSocket::event(QEvent*  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("event", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::event(arg__1);
}
bool  PythonQtShell_QSslSocket::eventFilter(QObject*  arg__1, QEvent*  arg__2)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "eventFilter");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QObject*" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      bool returnValue;
    void* args[3] = {NULL, (void*)&arg__1, (void*)&arg__2};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("eventFilter", methodInfo, result);
        } else {
          returnValue = *((bool*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QSslSocket::eventFilter(arg__1, arg__2);
}
QSslSocket* PythonQtWrapper_QSslSocket::new_QSslSocket(QObject*  parent)
{ 
return new PythonQtShell_QSslSocket(parent); }

QSslSocket::SslMode  PythonQtWrapper_QSslSocket::mode(QSslSocket* theWrappedObject) const
{
  return ( theWrappedObject->mode());
}

void PythonQtWrapper_QSslSocket::setProtocol(QSslSocket* theWrappedObject, QSsl::SslProtocol  protocol)
{
  ( theWrappedObject->setProtocol(protocol));
}

void PythonQtWrapper_QSslSocket::static_QSslSocket_setDefaultCiphers(const QList<QSslCipher >&  ciphers)
{
  (QSslSocket::setDefaultCiphers(ciphers));
}

void PythonQtWrapper_QSslSocket::static_QSslSocket_setDefaultCaCertificates(const QList<QSslCertificate >&  certificates)
{
  (QSslSocket::setDefaultCaCertificates(certificates));
}

bool  PythonQtWrapper_QSslSocket::static_QSslSocket_supportsSsl()
{
  return (QSslSocket::supportsSsl());
}

void PythonQtWrapper_QSslSocket::setSocketOption(QSslSocket* theWrappedObject, QAbstractSocket::SocketOption  option, const QVariant&  value)
{
  ( theWrappedObject->setSocketOption(option, value));
}

bool  PythonQtWrapper_QSslSocket::isEncrypted(QSslSocket* theWrappedObject) const
{
  return ( theWrappedObject->isEncrypted());
}

bool  PythonQtWrapper_QSslSocket::canReadLine(QSslSocket* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_QSslSocket*)theWrappedObject)->promoted_canReadLine());
}

QSslKey  PythonQtWrapper_QSslSocket::privateKey(QSslSocket* theWrappedObject) const
{
  return ( theWrappedObject->privateKey());
}

QList<QSslCertificate >  PythonQtWrapper_QSslSocket::static_QSslSocket_defaultCaCertificates()
{
  return (QSslSocket::defaultCaCertificates());
}

QSslConfiguration  PythonQtWrapper_QSslSocket::sslConfiguration(QSslSocket* theWrappedObject) const
{
  return ( theWrappedObject->sslConfiguration());
}

qint64  PythonQtWrapper_QSslSocket::bytesToWrite(QSslSocket* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_QSslSocket*)theWrappedObject)->promoted_bytesToWrite());
}

void PythonQtWrapper_QSslSocket::setCaCertificates(QSslSocket* theWrappedObject, const QList<QSslCertificate >&  certificates)
{
  ( theWrappedObject->setCaCertificates(certificates));
}

QList<QSslCipher >  PythonQtWrapper_QSslSocket::static_QSslSocket_defaultCiphers()
{
  return (QSslSocket::defaultCiphers());
}

QList<QSslError >  PythonQtWrapper_QSslSocket::sslErrors(QSslSocket* theWrappedObject) const
{
  return ( theWrappedObject->sslErrors());
}

void PythonQtWrapper_QSslSocket::connectToHostEncrypted(QSslSocket* theWrappedObject, const QString&  hostName, unsigned short  port, const QString&  sslPeerName, QIODevice::OpenMode  mode)
{
  ( theWrappedObject->connectToHostEncrypted(hostName, port, sslPeerName, mode));
}

void PythonQtWrapper_QSslSocket::connectToHostEncrypted(QSslSocket* theWrappedObject, const QString&  hostName, unsigned short  port, QIODevice::OpenMode  mode)
{
  ( theWrappedObject->connectToHostEncrypted(hostName, port, mode));
}

void PythonQtWrapper_QSslSocket::ignoreSslErrors(QSslSocket* theWrappedObject, const QList<QSslError >&  errors)
{
  ( theWrappedObject->ignoreSslErrors(errors));
}

void PythonQtWrapper_QSslSocket::setCiphers(QSslSocket* theWrappedObject, const QString&  ciphers)
{
  ( theWrappedObject->setCiphers(ciphers));
}

qint64  PythonQtWrapper_QSslSocket::encryptedBytesAvailable(QSslSocket* theWrappedObject) const
{
  return ( theWrappedObject->encryptedBytesAvailable());
}

void PythonQtWrapper_QSslSocket::setCiphers(QSslSocket* theWrappedObject, const QList<QSslCipher >&  ciphers)
{
  ( theWrappedObject->setCiphers(ciphers));
}

void PythonQtWrapper_QSslSocket::addCaCertificates(QSslSocket* theWrappedObject, const QList<QSslCertificate >&  certificates)
{
  ( theWrappedObject->addCaCertificates(certificates));
}

bool  PythonQtWrapper_QSslSocket::addCaCertificates(QSslSocket* theWrappedObject, const QString&  path, QSsl::EncodingFormat  format, QRegExp::PatternSyntax  syntax)
{
  return ( theWrappedObject->addCaCertificates(path, format, syntax));
}

bool  PythonQtWrapper_QSslSocket::flush(QSslSocket* theWrappedObject)
{
  return ( theWrappedObject->flush());
}

QSslSocket::PeerVerifyMode  PythonQtWrapper_QSslSocket::peerVerifyMode(QSslSocket* theWrappedObject) const
{
  return ( theWrappedObject->peerVerifyMode());
}

QSslCertificate  PythonQtWrapper_QSslSocket::peerCertificate(QSslSocket* theWrappedObject) const
{
  return ( theWrappedObject->peerCertificate());
}

bool  PythonQtWrapper_QSslSocket::setSocketDescriptor(QSslSocket* theWrappedObject, int  socketDescriptor, QAbstractSocket::SocketState  state, QIODevice::OpenMode  openMode)
{
  return ( theWrappedObject->setSocketDescriptor(socketDescriptor, state, openMode));
}

QList<QSslCertificate >  PythonQtWrapper_QSslSocket::caCertificates(QSslSocket* theWrappedObject) const
{
  return ( theWrappedObject->caCertificates());
}

void PythonQtWrapper_QSslSocket::setPrivateKey(QSslSocket* theWrappedObject, const QString&  fileName, QSsl::KeyAlgorithm  algorithm, QSsl::EncodingFormat  format, const QByteArray&  passPhrase)
{
  ( theWrappedObject->setPrivateKey(fileName, algorithm, format, passPhrase));
}

qint64  PythonQtWrapper_QSslSocket::readData(QSslSocket* theWrappedObject, char*  data, qint64  maxlen)
{
  return ( ((PythonQtPublicPromoter_QSslSocket*)theWrappedObject)->promoted_readData(data, maxlen));
}

QSslCipher  PythonQtWrapper_QSslSocket::sessionCipher(QSslSocket* theWrappedObject) const
{
  return ( theWrappedObject->sessionCipher());
}

void PythonQtWrapper_QSslSocket::setPrivateKey(QSslSocket* theWrappedObject, const QSslKey&  key)
{
  ( theWrappedObject->setPrivateKey(key));
}

void PythonQtWrapper_QSslSocket::setReadBufferSize(QSslSocket* theWrappedObject, qint64  size)
{
  ( theWrappedObject->setReadBufferSize(size));
}

void PythonQtWrapper_QSslSocket::addCaCertificate(QSslSocket* theWrappedObject, const QSslCertificate&  certificate)
{
  ( theWrappedObject->addCaCertificate(certificate));
}

qint64  PythonQtWrapper_QSslSocket::writeData(QSslSocket* theWrappedObject, const char*  data, qint64  len)
{
  return ( ((PythonQtPublicPromoter_QSslSocket*)theWrappedObject)->promoted_writeData(data, len));
}

void PythonQtWrapper_QSslSocket::setSslConfiguration(QSslSocket* theWrappedObject, const QSslConfiguration&  config)
{
  ( theWrappedObject->setSslConfiguration(config));
}

void PythonQtWrapper_QSslSocket::setPeerVerifyDepth(QSslSocket* theWrappedObject, int  depth)
{
  ( theWrappedObject->setPeerVerifyDepth(depth));
}

void PythonQtWrapper_QSslSocket::abort(QSslSocket* theWrappedObject)
{
  ( theWrappedObject->abort());
}

qint64  PythonQtWrapper_QSslSocket::bytesAvailable(QSslSocket* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_QSslSocket*)theWrappedObject)->promoted_bytesAvailable());
}

bool  PythonQtWrapper_QSslSocket::waitForDisconnected(QSslSocket* theWrappedObject, int  msecs)
{
  return ( theWrappedObject->waitForDisconnected(msecs));
}

bool  PythonQtWrapper_QSslSocket::waitForBytesWritten(QSslSocket* theWrappedObject, int  msecs)
{
  return ( ((PythonQtPublicPromoter_QSslSocket*)theWrappedObject)->promoted_waitForBytesWritten(msecs));
}

bool  PythonQtWrapper_QSslSocket::atEnd(QSslSocket* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_QSslSocket*)theWrappedObject)->promoted_atEnd());
}

void PythonQtWrapper_QSslSocket::static_QSslSocket_addDefaultCaCertificate(const QSslCertificate&  certificate)
{
  (QSslSocket::addDefaultCaCertificate(certificate));
}

void PythonQtWrapper_QSslSocket::setPeerVerifyMode(QSslSocket* theWrappedObject, QSslSocket::PeerVerifyMode  mode)
{
  ( theWrappedObject->setPeerVerifyMode(mode));
}

void PythonQtWrapper_QSslSocket::setLocalCertificate(QSslSocket* theWrappedObject, const QString&  fileName, QSsl::EncodingFormat  format)
{
  ( theWrappedObject->setLocalCertificate(fileName, format));
}

void PythonQtWrapper_QSslSocket::setLocalCertificate(QSslSocket* theWrappedObject, const QSslCertificate&  certificate)
{
  ( theWrappedObject->setLocalCertificate(certificate));
}

QList<QSslCipher >  PythonQtWrapper_QSslSocket::static_QSslSocket_supportedCiphers()
{
  return (QSslSocket::supportedCiphers());
}

qint64  PythonQtWrapper_QSslSocket::encryptedBytesToWrite(QSslSocket* theWrappedObject) const
{
  return ( theWrappedObject->encryptedBytesToWrite());
}

void PythonQtWrapper_QSslSocket::close(QSslSocket* theWrappedObject)
{
  ( ((PythonQtPublicPromoter_QSslSocket*)theWrappedObject)->promoted_close());
}

bool  PythonQtWrapper_QSslSocket::waitForConnected(QSslSocket* theWrappedObject, int  msecs)
{
  return ( theWrappedObject->waitForConnected(msecs));
}

QList<QSslCertificate >  PythonQtWrapper_QSslSocket::static_QSslSocket_systemCaCertificates()
{
  return (QSslSocket::systemCaCertificates());
}

bool  PythonQtWrapper_QSslSocket::waitForReadyRead(QSslSocket* theWrappedObject, int  msecs)
{
  return ( ((PythonQtPublicPromoter_QSslSocket*)theWrappedObject)->promoted_waitForReadyRead(msecs));
}

QSslCertificate  PythonQtWrapper_QSslSocket::localCertificate(QSslSocket* theWrappedObject) const
{
  return ( theWrappedObject->localCertificate());
}

bool  PythonQtWrapper_QSslSocket::waitForEncrypted(QSslSocket* theWrappedObject, int  msecs)
{
  return ( theWrappedObject->waitForEncrypted(msecs));
}

QVariant  PythonQtWrapper_QSslSocket::socketOption(QSslSocket* theWrappedObject, QAbstractSocket::SocketOption  option)
{
  return ( theWrappedObject->socketOption(option));
}

QList<QSslCertificate >  PythonQtWrapper_QSslSocket::peerCertificateChain(QSslSocket* theWrappedObject) const
{
  return ( theWrappedObject->peerCertificateChain());
}

QSsl::SslProtocol  PythonQtWrapper_QSslSocket::protocol(QSslSocket* theWrappedObject) const
{
  return ( theWrappedObject->protocol());
}

int  PythonQtWrapper_QSslSocket::peerVerifyDepth(QSslSocket* theWrappedObject) const
{
  return ( theWrappedObject->peerVerifyDepth());
}

void PythonQtWrapper_QSslSocket::static_QSslSocket_addDefaultCaCertificates(const QList<QSslCertificate >&  certificates)
{
  (QSslSocket::addDefaultCaCertificates(certificates));
}

bool  PythonQtWrapper_QSslSocket::static_QSslSocket_addDefaultCaCertificates(const QString&  path, QSsl::EncodingFormat  format, QRegExp::PatternSyntax  syntax)
{
  return (QSslSocket::addDefaultCaCertificates(path, format, syntax));
}

QList<QSslCipher >  PythonQtWrapper_QSslSocket::ciphers(QSslSocket* theWrappedObject) const
{
  return ( theWrappedObject->ciphers());
}


