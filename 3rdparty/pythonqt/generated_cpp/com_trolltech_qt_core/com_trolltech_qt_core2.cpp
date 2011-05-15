#include "com_trolltech_qt_core2.h"
#include <PythonQtConversion.h>
#include <PythonQtMethodInfo.h>
#include <PythonQtSignalReceiver.h>
#include <QVariant>
#include <qanimationgroup.h>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qdatastream.h>
#include <qeasingcurve.h>
#include <qlist.h>
#include <qmutex.h>
#include <qobject.h>
#include <qpair.h>
#include <qreadwritelock.h>
#include <qtconcurrentreducekernel.h>
#include <qtconcurrentthreadengine.h>
#include <qtranslator.h>
#include <quuid.h>
#include <qvariantanimation.h>
#include <qvector.h>
#include <qwaitcondition.h>

QTimerEvent* PythonQtWrapper_QTimerEvent::new_QTimerEvent(int  timerId)
{ 
return new PythonQtShell_QTimerEvent(timerId); }

int  PythonQtWrapper_QTimerEvent::timerId(QTimerEvent* theWrappedObject) const
{
  return ( theWrappedObject->timerId());
}



void PythonQtShell_QTranslator::childEvent(QChildEvent*  arg__1)
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
  QTranslator::childEvent(arg__1);
}
void PythonQtShell_QTranslator::customEvent(QEvent*  arg__1)
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
  QTranslator::customEvent(arg__1);
}
bool  PythonQtShell_QTranslator::event(QEvent*  arg__1)
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
  return QTranslator::event(arg__1);
}
bool  PythonQtShell_QTranslator::eventFilter(QObject*  arg__1, QEvent*  arg__2)
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
  return QTranslator::eventFilter(arg__1, arg__2);
}
bool  PythonQtShell_QTranslator::isEmpty() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "isEmpty");
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
          PythonQt::priv()->handleVirtualOverloadReturnError("isEmpty", methodInfo, result);
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
  return QTranslator::isEmpty();
}
void PythonQtShell_QTranslator::timerEvent(QTimerEvent*  arg__1)
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
  QTranslator::timerEvent(arg__1);
}
QString  PythonQtShell_QTranslator::translate(const char*  context, const char*  sourceText, const char*  disambiguation) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "translate");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QString" , "const char*" , "const char*" , "const char*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
      QString returnValue;
    void* args[4] = {NULL, (void*)&context, (void*)&sourceText, (void*)&disambiguation};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("translate", methodInfo, result);
        } else {
          returnValue = *((QString*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QTranslator::translate(context, sourceText, disambiguation);
}
QTranslator* PythonQtWrapper_QTranslator::new_QTranslator(QObject*  parent)
{ 
return new PythonQtShell_QTranslator(parent); }

bool  PythonQtWrapper_QTranslator::isEmpty(QTranslator* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_QTranslator*)theWrappedObject)->promoted_isEmpty());
}

bool  PythonQtWrapper_QTranslator::load(QTranslator* theWrappedObject, const QString&  filename, const QString&  directory, const QString&  search_delimiters, const QString&  suffix)
{
  return ( theWrappedObject->load(filename, directory, search_delimiters, suffix));
}

bool  PythonQtWrapper_QTranslator::load(QTranslator* theWrappedObject, const uchar*  data, int  len)
{
  return ( theWrappedObject->load(data, len));
}

QString  PythonQtWrapper_QTranslator::translate(QTranslator* theWrappedObject, const char*  context, const char*  sourceText, const char*  disambiguation) const
{
  return ( ((PythonQtPublicPromoter_QTranslator*)theWrappedObject)->promoted_translate(context, sourceText, disambiguation));
}



QUuid* PythonQtWrapper_QUuid::new_QUuid()
{ 
return new PythonQtShell_QUuid(); }

QUuid* PythonQtWrapper_QUuid::new_QUuid(const QString&  arg__1)
{ 
return new PythonQtShell_QUuid(arg__1); }

QUuid* PythonQtWrapper_QUuid::new_QUuid(uint  l, ushort  w1, ushort  w2, uchar  b1, uchar  b2, uchar  b3, uchar  b4, uchar  b5, uchar  b6, uchar  b7, uchar  b8)
{ 
return new PythonQtShell_QUuid(l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8); }

QUuid  PythonQtWrapper_QUuid::static_QUuid_createUuid()
{
  return (QUuid::createUuid());
}

bool  PythonQtWrapper_QUuid::isNull(QUuid* theWrappedObject) const
{
  return ( theWrappedObject->isNull());
}

bool  PythonQtWrapper_QUuid::__ne__(QUuid* theWrappedObject, const QUuid&  orig) const
{
  return ( (*theWrappedObject)!= orig);
}

bool  PythonQtWrapper_QUuid::__lt__(QUuid* theWrappedObject, const QUuid&  other) const
{
  return ( (*theWrappedObject)< other);
}

void PythonQtWrapper_QUuid::writeTo(QUuid* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 <<  (*theWrappedObject);
}

bool  PythonQtWrapper_QUuid::__eq__(QUuid* theWrappedObject, const QUuid&  orig) const
{
  return ( (*theWrappedObject)== orig);
}

bool  PythonQtWrapper_QUuid::__gt__(QUuid* theWrappedObject, const QUuid&  other) const
{
  return ( (*theWrappedObject)> other);
}

void PythonQtWrapper_QUuid::readFrom(QUuid* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 >>  (*theWrappedObject);
}

QString  PythonQtWrapper_QUuid::toString(QUuid* theWrappedObject) const
{
  return ( theWrappedObject->toString());
}

QUuid::Variant  PythonQtWrapper_QUuid::variant(QUuid* theWrappedObject) const
{
  return ( theWrappedObject->variant());
}

QUuid::Version  PythonQtWrapper_QUuid::version(QUuid* theWrappedObject) const
{
  return ( theWrappedObject->version());
}

QString PythonQtWrapper_QUuid::py_toString(QUuid* obj) { return obj->toString(); }


void PythonQtShell_QVariantAnimation::childEvent(QChildEvent*  arg__1)
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
  QVariantAnimation::childEvent(arg__1);
}
void PythonQtShell_QVariantAnimation::customEvent(QEvent*  arg__1)
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
  QVariantAnimation::customEvent(arg__1);
}
int  PythonQtShell_QVariantAnimation::duration() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "duration");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      int returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("duration", methodInfo, result);
        } else {
          returnValue = *((int*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QVariantAnimation::duration();
}
bool  PythonQtShell_QVariantAnimation::event(QEvent*  event)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "event");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"bool" , "QEvent*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      bool returnValue;
    void* args[2] = {NULL, (void*)&event};
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
  return QVariantAnimation::event(event);
}
bool  PythonQtShell_QVariantAnimation::eventFilter(QObject*  arg__1, QEvent*  arg__2)
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
  return QVariantAnimation::eventFilter(arg__1, arg__2);
}
QVariant  PythonQtShell_QVariantAnimation::interpolated(const QVariant&  from, const QVariant&  to, qreal  progress) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "interpolated");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QVariant" , "const QVariant&" , "const QVariant&" , "qreal"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(4, argumentList);
      QVariant returnValue;
    void* args[4] = {NULL, (void*)&from, (void*)&to, (void*)&progress};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("interpolated", methodInfo, result);
        } else {
          returnValue = *((QVariant*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QVariantAnimation::interpolated(from, to, progress);
}
void PythonQtShell_QVariantAnimation::timerEvent(QTimerEvent*  arg__1)
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
  QVariantAnimation::timerEvent(arg__1);
}
void PythonQtShell_QVariantAnimation::updateCurrentTime(int  arg__1)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "updateCurrentTime");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "int"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  QVariantAnimation::updateCurrentTime(arg__1);
}
void PythonQtShell_QVariantAnimation::updateCurrentValue(const QVariant&  value)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "updateCurrentValue");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "const QVariant&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&value};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  
}
void PythonQtShell_QVariantAnimation::updateDirection(QAbstractAnimation::Direction  direction)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "updateDirection");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QAbstractAnimation::Direction"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
    void* args[2] = {NULL, (void*)&direction};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  QVariantAnimation::updateDirection(direction);
}
void PythonQtShell_QVariantAnimation::updateState(QAbstractAnimation::State  newState, QAbstractAnimation::State  oldState)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "updateState");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"" , "QAbstractAnimation::State" , "QAbstractAnimation::State"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
    void* args[3] = {NULL, (void*)&newState, (void*)&oldState};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return;
  }
}
  QVariantAnimation::updateState(newState, oldState);
}
QVariantAnimation* PythonQtWrapper_QVariantAnimation::new_QVariantAnimation(QObject*  parent)
{ 
return new PythonQtShell_QVariantAnimation(parent); }

QVariant  PythonQtWrapper_QVariantAnimation::currentValue(QVariantAnimation* theWrappedObject) const
{
  return ( theWrappedObject->currentValue());
}

int  PythonQtWrapper_QVariantAnimation::duration(QVariantAnimation* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_QVariantAnimation*)theWrappedObject)->promoted_duration());
}

QEasingCurve  PythonQtWrapper_QVariantAnimation::easingCurve(QVariantAnimation* theWrappedObject) const
{
  return ( theWrappedObject->easingCurve());
}

QVariant  PythonQtWrapper_QVariantAnimation::endValue(QVariantAnimation* theWrappedObject) const
{
  return ( theWrappedObject->endValue());
}

bool  PythonQtWrapper_QVariantAnimation::event(QVariantAnimation* theWrappedObject, QEvent*  event)
{
  return ( ((PythonQtPublicPromoter_QVariantAnimation*)theWrappedObject)->promoted_event(event));
}

QVariant  PythonQtWrapper_QVariantAnimation::interpolated(QVariantAnimation* theWrappedObject, const QVariant&  from, const QVariant&  to, qreal  progress) const
{
  return ( ((PythonQtPublicPromoter_QVariantAnimation*)theWrappedObject)->promoted_interpolated(from, to, progress));
}

QVariant  PythonQtWrapper_QVariantAnimation::keyValueAt(QVariantAnimation* theWrappedObject, qreal  step) const
{
  return ( theWrappedObject->keyValueAt(step));
}

QVector<QPair<qreal , QVariant >  >  PythonQtWrapper_QVariantAnimation::keyValues(QVariantAnimation* theWrappedObject) const
{
  return ( theWrappedObject->keyValues());
}

void PythonQtWrapper_QVariantAnimation::setDuration(QVariantAnimation* theWrappedObject, int  msecs)
{
  ( theWrappedObject->setDuration(msecs));
}

void PythonQtWrapper_QVariantAnimation::setEasingCurve(QVariantAnimation* theWrappedObject, const QEasingCurve&  easing)
{
  ( theWrappedObject->setEasingCurve(easing));
}

void PythonQtWrapper_QVariantAnimation::setEndValue(QVariantAnimation* theWrappedObject, const QVariant&  value)
{
  ( theWrappedObject->setEndValue(value));
}

void PythonQtWrapper_QVariantAnimation::setKeyValueAt(QVariantAnimation* theWrappedObject, qreal  step, const QVariant&  value)
{
  ( theWrappedObject->setKeyValueAt(step, value));
}

void PythonQtWrapper_QVariantAnimation::setKeyValues(QVariantAnimation* theWrappedObject, const QVector<QPair<qreal , QVariant >  >&  values)
{
  ( theWrappedObject->setKeyValues(values));
}

void PythonQtWrapper_QVariantAnimation::setStartValue(QVariantAnimation* theWrappedObject, const QVariant&  value)
{
  ( theWrappedObject->setStartValue(value));
}

QVariant  PythonQtWrapper_QVariantAnimation::startValue(QVariantAnimation* theWrappedObject) const
{
  return ( theWrappedObject->startValue());
}

void PythonQtWrapper_QVariantAnimation::updateCurrentTime(QVariantAnimation* theWrappedObject, int  arg__1)
{
  ( ((PythonQtPublicPromoter_QVariantAnimation*)theWrappedObject)->promoted_updateCurrentTime(arg__1));
}

void PythonQtWrapper_QVariantAnimation::updateState(QVariantAnimation* theWrappedObject, QAbstractAnimation::State  newState, QAbstractAnimation::State  oldState)
{
  ( ((PythonQtPublicPromoter_QVariantAnimation*)theWrappedObject)->promoted_updateState(newState, oldState));
}



QWaitCondition* PythonQtWrapper_QWaitCondition::new_QWaitCondition()
{ 
return new QWaitCondition(); }

bool  PythonQtWrapper_QWaitCondition::wait(QWaitCondition* theWrappedObject, QMutex*  mutex, unsigned long  time)
{
  return ( theWrappedObject->wait(mutex, time));
}

bool  PythonQtWrapper_QWaitCondition::wait(QWaitCondition* theWrappedObject, QReadWriteLock*  readWriteLock, unsigned long  time)
{
  return ( theWrappedObject->wait(readWriteLock, time));
}

void PythonQtWrapper_QWaitCondition::wakeAll(QWaitCondition* theWrappedObject)
{
  ( theWrappedObject->wakeAll());
}

void PythonQtWrapper_QWaitCondition::wakeOne(QWaitCondition* theWrappedObject)
{
  ( theWrappedObject->wakeOne());
}



QString  PythonQtShell_QXmlStreamEntityResolver::resolveEntity(const QString&  publicId, const QString&  systemId)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "resolveEntity");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QString" , "const QString&" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(3, argumentList);
      QString returnValue;
    void* args[3] = {NULL, (void*)&publicId, (void*)&systemId};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("resolveEntity", methodInfo, result);
        } else {
          returnValue = *((QString*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QXmlStreamEntityResolver::resolveEntity(publicId, systemId);
}
QString  PythonQtShell_QXmlStreamEntityResolver::resolveUndeclaredEntity(const QString&  name)
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "resolveUndeclaredEntity");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QString" , "const QString&"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      QString returnValue;
    void* args[2] = {NULL, (void*)&name};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("resolveUndeclaredEntity", methodInfo, result);
        } else {
          returnValue = *((QString*)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QXmlStreamEntityResolver::resolveUndeclaredEntity(name);
}
QXmlStreamEntityResolver* PythonQtWrapper_QXmlStreamEntityResolver::new_QXmlStreamEntityResolver()
{ 
return new PythonQtShell_QXmlStreamEntityResolver(); }

QString  PythonQtWrapper_QXmlStreamEntityResolver::resolveEntity(QXmlStreamEntityResolver* theWrappedObject, const QString&  publicId, const QString&  systemId)
{
  return ( ((PythonQtPublicPromoter_QXmlStreamEntityResolver*)theWrappedObject)->promoted_resolveEntity(publicId, systemId));
}

QString  PythonQtWrapper_QXmlStreamEntityResolver::resolveUndeclaredEntity(QXmlStreamEntityResolver* theWrappedObject, const QString&  name)
{
  return ( ((PythonQtPublicPromoter_QXmlStreamEntityResolver*)theWrappedObject)->promoted_resolveUndeclaredEntity(name));
}




