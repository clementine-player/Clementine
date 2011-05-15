#include <PythonQt.h>
#include <QDateTime>
#include <QDir>
#include <QObject>
#include <QSize>
#include <QStringList>
#include <QUrl>
#include <QVariant>
#include <qabstractanimation.h>
#include <qabstractitemmodel.h>
#include <qabstractstate.h>
#include <qabstracttransition.h>
#include <qanimationgroup.h>
#include <qbasictimer.h>
#include <qbuffer.h>
#include <qbytearray.h>
#include <qbytearraymatcher.h>
#include <qcoreapplication.h>
#include <qcoreevent.h>
#include <qcryptographichash.h>
#include <qdatastream.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qdiriterator.h>
#include <qeasingcurve.h>
#include <qeventloop.h>
#include <qeventtransition.h>
#include <qfactoryinterface.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qfilesystemwatcher.h>
#include <qfinalstate.h>
#include <qhistorystate.h>
#include <qiodevice.h>
#include <qlibraryinfo.h>
#include <qlist.h>
#include <qmimedata.h>
#include <qobject.h>
#include <qsize.h>
#include <qstate.h>
#include <qstatemachine.h>
#include <qstringlist.h>
#include <qtranslator.h>
#include <qurl.h>



class PythonQtShell_QAbstractAnimation : public QAbstractAnimation
{
public:
    PythonQtShell_QAbstractAnimation(QObject*  parent = 0):QAbstractAnimation(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual int  duration() const;
virtual bool  event(QEvent*  event);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void updateCurrentTime(int  currentTime);
virtual void updateDirection(QAbstractAnimation::Direction  direction);
virtual void updateState(QAbstractAnimation::State  newState, QAbstractAnimation::State  oldState);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QAbstractAnimation : public QAbstractAnimation
{ public:
inline bool  promoted_event(QEvent*  event) { return QAbstractAnimation::event(event); }
inline void promoted_updateDirection(QAbstractAnimation::Direction  direction) { QAbstractAnimation::updateDirection(direction); }
inline void promoted_updateState(QAbstractAnimation::State  newState, QAbstractAnimation::State  oldState) { QAbstractAnimation::updateState(newState, oldState); }
};

class PythonQtWrapper_QAbstractAnimation : public QObject
{ Q_OBJECT
public:
Q_ENUMS(DeletionPolicy )
enum DeletionPolicy{
  KeepWhenStopped = QAbstractAnimation::KeepWhenStopped,   DeleteWhenStopped = QAbstractAnimation::DeleteWhenStopped};
public slots:
QAbstractAnimation* new_QAbstractAnimation(QObject*  parent = 0);
void delete_QAbstractAnimation(QAbstractAnimation* obj) { delete obj; } 
   int  currentLoop(QAbstractAnimation* theWrappedObject) const;
   int  currentLoopTime(QAbstractAnimation* theWrappedObject) const;
   int  currentTime(QAbstractAnimation* theWrappedObject) const;
   QAbstractAnimation::Direction  direction(QAbstractAnimation* theWrappedObject) const;
   bool  event(QAbstractAnimation* theWrappedObject, QEvent*  event);
   QAnimationGroup*  group(QAbstractAnimation* theWrappedObject) const;
   int  loopCount(QAbstractAnimation* theWrappedObject) const;
   void setDirection(QAbstractAnimation* theWrappedObject, QAbstractAnimation::Direction  direction);
   void setLoopCount(QAbstractAnimation* theWrappedObject, int  loopCount);
   QAbstractAnimation::State  state(QAbstractAnimation* theWrappedObject) const;
   int  totalDuration(QAbstractAnimation* theWrappedObject) const;
   void updateDirection(QAbstractAnimation* theWrappedObject, QAbstractAnimation::Direction  direction);
   void updateState(QAbstractAnimation* theWrappedObject, QAbstractAnimation::State  newState, QAbstractAnimation::State  oldState);
};





class PythonQtShell_QAbstractItemModel : public QAbstractItemModel
{
public:
    PythonQtShell_QAbstractItemModel(QObject*  parent = 0):QAbstractItemModel(parent),_wrapper(NULL) {};

virtual QModelIndex  buddy(const QModelIndex&  index) const;
virtual bool  canFetchMore(const QModelIndex&  parent) const;
virtual void childEvent(QChildEvent*  arg__1);
virtual int  columnCount(const QModelIndex&  parent = QModelIndex()) const;
virtual void customEvent(QEvent*  arg__1);
virtual QVariant  data(const QModelIndex&  index, int  role = Qt::DisplayRole) const;
virtual bool  dropMimeData(const QMimeData*  data, Qt::DropAction  action, int  row, int  column, const QModelIndex&  parent);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void fetchMore(const QModelIndex&  parent);
virtual Qt::ItemFlags  flags(const QModelIndex&  index) const;
virtual bool  hasChildren(const QModelIndex&  parent = QModelIndex()) const;
virtual QVariant  headerData(int  section, Qt::Orientation  orientation, int  role = Qt::DisplayRole) const;
virtual QModelIndex  index(int  row, int  column, const QModelIndex&  parent = QModelIndex()) const;
virtual bool  insertColumns(int  column, int  count, const QModelIndex&  parent = QModelIndex());
virtual bool  insertRows(int  row, int  count, const QModelIndex&  parent = QModelIndex());
virtual QMap<int , QVariant >  itemData(const QModelIndex&  index) const;
virtual QList<QModelIndex >  match(const QModelIndex&  start, int  role, const QVariant&  value, int  hits = 1, Qt::MatchFlags  flags = Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap)) const;
virtual QMimeData*  mimeData(const QList<QModelIndex >&  indexes) const;
virtual QStringList  mimeTypes() const;
virtual QModelIndex  parent(const QModelIndex&  child) const;
virtual bool  removeColumns(int  column, int  count, const QModelIndex&  parent = QModelIndex());
virtual bool  removeRows(int  row, int  count, const QModelIndex&  parent = QModelIndex());
virtual void revert();
virtual int  rowCount(const QModelIndex&  parent = QModelIndex()) const;
virtual bool  setData(const QModelIndex&  index, const QVariant&  value, int  role = Qt::EditRole);
virtual bool  setHeaderData(int  section, Qt::Orientation  orientation, const QVariant&  value, int  role = Qt::EditRole);
virtual bool  setItemData(const QModelIndex&  index, const QMap<int , QVariant >&  roles);
virtual void sort(int  column, Qt::SortOrder  order = Qt::AscendingOrder);
virtual QSize  span(const QModelIndex&  index) const;
virtual bool  submit();
virtual Qt::DropActions  supportedDropActions() const;
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QAbstractItemModel : public QAbstractItemModel
{ public:
inline QModelIndex  promoted_buddy(const QModelIndex&  index) const { return QAbstractItemModel::buddy(index); }
inline bool  promoted_canFetchMore(const QModelIndex&  parent) const { return QAbstractItemModel::canFetchMore(parent); }
inline bool  promoted_dropMimeData(const QMimeData*  data, Qt::DropAction  action, int  row, int  column, const QModelIndex&  parent) { return QAbstractItemModel::dropMimeData(data, action, row, column, parent); }
inline void promoted_fetchMore(const QModelIndex&  parent) { QAbstractItemModel::fetchMore(parent); }
inline Qt::ItemFlags  promoted_flags(const QModelIndex&  index) const { return QAbstractItemModel::flags(index); }
inline bool  promoted_hasChildren(const QModelIndex&  parent = QModelIndex()) const { return QAbstractItemModel::hasChildren(parent); }
inline QVariant  promoted_headerData(int  section, Qt::Orientation  orientation, int  role = Qt::DisplayRole) const { return QAbstractItemModel::headerData(section, orientation, role); }
inline bool  promoted_insertColumns(int  column, int  count, const QModelIndex&  parent = QModelIndex()) { return QAbstractItemModel::insertColumns(column, count, parent); }
inline bool  promoted_insertRows(int  row, int  count, const QModelIndex&  parent = QModelIndex()) { return QAbstractItemModel::insertRows(row, count, parent); }
inline QMap<int , QVariant >  promoted_itemData(const QModelIndex&  index) const { return QAbstractItemModel::itemData(index); }
inline QList<QModelIndex >  promoted_match(const QModelIndex&  start, int  role, const QVariant&  value, int  hits = 1, Qt::MatchFlags  flags = Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap)) const { return QAbstractItemModel::match(start, role, value, hits, flags); }
inline QMimeData*  promoted_mimeData(const QList<QModelIndex >&  indexes) const { return QAbstractItemModel::mimeData(indexes); }
inline QStringList  promoted_mimeTypes() const { return QAbstractItemModel::mimeTypes(); }
inline bool  promoted_removeColumns(int  column, int  count, const QModelIndex&  parent = QModelIndex()) { return QAbstractItemModel::removeColumns(column, count, parent); }
inline bool  promoted_removeRows(int  row, int  count, const QModelIndex&  parent = QModelIndex()) { return QAbstractItemModel::removeRows(row, count, parent); }
inline void promoted_revert() { QAbstractItemModel::revert(); }
inline bool  promoted_setData(const QModelIndex&  index, const QVariant&  value, int  role = Qt::EditRole) { return QAbstractItemModel::setData(index, value, role); }
inline bool  promoted_setHeaderData(int  section, Qt::Orientation  orientation, const QVariant&  value, int  role = Qt::EditRole) { return QAbstractItemModel::setHeaderData(section, orientation, value, role); }
inline bool  promoted_setItemData(const QModelIndex&  index, const QMap<int , QVariant >&  roles) { return QAbstractItemModel::setItemData(index, roles); }
inline void promoted_sort(int  column, Qt::SortOrder  order = Qt::AscendingOrder) { QAbstractItemModel::sort(column, order); }
inline QSize  promoted_span(const QModelIndex&  index) const { return QAbstractItemModel::span(index); }
inline bool  promoted_submit() { return QAbstractItemModel::submit(); }
inline Qt::DropActions  promoted_supportedDropActions() const { return QAbstractItemModel::supportedDropActions(); }
};

class PythonQtWrapper_QAbstractItemModel : public QObject
{ Q_OBJECT
public:
public slots:
QAbstractItemModel* new_QAbstractItemModel(QObject*  parent = 0);
void delete_QAbstractItemModel(QAbstractItemModel* obj) { delete obj; } 
   QModelIndex  buddy(QAbstractItemModel* theWrappedObject, const QModelIndex&  index) const;
   bool  canFetchMore(QAbstractItemModel* theWrappedObject, const QModelIndex&  parent) const;
   bool  dropMimeData(QAbstractItemModel* theWrappedObject, const QMimeData*  data, Qt::DropAction  action, int  row, int  column, const QModelIndex&  parent);
   void fetchMore(QAbstractItemModel* theWrappedObject, const QModelIndex&  parent);
   Qt::ItemFlags  flags(QAbstractItemModel* theWrappedObject, const QModelIndex&  index) const;
   bool  hasChildren(QAbstractItemModel* theWrappedObject, const QModelIndex&  parent = QModelIndex()) const;
   bool  hasIndex(QAbstractItemModel* theWrappedObject, int  row, int  column, const QModelIndex&  parent = QModelIndex()) const;
   QVariant  headerData(QAbstractItemModel* theWrappedObject, int  section, Qt::Orientation  orientation, int  role = Qt::DisplayRole) const;
   bool  insertColumn(QAbstractItemModel* theWrappedObject, int  column, const QModelIndex&  parent = QModelIndex());
   bool  insertColumns(QAbstractItemModel* theWrappedObject, int  column, int  count, const QModelIndex&  parent = QModelIndex());
   bool  insertRow(QAbstractItemModel* theWrappedObject, int  row, const QModelIndex&  parent = QModelIndex());
   bool  insertRows(QAbstractItemModel* theWrappedObject, int  row, int  count, const QModelIndex&  parent = QModelIndex());
   QMap<int , QVariant >  itemData(QAbstractItemModel* theWrappedObject, const QModelIndex&  index) const;
   QList<QModelIndex >  match(QAbstractItemModel* theWrappedObject, const QModelIndex&  start, int  role, const QVariant&  value, int  hits = 1, Qt::MatchFlags  flags = Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap)) const;
   QMimeData*  mimeData(QAbstractItemModel* theWrappedObject, const QList<QModelIndex >&  indexes) const;
   QStringList  mimeTypes(QAbstractItemModel* theWrappedObject) const;
   QObject*  parent(QAbstractItemModel* theWrappedObject) const;
   bool  removeColumn(QAbstractItemModel* theWrappedObject, int  column, const QModelIndex&  parent = QModelIndex());
   bool  removeColumns(QAbstractItemModel* theWrappedObject, int  column, int  count, const QModelIndex&  parent = QModelIndex());
   bool  removeRow(QAbstractItemModel* theWrappedObject, int  row, const QModelIndex&  parent = QModelIndex());
   bool  removeRows(QAbstractItemModel* theWrappedObject, int  row, int  count, const QModelIndex&  parent = QModelIndex());
   const QHash<int , QByteArray >*  roleNames(QAbstractItemModel* theWrappedObject) const;
   bool  setData(QAbstractItemModel* theWrappedObject, const QModelIndex&  index, const QVariant&  value, int  role = Qt::EditRole);
   bool  setHeaderData(QAbstractItemModel* theWrappedObject, int  section, Qt::Orientation  orientation, const QVariant&  value, int  role = Qt::EditRole);
   bool  setItemData(QAbstractItemModel* theWrappedObject, const QModelIndex&  index, const QMap<int , QVariant >&  roles);
   void setSupportedDragActions(QAbstractItemModel* theWrappedObject, Qt::DropActions  arg__1);
   QModelIndex  sibling(QAbstractItemModel* theWrappedObject, int  row, int  column, const QModelIndex&  idx) const;
   void sort(QAbstractItemModel* theWrappedObject, int  column, Qt::SortOrder  order = Qt::AscendingOrder);
   QSize  span(QAbstractItemModel* theWrappedObject, const QModelIndex&  index) const;
   Qt::DropActions  supportedDragActions(QAbstractItemModel* theWrappedObject) const;
   Qt::DropActions  supportedDropActions(QAbstractItemModel* theWrappedObject) const;
};





class PythonQtShell_QAbstractListModel : public QAbstractListModel
{
public:
    PythonQtShell_QAbstractListModel(QObject*  parent = 0):QAbstractListModel(parent),_wrapper(NULL) {};

virtual QModelIndex  buddy(const QModelIndex&  index) const;
virtual bool  canFetchMore(const QModelIndex&  parent) const;
virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual QVariant  data(const QModelIndex&  index, int  role) const;
virtual bool  dropMimeData(const QMimeData*  data, Qt::DropAction  action, int  row, int  column, const QModelIndex&  parent);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void fetchMore(const QModelIndex&  parent);
virtual Qt::ItemFlags  flags(const QModelIndex&  index) const;
virtual QVariant  headerData(int  section, Qt::Orientation  orientation, int  role) const;
virtual QModelIndex  index(int  row, int  column = 0, const QModelIndex&  parent = QModelIndex()) const;
virtual bool  insertColumns(int  column, int  count, const QModelIndex&  parent);
virtual bool  insertRows(int  row, int  count, const QModelIndex&  parent);
virtual QMap<int , QVariant >  itemData(const QModelIndex&  index) const;
virtual QList<QModelIndex >  match(const QModelIndex&  start, int  role, const QVariant&  value, int  hits, Qt::MatchFlags  flags) const;
virtual QMimeData*  mimeData(const QList<QModelIndex >&  indexes) const;
virtual QStringList  mimeTypes() const;
virtual bool  removeColumns(int  column, int  count, const QModelIndex&  parent);
virtual bool  removeRows(int  row, int  count, const QModelIndex&  parent);
virtual void revert();
virtual int  rowCount(const QModelIndex&  parent) const;
virtual bool  setData(const QModelIndex&  index, const QVariant&  value, int  role);
virtual bool  setHeaderData(int  section, Qt::Orientation  orientation, const QVariant&  value, int  role);
virtual bool  setItemData(const QModelIndex&  index, const QMap<int , QVariant >&  roles);
virtual void sort(int  column, Qt::SortOrder  order);
virtual QSize  span(const QModelIndex&  index) const;
virtual bool  submit();
virtual Qt::DropActions  supportedDropActions() const;
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QAbstractListModel : public QAbstractListModel
{ public:
inline bool  promoted_dropMimeData(const QMimeData*  data, Qt::DropAction  action, int  row, int  column, const QModelIndex&  parent) { return QAbstractListModel::dropMimeData(data, action, row, column, parent); }
inline QModelIndex  promoted_index(int  row, int  column = 0, const QModelIndex&  parent = QModelIndex()) const { return QAbstractListModel::index(row, column, parent); }
};

class PythonQtWrapper_QAbstractListModel : public QObject
{ Q_OBJECT
public:
public slots:
QAbstractListModel* new_QAbstractListModel(QObject*  parent = 0);
void delete_QAbstractListModel(QAbstractListModel* obj) { delete obj; } 
   bool  dropMimeData(QAbstractListModel* theWrappedObject, const QMimeData*  data, Qt::DropAction  action, int  row, int  column, const QModelIndex&  parent);
   QModelIndex  index(QAbstractListModel* theWrappedObject, int  row, int  column = 0, const QModelIndex&  parent = QModelIndex()) const;
};





class PythonQtShell_QAbstractState : public QAbstractState
{
public:
    PythonQtShell_QAbstractState(QState*  parent = 0):QAbstractState(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  e);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void onEntry(QEvent*  event);
virtual void onExit(QEvent*  event);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QAbstractState : public QAbstractState
{ public:
inline bool  promoted_event(QEvent*  e) { return QAbstractState::event(e); }
};

class PythonQtWrapper_QAbstractState : public QObject
{ Q_OBJECT
public:
public slots:
void delete_QAbstractState(QAbstractState* obj) { delete obj; } 
   bool  event(QAbstractState* theWrappedObject, QEvent*  e);
   QStateMachine*  machine(QAbstractState* theWrappedObject) const;
   QState*  parentState(QAbstractState* theWrappedObject) const;
};





class PythonQtShell_QAbstractTransition : public QAbstractTransition
{
public:
    PythonQtShell_QAbstractTransition(QState*  sourceState = 0):QAbstractTransition(sourceState),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  e);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual bool  eventTest(QEvent*  event);
virtual void onTransition(QEvent*  event);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QAbstractTransition : public QAbstractTransition
{ public:
inline bool  promoted_event(QEvent*  e) { return QAbstractTransition::event(e); }
};

class PythonQtWrapper_QAbstractTransition : public QObject
{ Q_OBJECT
public:
public slots:
QAbstractTransition* new_QAbstractTransition(QState*  sourceState = 0);
void delete_QAbstractTransition(QAbstractTransition* obj) { delete obj; } 
   void addAnimation(QAbstractTransition* theWrappedObject, QAbstractAnimation*  animation);
   QList<QAbstractAnimation* >  animations(QAbstractTransition* theWrappedObject) const;
   bool  event(QAbstractTransition* theWrappedObject, QEvent*  e);
   QStateMachine*  machine(QAbstractTransition* theWrappedObject) const;
   void removeAnimation(QAbstractTransition* theWrappedObject, QAbstractAnimation*  animation);
   void setTargetState(QAbstractTransition* theWrappedObject, QAbstractState*  target);
   void setTargetStates(QAbstractTransition* theWrappedObject, const QList<QAbstractState* >&  targets);
   QState*  sourceState(QAbstractTransition* theWrappedObject) const;
   QAbstractState*  targetState(QAbstractTransition* theWrappedObject) const;
   QList<QAbstractState* >  targetStates(QAbstractTransition* theWrappedObject) const;
};





class PythonQtShell_QAnimationGroup : public QAnimationGroup
{
public:
    PythonQtShell_QAnimationGroup(QObject*  parent = 0):QAnimationGroup(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual int  duration() const;
virtual bool  event(QEvent*  event);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void updateCurrentTime(int  currentTime);
virtual void updateDirection(QAbstractAnimation::Direction  direction);
virtual void updateState(QAbstractAnimation::State  newState, QAbstractAnimation::State  oldState);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QAnimationGroup : public QAnimationGroup
{ public:
inline bool  promoted_event(QEvent*  event) { return QAnimationGroup::event(event); }
};

class PythonQtWrapper_QAnimationGroup : public QObject
{ Q_OBJECT
public:
public slots:
QAnimationGroup* new_QAnimationGroup(QObject*  parent = 0);
void delete_QAnimationGroup(QAnimationGroup* obj) { delete obj; } 
   void addAnimation(QAnimationGroup* theWrappedObject, QAbstractAnimation*  animation);
   QAbstractAnimation*  animationAt(QAnimationGroup* theWrappedObject, int  index) const;
   int  animationCount(QAnimationGroup* theWrappedObject) const;
   void clear(QAnimationGroup* theWrappedObject);
   bool  event(QAnimationGroup* theWrappedObject, QEvent*  event);
   int  indexOfAnimation(QAnimationGroup* theWrappedObject, QAbstractAnimation*  animation) const;
   void insertAnimation(QAnimationGroup* theWrappedObject, int  index, QAbstractAnimation*  animation);
   void removeAnimation(QAnimationGroup* theWrappedObject, QAbstractAnimation*  animation);
   QAbstractAnimation*  takeAnimation(QAnimationGroup* theWrappedObject, int  index);
};





class PythonQtWrapper_QBasicTimer : public QObject
{ Q_OBJECT
public:
public slots:
QBasicTimer* new_QBasicTimer();
QBasicTimer* new_QBasicTimer(const QBasicTimer& other) {
QBasicTimer* a = new QBasicTimer();
*((QBasicTimer*)a) = other;
return a; }
void delete_QBasicTimer(QBasicTimer* obj) { delete obj; } 
   bool  isActive(QBasicTimer* theWrappedObject) const;
   void start(QBasicTimer* theWrappedObject, int  msec, QObject*  obj);
   void stop(QBasicTimer* theWrappedObject);
   int  timerId(QBasicTimer* theWrappedObject) const;
};





class PythonQtShell_QBuffer : public QBuffer
{
public:
    PythonQtShell_QBuffer(QByteArray*  buf, QObject*  parent = 0):QBuffer(buf, parent),_wrapper(NULL) {};
    PythonQtShell_QBuffer(QObject*  parent = 0):QBuffer(parent),_wrapper(NULL) {};

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
virtual bool  open(QIODevice::OpenMode  openMode);
virtual qint64  pos() const;
virtual qint64  readData(char*  data, qint64  maxlen);
virtual qint64  readLineData(char*  data, qint64  maxlen);
virtual bool  reset();
virtual bool  seek(qint64  off);
virtual qint64  size() const;
virtual void timerEvent(QTimerEvent*  arg__1);
virtual bool  waitForBytesWritten(int  msecs);
virtual bool  waitForReadyRead(int  msecs);
virtual qint64  writeData(const char*  data, qint64  len);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QBuffer : public QBuffer
{ public:
inline bool  promoted_atEnd() const { return QBuffer::atEnd(); }
inline bool  promoted_canReadLine() const { return QBuffer::canReadLine(); }
inline void promoted_close() { QBuffer::close(); }
inline bool  promoted_open(QIODevice::OpenMode  openMode) { return QBuffer::open(openMode); }
inline qint64  promoted_pos() const { return QBuffer::pos(); }
inline qint64  promoted_readData(char*  data, qint64  maxlen) { return QBuffer::readData(data, maxlen); }
inline bool  promoted_seek(qint64  off) { return QBuffer::seek(off); }
inline qint64  promoted_size() const { return QBuffer::size(); }
inline qint64  promoted_writeData(const char*  data, qint64  len) { return QBuffer::writeData(data, len); }
};

class PythonQtWrapper_QBuffer : public QObject
{ Q_OBJECT
public:
public slots:
QBuffer* new_QBuffer(QByteArray*  buf, QObject*  parent = 0);
QBuffer* new_QBuffer(QObject*  parent = 0);
void delete_QBuffer(QBuffer* obj) { delete obj; } 
   bool  atEnd(QBuffer* theWrappedObject) const;
   bool  canReadLine(QBuffer* theWrappedObject) const;
   void close(QBuffer* theWrappedObject);
   bool  open(QBuffer* theWrappedObject, QIODevice::OpenMode  openMode);
   qint64  pos(QBuffer* theWrappedObject) const;
   qint64  readData(QBuffer* theWrappedObject, char*  data, qint64  maxlen);
   bool  seek(QBuffer* theWrappedObject, qint64  off);
   void setBuffer(QBuffer* theWrappedObject, QByteArray*  a);
   void setData(QBuffer* theWrappedObject, const QByteArray&  data);
   qint64  size(QBuffer* theWrappedObject) const;
   qint64  writeData(QBuffer* theWrappedObject, const char*  data, qint64  len);
};





class PythonQtWrapper_QByteArrayMatcher : public QObject
{ Q_OBJECT
public:
public slots:
QByteArrayMatcher* new_QByteArrayMatcher();
QByteArrayMatcher* new_QByteArrayMatcher(const QByteArray&  pattern);
QByteArrayMatcher* new_QByteArrayMatcher(const QByteArrayMatcher&  other);
QByteArrayMatcher* new_QByteArrayMatcher(const char*  pattern, int  length);
void delete_QByteArrayMatcher(QByteArrayMatcher* obj) { delete obj; } 
   int  indexIn(QByteArrayMatcher* theWrappedObject, const QByteArray&  ba, int  from = 0) const;
   int  indexIn(QByteArrayMatcher* theWrappedObject, const char*  str, int  len, int  from = 0) const;
   QByteArray  pattern(QByteArrayMatcher* theWrappedObject) const;
   void setPattern(QByteArrayMatcher* theWrappedObject, const QByteArray&  pattern);
};





class PythonQtShell_QChildEvent : public QChildEvent
{
public:
    PythonQtShell_QChildEvent(QEvent::Type  type, QObject*  child):QChildEvent(type, child),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QChildEvent : public QObject
{ Q_OBJECT
public:
public slots:
QChildEvent* new_QChildEvent(QEvent::Type  type, QObject*  child);
void delete_QChildEvent(QChildEvent* obj) { delete obj; } 
   bool  added(QChildEvent* theWrappedObject) const;
   QObject*  child(QChildEvent* theWrappedObject) const;
   bool  polished(QChildEvent* theWrappedObject) const;
   bool  removed(QChildEvent* theWrappedObject) const;
};





class PythonQtShell_QCoreApplication : public QCoreApplication
{
public:

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual bool  notify(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QCoreApplication : public QCoreApplication
{ public:
inline bool  promoted_event(QEvent*  arg__1) { return QCoreApplication::event(arg__1); }
inline bool  promoted_notify(QObject*  arg__1, QEvent*  arg__2) { return QCoreApplication::notify(arg__1, arg__2); }
};

class PythonQtWrapper_QCoreApplication : public QObject
{ Q_OBJECT
public:
Q_ENUMS(Encoding )
enum Encoding{
  CodecForTr = QCoreApplication::CodecForTr,   UnicodeUTF8 = QCoreApplication::UnicodeUTF8,   DefaultCodec = QCoreApplication::DefaultCodec};
public slots:
void delete_QCoreApplication(QCoreApplication* obj) { delete obj; } 
   void static_QCoreApplication_addLibraryPath(const QString&  arg__1);
   QString  static_QCoreApplication_applicationDirPath();
   QString  static_QCoreApplication_applicationFilePath();
   QString  static_QCoreApplication_applicationName();
   qint64  static_QCoreApplication_applicationPid();
   QString  static_QCoreApplication_applicationVersion();
   bool  static_QCoreApplication_closingDown();
   bool  event(QCoreApplication* theWrappedObject, QEvent*  arg__1);
   int  static_QCoreApplication_exec();
   void static_QCoreApplication_exit(int  retcode = 0);
   void static_QCoreApplication_flush();
   bool  static_QCoreApplication_hasPendingEvents();
   void static_QCoreApplication_installTranslator(QTranslator*  messageFile);
   QCoreApplication*  static_QCoreApplication_instance();
   QStringList  static_QCoreApplication_libraryPaths();
   bool  notify(QCoreApplication* theWrappedObject, QObject*  arg__1, QEvent*  arg__2);
   QString  static_QCoreApplication_organizationDomain();
   QString  static_QCoreApplication_organizationName();
   void static_QCoreApplication_postEvent(QObject*  receiver, QEvent*  event);
   void static_QCoreApplication_postEvent(QObject*  receiver, QEvent*  event, int  priority);
   void static_QCoreApplication_processEvents(QEventLoop::ProcessEventsFlags  flags = QEventLoop::AllEvents);
   void static_QCoreApplication_processEvents(QEventLoop::ProcessEventsFlags  flags, int  maxtime);
   void static_QCoreApplication_removeLibraryPath(const QString&  arg__1);
   void static_QCoreApplication_removePostedEvents(QObject*  receiver);
   void static_QCoreApplication_removePostedEvents(QObject*  receiver, int  eventType);
   void static_QCoreApplication_removeTranslator(QTranslator*  messageFile);
   bool  static_QCoreApplication_sendEvent(QObject*  receiver, QEvent*  event);
   void static_QCoreApplication_sendPostedEvents();
   void static_QCoreApplication_sendPostedEvents(QObject*  receiver, int  event_type);
   void static_QCoreApplication_setApplicationName(const QString&  application);
   void static_QCoreApplication_setApplicationVersion(const QString&  version);
   void static_QCoreApplication_setAttribute(Qt::ApplicationAttribute  attribute, bool  on = true);
   void static_QCoreApplication_setLibraryPaths(const QStringList&  arg__1);
   void static_QCoreApplication_setOrganizationDomain(const QString&  orgDomain);
   void static_QCoreApplication_setOrganizationName(const QString&  orgName);
   bool  static_QCoreApplication_startingUp();
   bool  static_QCoreApplication_testAttribute(Qt::ApplicationAttribute  attribute);
   QString  static_QCoreApplication_translate(const char*  context, const char*  key, const char*  disambiguation = 0, QCoreApplication::Encoding  encoding = QCoreApplication::CodecForTr);
   QString  static_QCoreApplication_translate(const char*  context, const char*  key, const char*  disambiguation, QCoreApplication::Encoding  encoding, int  n);
};





class PythonQtWrapper_QCryptographicHash : public QObject
{ Q_OBJECT
public:
Q_ENUMS(Algorithm )
enum Algorithm{
  Md4 = QCryptographicHash::Md4,   Md5 = QCryptographicHash::Md5,   Sha1 = QCryptographicHash::Sha1};
public slots:
QCryptographicHash* new_QCryptographicHash(QCryptographicHash::Algorithm  method);
void delete_QCryptographicHash(QCryptographicHash* obj) { delete obj; } 
   void addData(QCryptographicHash* theWrappedObject, const QByteArray&  data);
   QByteArray  static_QCryptographicHash_hash(const QByteArray&  data, QCryptographicHash::Algorithm  method);
   void reset(QCryptographicHash* theWrappedObject);
   QByteArray  result(QCryptographicHash* theWrappedObject) const;
};





class PythonQtShell_QDataStream : public QDataStream
{
public:
    PythonQtShell_QDataStream():QDataStream(),_wrapper(NULL) {};
    PythonQtShell_QDataStream(QByteArray*  arg__1, QIODevice::OpenMode  flags):QDataStream(arg__1, flags),_wrapper(NULL) {};
    PythonQtShell_QDataStream(QIODevice*  arg__1):QDataStream(arg__1),_wrapper(NULL) {};
    PythonQtShell_QDataStream(const QByteArray&  arg__1):QDataStream(arg__1),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QDataStream : public QObject
{ Q_OBJECT
public:
Q_ENUMS(FloatingPointPrecision Version Status )
enum FloatingPointPrecision{
  SinglePrecision = QDataStream::SinglePrecision,   DoublePrecision = QDataStream::DoublePrecision};
enum Version{
  Qt_1_0 = QDataStream::Qt_1_0,   Qt_2_0 = QDataStream::Qt_2_0,   Qt_2_1 = QDataStream::Qt_2_1,   Qt_3_0 = QDataStream::Qt_3_0,   Qt_3_1 = QDataStream::Qt_3_1,   Qt_3_3 = QDataStream::Qt_3_3,   Qt_4_0 = QDataStream::Qt_4_0,   Qt_4_1 = QDataStream::Qt_4_1,   Qt_4_2 = QDataStream::Qt_4_2,   Qt_4_3 = QDataStream::Qt_4_3,   Qt_4_4 = QDataStream::Qt_4_4,   Qt_4_5 = QDataStream::Qt_4_5,   Qt_4_6 = QDataStream::Qt_4_6};
enum Status{
  Ok = QDataStream::Ok,   ReadPastEnd = QDataStream::ReadPastEnd,   ReadCorruptData = QDataStream::ReadCorruptData};
public slots:
QDataStream* new_QDataStream();
QDataStream* new_QDataStream(QByteArray*  arg__1, QIODevice::OpenMode  flags);
QDataStream* new_QDataStream(QIODevice*  arg__1);
QDataStream* new_QDataStream(const QByteArray&  arg__1);
void delete_QDataStream(QDataStream* obj) { delete obj; } 
   bool  atEnd(QDataStream* theWrappedObject) const;
   QIODevice*  device(QDataStream* theWrappedObject) const;
   QDataStream::FloatingPointPrecision  floatingPointPrecision(QDataStream* theWrappedObject) const;
   QDataStream*  writeBoolean(QDataStream* theWrappedObject, bool  i);
   QDataStream*  writeDouble(QDataStream* theWrappedObject, double  f);
   QDataStream*  writeFloat(QDataStream* theWrappedObject, float  f);
   QDataStream*  writeInt(QDataStream* theWrappedObject, int  i);
   QDataStream*  writeLongLong(QDataStream* theWrappedObject, qint64  i);
   QDataStream*  writeShort(QDataStream* theWrappedObject, short  i);
   QDataStream*  readBoolean(QDataStream* theWrappedObject, bool&  i);
   QDataStream*  readDouble(QDataStream* theWrappedObject, double&  f);
   QDataStream*  readFloat(QDataStream* theWrappedObject, float&  f);
   QDataStream*  readInt(QDataStream* theWrappedObject, int&  i);
   QDataStream*  readLongLong(QDataStream* theWrappedObject, qint64&  i);
   QDataStream*  readShort(QDataStream* theWrappedObject, short&  i);
   QDataStream*  readUShort(QDataStream* theWrappedObject, unsigned short&  i);
   void resetStatus(QDataStream* theWrappedObject);
   void setDevice(QDataStream* theWrappedObject, QIODevice*  arg__1);
   void setFloatingPointPrecision(QDataStream* theWrappedObject, QDataStream::FloatingPointPrecision  precision);
   void setStatus(QDataStream* theWrappedObject, QDataStream::Status  status);
   void setVersion(QDataStream* theWrappedObject, int  arg__1);
   int  skipRawData(QDataStream* theWrappedObject, int  len);
   QDataStream::Status  status(QDataStream* theWrappedObject) const;
   void unsetDevice(QDataStream* theWrappedObject);
   int  version(QDataStream* theWrappedObject) const;
};





class PythonQtWrapper_QDir : public QObject
{ Q_OBJECT
public:
Q_ENUMS(Filter SortFlag )
Q_FLAGS(Filters SortFlags )
enum Filter{
  Dirs = QDir::Dirs,   Files = QDir::Files,   Drives = QDir::Drives,   NoSymLinks = QDir::NoSymLinks,   AllEntries = QDir::AllEntries,   TypeMask = QDir::TypeMask,   Readable = QDir::Readable,   Writable = QDir::Writable,   Executable = QDir::Executable,   PermissionMask = QDir::PermissionMask,   Modified = QDir::Modified,   Hidden = QDir::Hidden,   System = QDir::System,   AccessMask = QDir::AccessMask,   AllDirs = QDir::AllDirs,   CaseSensitive = QDir::CaseSensitive,   NoDotAndDotDot = QDir::NoDotAndDotDot,   NoFilter = QDir::NoFilter};
enum SortFlag{
  Name = QDir::Name,   Time = QDir::Time,   Size = QDir::Size,   Unsorted = QDir::Unsorted,   SortByMask = QDir::SortByMask,   DirsFirst = QDir::DirsFirst,   Reversed = QDir::Reversed,   IgnoreCase = QDir::IgnoreCase,   DirsLast = QDir::DirsLast,   LocaleAware = QDir::LocaleAware,   Type = QDir::Type,   NoSort = QDir::NoSort};
Q_DECLARE_FLAGS(Filters, Filter)
Q_DECLARE_FLAGS(SortFlags, SortFlag)
public slots:
QDir* new_QDir(const QDir&  arg__1);
QDir* new_QDir(const QString&  path = QString());
QDir* new_QDir(const QString&  path, const QString&  nameFilter, QDir::SortFlags  sort = QDir::SortFlags(Name | IgnoreCase), QDir::Filters  filter = QDir::AllEntries);
void delete_QDir(QDir* obj) { delete obj; } 
   QString  absoluteFilePath(QDir* theWrappedObject, const QString&  fileName) const;
   QString  absolutePath(QDir* theWrappedObject) const;
   void static_QDir_addSearchPath(const QString&  prefix, const QString&  path);
   QString  canonicalPath(QDir* theWrappedObject) const;
   bool  cd(QDir* theWrappedObject, const QString&  dirName);
   bool  cdUp(QDir* theWrappedObject);
   QString  static_QDir_cleanPath(const QString&  path);
   QString  static_QDir_convertSeparators(const QString&  pathName);
   uint  count(QDir* theWrappedObject) const;
   QDir  static_QDir_current();
   QString  static_QDir_currentPath();
   QString  dirName(QDir* theWrappedObject) const;
   QList<QFileInfo >  static_QDir_drives();
   QList<QFileInfo >  entryInfoList(QDir* theWrappedObject, QDir::Filters  filters = QDir::NoFilter, QDir::SortFlags  sort = QDir::NoSort) const;
   QList<QFileInfo >  entryInfoList(QDir* theWrappedObject, const QStringList&  nameFilters, QDir::Filters  filters = QDir::NoFilter, QDir::SortFlags  sort = QDir::NoSort) const;
   QStringList  entryList(QDir* theWrappedObject, QDir::Filters  filters = QDir::NoFilter, QDir::SortFlags  sort = QDir::NoSort) const;
   QStringList  entryList(QDir* theWrappedObject, const QStringList&  nameFilters, QDir::Filters  filters = QDir::NoFilter, QDir::SortFlags  sort = QDir::NoSort) const;
   bool  exists(QDir* theWrappedObject) const;
   bool  exists(QDir* theWrappedObject, const QString&  name) const;
   QString  filePath(QDir* theWrappedObject, const QString&  fileName) const;
   QDir::Filters  filter(QDir* theWrappedObject) const;
   QString  static_QDir_fromNativeSeparators(const QString&  pathName);
   QDir  static_QDir_home();
   QString  static_QDir_homePath();
   bool  isAbsolute(QDir* theWrappedObject) const;
   bool  static_QDir_isAbsolutePath(const QString&  path);
   bool  isReadable(QDir* theWrappedObject) const;
   bool  isRelative(QDir* theWrappedObject) const;
   bool  static_QDir_isRelativePath(const QString&  path);
   bool  isRoot(QDir* theWrappedObject) const;
   bool  makeAbsolute(QDir* theWrappedObject);
   bool  static_QDir_match(const QString&  filter, const QString&  fileName);
   bool  static_QDir_match(const QStringList&  filters, const QString&  fileName);
   bool  mkdir(QDir* theWrappedObject, const QString&  dirName) const;
   bool  mkpath(QDir* theWrappedObject, const QString&  dirPath) const;
   QStringList  nameFilters(QDir* theWrappedObject) const;
   QStringList  static_QDir_nameFiltersFromString(const QString&  nameFilter);
   bool  __ne__(QDir* theWrappedObject, const QDir&  dir) const;
   bool  __eq__(QDir* theWrappedObject, const QDir&  dir) const;
   QString  operator_subscript(QDir* theWrappedObject, int  arg__1) const;
   QString  path(QDir* theWrappedObject) const;
   void refresh(QDir* theWrappedObject) const;
   QString  relativeFilePath(QDir* theWrappedObject, const QString&  fileName) const;
   bool  remove(QDir* theWrappedObject, const QString&  fileName);
   bool  rename(QDir* theWrappedObject, const QString&  oldName, const QString&  newName);
   bool  rmdir(QDir* theWrappedObject, const QString&  dirName) const;
   bool  rmpath(QDir* theWrappedObject, const QString&  dirPath) const;
   QDir  static_QDir_root();
   QString  static_QDir_rootPath();
   QStringList  static_QDir_searchPaths(const QString&  prefix);
   QChar  static_QDir_separator();
   bool  static_QDir_setCurrent(const QString&  path);
   void setFilter(QDir* theWrappedObject, QDir::Filters  filter);
   void setNameFilters(QDir* theWrappedObject, const QStringList&  nameFilters);
   void setPath(QDir* theWrappedObject, const QString&  path);
   void static_QDir_setSearchPaths(const QString&  prefix, const QStringList&  searchPaths);
   void setSorting(QDir* theWrappedObject, QDir::SortFlags  sort);
   QDir::SortFlags  sorting(QDir* theWrappedObject) const;
   QDir  static_QDir_temp();
   QString  static_QDir_tempPath();
   QString  static_QDir_toNativeSeparators(const QString&  pathName);
    QString py_toString(QDir*);
};





class PythonQtShell_QDirIterator : public QDirIterator
{
public:
    PythonQtShell_QDirIterator(const QDir&  dir, QDirIterator::IteratorFlags  flags = QDirIterator::NoIteratorFlags):QDirIterator(dir, flags),_wrapper(NULL) {};
    PythonQtShell_QDirIterator(const QString&  path, QDir::Filters  filter, QDirIterator::IteratorFlags  flags = QDirIterator::NoIteratorFlags):QDirIterator(path, filter, flags),_wrapper(NULL) {};
    PythonQtShell_QDirIterator(const QString&  path, QDirIterator::IteratorFlags  flags = QDirIterator::NoIteratorFlags):QDirIterator(path, flags),_wrapper(NULL) {};
    PythonQtShell_QDirIterator(const QString&  path, const QStringList&  nameFilters, QDir::Filters  filters = QDir::NoFilter, QDirIterator::IteratorFlags  flags = QDirIterator::NoIteratorFlags):QDirIterator(path, nameFilters, filters, flags),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QDirIterator : public QObject
{ Q_OBJECT
public:
Q_ENUMS(IteratorFlag )
Q_FLAGS(IteratorFlags )
enum IteratorFlag{
  NoIteratorFlags = QDirIterator::NoIteratorFlags,   FollowSymlinks = QDirIterator::FollowSymlinks,   Subdirectories = QDirIterator::Subdirectories};
Q_DECLARE_FLAGS(IteratorFlags, IteratorFlag)
public slots:
QDirIterator* new_QDirIterator(const QDir&  dir, QDirIterator::IteratorFlags  flags = QDirIterator::NoIteratorFlags);
QDirIterator* new_QDirIterator(const QString&  path, QDir::Filters  filter, QDirIterator::IteratorFlags  flags = QDirIterator::NoIteratorFlags);
QDirIterator* new_QDirIterator(const QString&  path, QDirIterator::IteratorFlags  flags = QDirIterator::NoIteratorFlags);
QDirIterator* new_QDirIterator(const QString&  path, const QStringList&  nameFilters, QDir::Filters  filters = QDir::NoFilter, QDirIterator::IteratorFlags  flags = QDirIterator::NoIteratorFlags);
void delete_QDirIterator(QDirIterator* obj) { delete obj; } 
   QFileInfo  fileInfo(QDirIterator* theWrappedObject) const;
   QString  fileName(QDirIterator* theWrappedObject) const;
   QString  filePath(QDirIterator* theWrappedObject) const;
   bool  hasNext(QDirIterator* theWrappedObject) const;
   QString  next(QDirIterator* theWrappedObject);
   QString  path(QDirIterator* theWrappedObject) const;
};





class PythonQtWrapper_QDynamicPropertyChangeEvent : public QObject
{ Q_OBJECT
public:
public slots:
QDynamicPropertyChangeEvent* new_QDynamicPropertyChangeEvent(const QByteArray&  name);
void delete_QDynamicPropertyChangeEvent(QDynamicPropertyChangeEvent* obj) { delete obj; } 
   QByteArray  propertyName(QDynamicPropertyChangeEvent* theWrappedObject) const;
};





class PythonQtWrapper_QEasingCurve : public QObject
{ Q_OBJECT
public:
Q_ENUMS(Type )
enum Type{
  Linear = QEasingCurve::Linear,   InQuad = QEasingCurve::InQuad,   OutQuad = QEasingCurve::OutQuad,   InOutQuad = QEasingCurve::InOutQuad,   OutInQuad = QEasingCurve::OutInQuad,   InCubic = QEasingCurve::InCubic,   OutCubic = QEasingCurve::OutCubic,   InOutCubic = QEasingCurve::InOutCubic,   OutInCubic = QEasingCurve::OutInCubic,   InQuart = QEasingCurve::InQuart,   OutQuart = QEasingCurve::OutQuart,   InOutQuart = QEasingCurve::InOutQuart,   OutInQuart = QEasingCurve::OutInQuart,   InQuint = QEasingCurve::InQuint,   OutQuint = QEasingCurve::OutQuint,   InOutQuint = QEasingCurve::InOutQuint,   OutInQuint = QEasingCurve::OutInQuint,   InSine = QEasingCurve::InSine,   OutSine = QEasingCurve::OutSine,   InOutSine = QEasingCurve::InOutSine,   OutInSine = QEasingCurve::OutInSine,   InExpo = QEasingCurve::InExpo,   OutExpo = QEasingCurve::OutExpo,   InOutExpo = QEasingCurve::InOutExpo,   OutInExpo = QEasingCurve::OutInExpo,   InCirc = QEasingCurve::InCirc,   OutCirc = QEasingCurve::OutCirc,   InOutCirc = QEasingCurve::InOutCirc,   OutInCirc = QEasingCurve::OutInCirc,   InElastic = QEasingCurve::InElastic,   OutElastic = QEasingCurve::OutElastic,   InOutElastic = QEasingCurve::InOutElastic,   OutInElastic = QEasingCurve::OutInElastic,   InBack = QEasingCurve::InBack,   OutBack = QEasingCurve::OutBack,   InOutBack = QEasingCurve::InOutBack,   OutInBack = QEasingCurve::OutInBack,   InBounce = QEasingCurve::InBounce,   OutBounce = QEasingCurve::OutBounce,   InOutBounce = QEasingCurve::InOutBounce,   OutInBounce = QEasingCurve::OutInBounce,   InCurve = QEasingCurve::InCurve,   OutCurve = QEasingCurve::OutCurve,   SineCurve = QEasingCurve::SineCurve,   CosineCurve = QEasingCurve::CosineCurve,   Custom = QEasingCurve::Custom,   NCurveTypes = QEasingCurve::NCurveTypes};
public slots:
QEasingCurve* new_QEasingCurve(QEasingCurve::Type  type = QEasingCurve::Linear);
QEasingCurve* new_QEasingCurve(const QEasingCurve&  other);
void delete_QEasingCurve(QEasingCurve* obj) { delete obj; } 
   qreal  amplitude(QEasingCurve* theWrappedObject) const;
   bool  __ne__(QEasingCurve* theWrappedObject, const QEasingCurve&  other) const;
   QEasingCurve*  operator_assign(QEasingCurve* theWrappedObject, const QEasingCurve&  other);
   bool  __eq__(QEasingCurve* theWrappedObject, const QEasingCurve&  other) const;
   qreal  overshoot(QEasingCurve* theWrappedObject) const;
   qreal  period(QEasingCurve* theWrappedObject) const;
   void setAmplitude(QEasingCurve* theWrappedObject, qreal  amplitude);
   void setOvershoot(QEasingCurve* theWrappedObject, qreal  overshoot);
   void setPeriod(QEasingCurve* theWrappedObject, qreal  period);
   void setType(QEasingCurve* theWrappedObject, QEasingCurve::Type  type);
   QEasingCurve::Type  type(QEasingCurve* theWrappedObject) const;
   qreal  valueForProgress(QEasingCurve* theWrappedObject, qreal  progress) const;
    QString py_toString(QEasingCurve*);
};





class PythonQtShell_QEvent : public QEvent
{
public:
    PythonQtShell_QEvent(QEvent::Type  type):QEvent(type),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QEvent : public QObject
{ Q_OBJECT
public:
Q_ENUMS(Type )
enum Type{
  None = QEvent::None,   Timer = QEvent::Timer,   MouseButtonPress = QEvent::MouseButtonPress,   MouseButtonRelease = QEvent::MouseButtonRelease,   MouseButtonDblClick = QEvent::MouseButtonDblClick,   MouseMove = QEvent::MouseMove,   KeyPress = QEvent::KeyPress,   KeyRelease = QEvent::KeyRelease,   FocusIn = QEvent::FocusIn,   FocusOut = QEvent::FocusOut,   Enter = QEvent::Enter,   Leave = QEvent::Leave,   Paint = QEvent::Paint,   Move = QEvent::Move,   Resize = QEvent::Resize,   Create = QEvent::Create,   Destroy = QEvent::Destroy,   Show = QEvent::Show,   Hide = QEvent::Hide,   Close = QEvent::Close,   Quit = QEvent::Quit,   ParentChange = QEvent::ParentChange,   ParentAboutToChange = QEvent::ParentAboutToChange,   ThreadChange = QEvent::ThreadChange,   WindowActivate = QEvent::WindowActivate,   WindowDeactivate = QEvent::WindowDeactivate,   ShowToParent = QEvent::ShowToParent,   HideToParent = QEvent::HideToParent,   Wheel = QEvent::Wheel,   WindowTitleChange = QEvent::WindowTitleChange,   WindowIconChange = QEvent::WindowIconChange,   ApplicationWindowIconChange = QEvent::ApplicationWindowIconChange,   ApplicationFontChange = QEvent::ApplicationFontChange,   ApplicationLayoutDirectionChange = QEvent::ApplicationLayoutDirectionChange,   ApplicationPaletteChange = QEvent::ApplicationPaletteChange,   PaletteChange = QEvent::PaletteChange,   Clipboard = QEvent::Clipboard,   Speech = QEvent::Speech,   MetaCall = QEvent::MetaCall,   SockAct = QEvent::SockAct,   WinEventAct = QEvent::WinEventAct,   DeferredDelete = QEvent::DeferredDelete,   DragEnter = QEvent::DragEnter,   DragMove = QEvent::DragMove,   DragLeave = QEvent::DragLeave,   Drop = QEvent::Drop,   DragResponse = QEvent::DragResponse,   ChildAdded = QEvent::ChildAdded,   ChildPolished = QEvent::ChildPolished,   ChildRemoved = QEvent::ChildRemoved,   ShowWindowRequest = QEvent::ShowWindowRequest,   PolishRequest = QEvent::PolishRequest,   Polish = QEvent::Polish,   LayoutRequest = QEvent::LayoutRequest,   UpdateRequest = QEvent::UpdateRequest,   UpdateLater = QEvent::UpdateLater,   EmbeddingControl = QEvent::EmbeddingControl,   ActivateControl = QEvent::ActivateControl,   DeactivateControl = QEvent::DeactivateControl,   ContextMenu = QEvent::ContextMenu,   InputMethod = QEvent::InputMethod,   AccessibilityPrepare = QEvent::AccessibilityPrepare,   TabletMove = QEvent::TabletMove,   LocaleChange = QEvent::LocaleChange,   LanguageChange = QEvent::LanguageChange,   LayoutDirectionChange = QEvent::LayoutDirectionChange,   Style = QEvent::Style,   TabletPress = QEvent::TabletPress,   TabletRelease = QEvent::TabletRelease,   OkRequest = QEvent::OkRequest,   HelpRequest = QEvent::HelpRequest,   IconDrag = QEvent::IconDrag,   FontChange = QEvent::FontChange,   EnabledChange = QEvent::EnabledChange,   ActivationChange = QEvent::ActivationChange,   StyleChange = QEvent::StyleChange,   IconTextChange = QEvent::IconTextChange,   ModifiedChange = QEvent::ModifiedChange,   MouseTrackingChange = QEvent::MouseTrackingChange,   WindowBlocked = QEvent::WindowBlocked,   WindowUnblocked = QEvent::WindowUnblocked,   WindowStateChange = QEvent::WindowStateChange,   ToolTip = QEvent::ToolTip,   WhatsThis = QEvent::WhatsThis,   StatusTip = QEvent::StatusTip,   ActionChanged = QEvent::ActionChanged,   ActionAdded = QEvent::ActionAdded,   ActionRemoved = QEvent::ActionRemoved,   FileOpen = QEvent::FileOpen,   Shortcut = QEvent::Shortcut,   ShortcutOverride = QEvent::ShortcutOverride,   WhatsThisClicked = QEvent::WhatsThisClicked,   ToolBarChange = QEvent::ToolBarChange,   ApplicationActivate = QEvent::ApplicationActivate,   ApplicationActivated = QEvent::ApplicationActivated,   ApplicationDeactivate = QEvent::ApplicationDeactivate,   ApplicationDeactivated = QEvent::ApplicationDeactivated,   QueryWhatsThis = QEvent::QueryWhatsThis,   EnterWhatsThisMode = QEvent::EnterWhatsThisMode,   LeaveWhatsThisMode = QEvent::LeaveWhatsThisMode,   ZOrderChange = QEvent::ZOrderChange,   HoverEnter = QEvent::HoverEnter,   HoverLeave = QEvent::HoverLeave,   HoverMove = QEvent::HoverMove,   AccessibilityHelp = QEvent::AccessibilityHelp,   AccessibilityDescription = QEvent::AccessibilityDescription,   AcceptDropsChange = QEvent::AcceptDropsChange,   MenubarUpdated = QEvent::MenubarUpdated,   ZeroTimerEvent = QEvent::ZeroTimerEvent,   GraphicsSceneMouseMove = QEvent::GraphicsSceneMouseMove,   GraphicsSceneMousePress = QEvent::GraphicsSceneMousePress,   GraphicsSceneMouseRelease = QEvent::GraphicsSceneMouseRelease,   GraphicsSceneMouseDoubleClick = QEvent::GraphicsSceneMouseDoubleClick,   GraphicsSceneContextMenu = QEvent::GraphicsSceneContextMenu,   GraphicsSceneHoverEnter = QEvent::GraphicsSceneHoverEnter,   GraphicsSceneHoverMove = QEvent::GraphicsSceneHoverMove,   GraphicsSceneHoverLeave = QEvent::GraphicsSceneHoverLeave,   GraphicsSceneHelp = QEvent::GraphicsSceneHelp,   GraphicsSceneDragEnter = QEvent::GraphicsSceneDragEnter,   GraphicsSceneDragMove = QEvent::GraphicsSceneDragMove,   GraphicsSceneDragLeave = QEvent::GraphicsSceneDragLeave,   GraphicsSceneDrop = QEvent::GraphicsSceneDrop,   GraphicsSceneWheel = QEvent::GraphicsSceneWheel,   KeyboardLayoutChange = QEvent::KeyboardLayoutChange,   DynamicPropertyChange = QEvent::DynamicPropertyChange,   TabletEnterProximity = QEvent::TabletEnterProximity,   TabletLeaveProximity = QEvent::TabletLeaveProximity,   NonClientAreaMouseMove = QEvent::NonClientAreaMouseMove,   NonClientAreaMouseButtonPress = QEvent::NonClientAreaMouseButtonPress,   NonClientAreaMouseButtonRelease = QEvent::NonClientAreaMouseButtonRelease,   NonClientAreaMouseButtonDblClick = QEvent::NonClientAreaMouseButtonDblClick,   MacSizeChange = QEvent::MacSizeChange,   ContentsRectChange = QEvent::ContentsRectChange,   MacGLWindowChange = QEvent::MacGLWindowChange,   FutureCallOut = QEvent::FutureCallOut,   GraphicsSceneResize = QEvent::GraphicsSceneResize,   GraphicsSceneMove = QEvent::GraphicsSceneMove,   CursorChange = QEvent::CursorChange,   ToolTipChange = QEvent::ToolTipChange,   NetworkReplyUpdated = QEvent::NetworkReplyUpdated,   GrabMouse = QEvent::GrabMouse,   UngrabMouse = QEvent::UngrabMouse,   GrabKeyboard = QEvent::GrabKeyboard,   UngrabKeyboard = QEvent::UngrabKeyboard,   MacGLClearDrawable = QEvent::MacGLClearDrawable,   StateMachineSignal = QEvent::StateMachineSignal,   StateMachineWrapped = QEvent::StateMachineWrapped,   TouchBegin = QEvent::TouchBegin,   TouchUpdate = QEvent::TouchUpdate,   TouchEnd = QEvent::TouchEnd,   NativeGesture = QEvent::NativeGesture,   RequestSoftwareInputPanel = QEvent::RequestSoftwareInputPanel,   CloseSoftwareInputPanel = QEvent::CloseSoftwareInputPanel,   UpdateSoftKeys = QEvent::UpdateSoftKeys,   WinIdChange = QEvent::WinIdChange,   Gesture = QEvent::Gesture,   GestureOverride = QEvent::GestureOverride,   User = QEvent::User,   MaxUser = QEvent::MaxUser};
public slots:
QEvent* new_QEvent(QEvent::Type  type);
void delete_QEvent(QEvent* obj) { delete obj; } 
   void accept(QEvent* theWrappedObject);
   void ignore(QEvent* theWrappedObject);
   bool  isAccepted(QEvent* theWrappedObject) const;
   int  static_QEvent_registerEventType(int  hint = -1);
   void setAccepted(QEvent* theWrappedObject, bool  accepted);
   bool  spontaneous(QEvent* theWrappedObject) const;
   QEvent::Type  type(QEvent* theWrappedObject) const;
    QString py_toString(QEvent*);
};





class PythonQtShell_QEventLoop : public QEventLoop
{
public:
    PythonQtShell_QEventLoop(QObject*  parent = 0):QEventLoop(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QEventLoop : public QObject
{ Q_OBJECT
public:
Q_ENUMS(ProcessEventsFlag )
Q_FLAGS(ProcessEventsFlags )
enum ProcessEventsFlag{
  AllEvents = QEventLoop::AllEvents,   ExcludeUserInputEvents = QEventLoop::ExcludeUserInputEvents,   ExcludeSocketNotifiers = QEventLoop::ExcludeSocketNotifiers,   WaitForMoreEvents = QEventLoop::WaitForMoreEvents,   X11ExcludeTimers = QEventLoop::X11ExcludeTimers,   DeferredDeletion = QEventLoop::DeferredDeletion,   EventLoopExec = QEventLoop::EventLoopExec,   DialogExec = QEventLoop::DialogExec};
Q_DECLARE_FLAGS(ProcessEventsFlags, ProcessEventsFlag)
public slots:
QEventLoop* new_QEventLoop(QObject*  parent = 0);
void delete_QEventLoop(QEventLoop* obj) { delete obj; } 
   int  exec(QEventLoop* theWrappedObject, QEventLoop::ProcessEventsFlags  flags = QEventLoop::AllEvents);
   void exit(QEventLoop* theWrappedObject, int  returnCode = 0);
   bool  isRunning(QEventLoop* theWrappedObject) const;
   bool  processEvents(QEventLoop* theWrappedObject, QEventLoop::ProcessEventsFlags  flags = QEventLoop::AllEvents);
   void processEvents(QEventLoop* theWrappedObject, QEventLoop::ProcessEventsFlags  flags, int  maximumTime);
   void wakeUp(QEventLoop* theWrappedObject);
};





class PythonQtShell_QEventTransition : public QEventTransition
{
public:
    PythonQtShell_QEventTransition(QObject*  object, QEvent::Type  type, QState*  sourceState = 0):QEventTransition(object, type, sourceState),_wrapper(NULL) {};
    PythonQtShell_QEventTransition(QState*  sourceState = 0):QEventTransition(sourceState),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  e);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual bool  eventTest(QEvent*  event);
virtual void onTransition(QEvent*  event);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QEventTransition : public QEventTransition
{ public:
inline bool  promoted_event(QEvent*  e) { return QEventTransition::event(e); }
inline bool  promoted_eventTest(QEvent*  event) { return QEventTransition::eventTest(event); }
inline void promoted_onTransition(QEvent*  event) { QEventTransition::onTransition(event); }
};

class PythonQtWrapper_QEventTransition : public QObject
{ Q_OBJECT
public:
public slots:
QEventTransition* new_QEventTransition(QObject*  object, QEvent::Type  type, QState*  sourceState = 0);
QEventTransition* new_QEventTransition(QState*  sourceState = 0);
void delete_QEventTransition(QEventTransition* obj) { delete obj; } 
   bool  event(QEventTransition* theWrappedObject, QEvent*  e);
   QObject*  eventSource(QEventTransition* theWrappedObject) const;
   bool  eventTest(QEventTransition* theWrappedObject, QEvent*  event);
   QEvent::Type  eventType(QEventTransition* theWrappedObject) const;
   void onTransition(QEventTransition* theWrappedObject, QEvent*  event);
   void setEventSource(QEventTransition* theWrappedObject, QObject*  object);
   void setEventType(QEventTransition* theWrappedObject, QEvent::Type  type);
};





class PythonQtShell_QFactoryInterface : public QFactoryInterface
{
public:
    PythonQtShell_QFactoryInterface():QFactoryInterface(),_wrapper(NULL) {};

virtual QStringList  keys() const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QFactoryInterface : public QObject
{ Q_OBJECT
public:
public slots:
QFactoryInterface* new_QFactoryInterface();
void delete_QFactoryInterface(QFactoryInterface* obj) { delete obj; } 
};





class PythonQtShell_QFile : public QFile
{
public:
    PythonQtShell_QFile():QFile(),_wrapper(NULL) {};
    PythonQtShell_QFile(QObject*  parent):QFile(parent),_wrapper(NULL) {};
    PythonQtShell_QFile(const QString&  name):QFile(name),_wrapper(NULL) {};
    PythonQtShell_QFile(const QString&  name, QObject*  parent):QFile(name, parent),_wrapper(NULL) {};

virtual bool  atEnd() const;
virtual qint64  bytesAvailable() const;
virtual qint64  bytesToWrite() const;
virtual bool  canReadLine() const;
virtual void childEvent(QChildEvent*  arg__1);
virtual void close();
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual QAbstractFileEngine*  fileEngine() const;
virtual bool  isSequential() const;
virtual bool  open(QIODevice::OpenMode  flags);
virtual qint64  pos() const;
virtual qint64  readData(char*  data, qint64  maxlen);
virtual qint64  readLineData(char*  data, qint64  maxlen);
virtual bool  reset();
virtual bool  seek(qint64  offset);
virtual qint64  size() const;
virtual void timerEvent(QTimerEvent*  arg__1);
virtual bool  waitForBytesWritten(int  msecs);
virtual bool  waitForReadyRead(int  msecs);
virtual qint64  writeData(const char*  data, qint64  len);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QFile : public QFile
{ public:
inline bool  promoted_atEnd() const { return QFile::atEnd(); }
inline void promoted_close() { QFile::close(); }
inline QAbstractFileEngine*  promoted_fileEngine() const { return QFile::fileEngine(); }
inline bool  promoted_isSequential() const { return QFile::isSequential(); }
inline bool  promoted_open(QIODevice::OpenMode  flags) { return QFile::open(flags); }
inline qint64  promoted_pos() const { return QFile::pos(); }
inline qint64  promoted_readData(char*  data, qint64  maxlen) { return QFile::readData(data, maxlen); }
inline qint64  promoted_readLineData(char*  data, qint64  maxlen) { return QFile::readLineData(data, maxlen); }
inline bool  promoted_seek(qint64  offset) { return QFile::seek(offset); }
inline qint64  promoted_size() const { return QFile::size(); }
inline qint64  promoted_writeData(const char*  data, qint64  len) { return QFile::writeData(data, len); }
};

class PythonQtWrapper_QFile : public QObject
{ Q_OBJECT
public:
Q_ENUMS(FileError MemoryMapFlags Permission )
Q_FLAGS(Permissions )
enum FileError{
  NoError = QFile::NoError,   ReadError = QFile::ReadError,   WriteError = QFile::WriteError,   FatalError = QFile::FatalError,   ResourceError = QFile::ResourceError,   OpenError = QFile::OpenError,   AbortError = QFile::AbortError,   TimeOutError = QFile::TimeOutError,   UnspecifiedError = QFile::UnspecifiedError,   RemoveError = QFile::RemoveError,   RenameError = QFile::RenameError,   PositionError = QFile::PositionError,   ResizeError = QFile::ResizeError,   PermissionsError = QFile::PermissionsError,   CopyError = QFile::CopyError};
enum MemoryMapFlags{
  NoOptions = QFile::NoOptions};
enum Permission{
  ReadOwner = QFile::ReadOwner,   WriteOwner = QFile::WriteOwner,   ExeOwner = QFile::ExeOwner,   ReadUser = QFile::ReadUser,   WriteUser = QFile::WriteUser,   ExeUser = QFile::ExeUser,   ReadGroup = QFile::ReadGroup,   WriteGroup = QFile::WriteGroup,   ExeGroup = QFile::ExeGroup,   ReadOther = QFile::ReadOther,   WriteOther = QFile::WriteOther,   ExeOther = QFile::ExeOther};
Q_DECLARE_FLAGS(Permissions, Permission)
public slots:
QFile* new_QFile();
QFile* new_QFile(QObject*  parent);
QFile* new_QFile(const QString&  name);
QFile* new_QFile(const QString&  name, QObject*  parent);
void delete_QFile(QFile* obj) { delete obj; } 
   bool  atEnd(QFile* theWrappedObject) const;
   void close(QFile* theWrappedObject);
   bool  static_QFile_copy(const QString&  fileName, const QString&  newName);
   bool  copy(QFile* theWrappedObject, const QString&  newName);
   QString  static_QFile_decodeName(const QByteArray&  localFileName);
   QByteArray  static_QFile_encodeName(const QString&  fileName);
   QFile::FileError  error(QFile* theWrappedObject) const;
   bool  exists(QFile* theWrappedObject) const;
   bool  static_QFile_exists(const QString&  fileName);
   QAbstractFileEngine*  fileEngine(QFile* theWrappedObject) const;
   QString  fileName(QFile* theWrappedObject) const;
   bool  flush(QFile* theWrappedObject);
   int  handle(QFile* theWrappedObject) const;
   bool  isSequential(QFile* theWrappedObject) const;
   bool  link(QFile* theWrappedObject, const QString&  newName);
   bool  static_QFile_link(const QString&  oldname, const QString&  newName);
   bool  open(QFile* theWrappedObject, QIODevice::OpenMode  flags);
   QFile::Permissions  permissions(QFile* theWrappedObject) const;
   QFile::Permissions  static_QFile_permissions(const QString&  filename);
   qint64  pos(QFile* theWrappedObject) const;
   qint64  readData(QFile* theWrappedObject, char*  data, qint64  maxlen);
   qint64  readLineData(QFile* theWrappedObject, char*  data, qint64  maxlen);
   bool  remove(QFile* theWrappedObject);
   bool  static_QFile_remove(const QString&  fileName);
   bool  rename(QFile* theWrappedObject, const QString&  newName);
   bool  static_QFile_rename(const QString&  oldName, const QString&  newName);
   bool  static_QFile_resize(const QString&  filename, qint64  sz);
   bool  resize(QFile* theWrappedObject, qint64  sz);
   bool  seek(QFile* theWrappedObject, qint64  offset);
   void setFileName(QFile* theWrappedObject, const QString&  name);
   bool  setPermissions(QFile* theWrappedObject, QFile::Permissions  permissionSpec);
   bool  static_QFile_setPermissions(const QString&  filename, QFile::Permissions  permissionSpec);
   qint64  size(QFile* theWrappedObject) const;
   QString  symLinkTarget(QFile* theWrappedObject) const;
   QString  static_QFile_symLinkTarget(const QString&  fileName);
   void unsetError(QFile* theWrappedObject);
   qint64  writeData(QFile* theWrappedObject, const char*  data, qint64  len);
};





class PythonQtWrapper_QFileInfo : public QObject
{ Q_OBJECT
public:
public slots:
QFileInfo* new_QFileInfo();
QFileInfo* new_QFileInfo(const QDir&  dir, const QString&  file);
QFileInfo* new_QFileInfo(const QFile&  file);
QFileInfo* new_QFileInfo(const QFileInfo&  fileinfo);
QFileInfo* new_QFileInfo(const QString&  file);
void delete_QFileInfo(QFileInfo* obj) { delete obj; } 
   QDir  absoluteDir(QFileInfo* theWrappedObject) const;
   QString  absoluteFilePath(QFileInfo* theWrappedObject) const;
   QString  absolutePath(QFileInfo* theWrappedObject) const;
   QString  baseName(QFileInfo* theWrappedObject) const;
   QString  bundleName(QFileInfo* theWrappedObject) const;
   bool  caching(QFileInfo* theWrappedObject) const;
   QString  canonicalFilePath(QFileInfo* theWrappedObject) const;
   QString  canonicalPath(QFileInfo* theWrappedObject) const;
   QString  completeBaseName(QFileInfo* theWrappedObject) const;
   QString  completeSuffix(QFileInfo* theWrappedObject) const;
   QDateTime  created(QFileInfo* theWrappedObject) const;
   QDir  dir(QFileInfo* theWrappedObject) const;
   bool  exists(QFileInfo* theWrappedObject) const;
   QString  fileName(QFileInfo* theWrappedObject) const;
   QString  filePath(QFileInfo* theWrappedObject) const;
   QString  group(QFileInfo* theWrappedObject) const;
   uint  groupId(QFileInfo* theWrappedObject) const;
   bool  isAbsolute(QFileInfo* theWrappedObject) const;
   bool  isBundle(QFileInfo* theWrappedObject) const;
   bool  isDir(QFileInfo* theWrappedObject) const;
   bool  isExecutable(QFileInfo* theWrappedObject) const;
   bool  isFile(QFileInfo* theWrappedObject) const;
   bool  isHidden(QFileInfo* theWrappedObject) const;
   bool  isReadable(QFileInfo* theWrappedObject) const;
   bool  isRelative(QFileInfo* theWrappedObject) const;
   bool  isRoot(QFileInfo* theWrappedObject) const;
   bool  isSymLink(QFileInfo* theWrappedObject) const;
   bool  isWritable(QFileInfo* theWrappedObject) const;
   QDateTime  lastModified(QFileInfo* theWrappedObject) const;
   QDateTime  lastRead(QFileInfo* theWrappedObject) const;
   bool  makeAbsolute(QFileInfo* theWrappedObject);
   bool  __ne__(QFileInfo* theWrappedObject, const QFileInfo&  fileinfo);
   bool  __eq__(QFileInfo* theWrappedObject, const QFileInfo&  fileinfo);
   QString  owner(QFileInfo* theWrappedObject) const;
   uint  ownerId(QFileInfo* theWrappedObject) const;
   QString  path(QFileInfo* theWrappedObject) const;
   bool  permission(QFileInfo* theWrappedObject, QFile::Permissions  permissions) const;
   QFile::Permissions  permissions(QFileInfo* theWrappedObject) const;
   void refresh(QFileInfo* theWrappedObject);
   void setCaching(QFileInfo* theWrappedObject, bool  on);
   void setFile(QFileInfo* theWrappedObject, const QDir&  dir, const QString&  file);
   void setFile(QFileInfo* theWrappedObject, const QFile&  file);
   void setFile(QFileInfo* theWrappedObject, const QString&  file);
   qint64  size(QFileInfo* theWrappedObject) const;
   QString  suffix(QFileInfo* theWrappedObject) const;
   QString  symLinkTarget(QFileInfo* theWrappedObject) const;
};





class PythonQtShell_QFileSystemWatcher : public QFileSystemWatcher
{
public:
    PythonQtShell_QFileSystemWatcher(QObject*  parent = 0):QFileSystemWatcher(parent),_wrapper(NULL) {};
    PythonQtShell_QFileSystemWatcher(const QStringList&  paths, QObject*  parent = 0):QFileSystemWatcher(paths, parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QFileSystemWatcher : public QObject
{ Q_OBJECT
public:
public slots:
QFileSystemWatcher* new_QFileSystemWatcher(QObject*  parent = 0);
QFileSystemWatcher* new_QFileSystemWatcher(const QStringList&  paths, QObject*  parent = 0);
void delete_QFileSystemWatcher(QFileSystemWatcher* obj) { delete obj; } 
   void addPath(QFileSystemWatcher* theWrappedObject, const QString&  file);
   void addPaths(QFileSystemWatcher* theWrappedObject, const QStringList&  files);
   QStringList  directories(QFileSystemWatcher* theWrappedObject) const;
   QStringList  files(QFileSystemWatcher* theWrappedObject) const;
   void removePath(QFileSystemWatcher* theWrappedObject, const QString&  file);
   void removePaths(QFileSystemWatcher* theWrappedObject, const QStringList&  files);
};





class PythonQtShell_QFinalState : public QFinalState
{
public:
    PythonQtShell_QFinalState(QState*  parent = 0):QFinalState(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  e);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void onEntry(QEvent*  event);
virtual void onExit(QEvent*  event);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QFinalState : public QFinalState
{ public:
inline bool  promoted_event(QEvent*  e) { return QFinalState::event(e); }
inline void promoted_onEntry(QEvent*  event) { QFinalState::onEntry(event); }
inline void promoted_onExit(QEvent*  event) { QFinalState::onExit(event); }
};

class PythonQtWrapper_QFinalState : public QObject
{ Q_OBJECT
public:
public slots:
QFinalState* new_QFinalState(QState*  parent = 0);
void delete_QFinalState(QFinalState* obj) { delete obj; } 
   bool  event(QFinalState* theWrappedObject, QEvent*  e);
   void onEntry(QFinalState* theWrappedObject, QEvent*  event);
   void onExit(QFinalState* theWrappedObject, QEvent*  event);
};





class PythonQtShell_QHistoryState : public QHistoryState
{
public:
    PythonQtShell_QHistoryState(QHistoryState::HistoryType  type, QState*  parent = 0):QHistoryState(type, parent),_wrapper(NULL) {};
    PythonQtShell_QHistoryState(QState*  parent = 0):QHistoryState(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  e);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void onEntry(QEvent*  event);
virtual void onExit(QEvent*  event);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QHistoryState : public QHistoryState
{ public:
inline bool  promoted_event(QEvent*  e) { return QHistoryState::event(e); }
inline void promoted_onEntry(QEvent*  event) { QHistoryState::onEntry(event); }
inline void promoted_onExit(QEvent*  event) { QHistoryState::onExit(event); }
};

class PythonQtWrapper_QHistoryState : public QObject
{ Q_OBJECT
public:
public slots:
QHistoryState* new_QHistoryState(QHistoryState::HistoryType  type, QState*  parent = 0);
QHistoryState* new_QHistoryState(QState*  parent = 0);
void delete_QHistoryState(QHistoryState* obj) { delete obj; } 
   QAbstractState*  defaultState(QHistoryState* theWrappedObject) const;
   bool  event(QHistoryState* theWrappedObject, QEvent*  e);
   QHistoryState::HistoryType  historyType(QHistoryState* theWrappedObject) const;
   void onEntry(QHistoryState* theWrappedObject, QEvent*  event);
   void onExit(QHistoryState* theWrappedObject, QEvent*  event);
   void setDefaultState(QHistoryState* theWrappedObject, QAbstractState*  state);
   void setHistoryType(QHistoryState* theWrappedObject, QHistoryState::HistoryType  type);
};





class PythonQtShell_QIODevice : public QIODevice
{
public:
    PythonQtShell_QIODevice():QIODevice(),_wrapper(NULL) {};
    PythonQtShell_QIODevice(QObject*  parent):QIODevice(parent),_wrapper(NULL) {};

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

class PythonQtPublicPromoter_QIODevice : public QIODevice
{ public:
inline bool  promoted_atEnd() const { return QIODevice::atEnd(); }
inline qint64  promoted_bytesAvailable() const { return QIODevice::bytesAvailable(); }
inline qint64  promoted_bytesToWrite() const { return QIODevice::bytesToWrite(); }
inline bool  promoted_canReadLine() const { return QIODevice::canReadLine(); }
inline void promoted_close() { QIODevice::close(); }
inline bool  promoted_isSequential() const { return QIODevice::isSequential(); }
inline bool  promoted_open(QIODevice::OpenMode  mode) { return QIODevice::open(mode); }
inline qint64  promoted_pos() const { return QIODevice::pos(); }
inline qint64  promoted_readLineData(char*  data, qint64  maxlen) { return QIODevice::readLineData(data, maxlen); }
inline bool  promoted_reset() { return QIODevice::reset(); }
inline bool  promoted_seek(qint64  pos) { return QIODevice::seek(pos); }
inline qint64  promoted_size() const { return QIODevice::size(); }
inline bool  promoted_waitForBytesWritten(int  msecs) { return QIODevice::waitForBytesWritten(msecs); }
inline bool  promoted_waitForReadyRead(int  msecs) { return QIODevice::waitForReadyRead(msecs); }
};

class PythonQtWrapper_QIODevice : public QObject
{ Q_OBJECT
public:
Q_ENUMS(OpenModeFlag )
Q_FLAGS(OpenMode )
enum OpenModeFlag{
  NotOpen = QIODevice::NotOpen,   ReadOnly = QIODevice::ReadOnly,   WriteOnly = QIODevice::WriteOnly,   ReadWrite = QIODevice::ReadWrite,   Append = QIODevice::Append,   Truncate = QIODevice::Truncate,   Text = QIODevice::Text,   Unbuffered = QIODevice::Unbuffered};
Q_DECLARE_FLAGS(OpenMode, OpenModeFlag)
public slots:
QIODevice* new_QIODevice();
QIODevice* new_QIODevice(QObject*  parent);
void delete_QIODevice(QIODevice* obj) { delete obj; } 
   bool  atEnd(QIODevice* theWrappedObject) const;
   qint64  bytesAvailable(QIODevice* theWrappedObject) const;
   qint64  bytesToWrite(QIODevice* theWrappedObject) const;
   bool  canReadLine(QIODevice* theWrappedObject) const;
   void close(QIODevice* theWrappedObject);
   QString  errorString(QIODevice* theWrappedObject) const;
   bool  getChar(QIODevice* theWrappedObject, char*  c);
   bool  isOpen(QIODevice* theWrappedObject) const;
   bool  isReadable(QIODevice* theWrappedObject) const;
   bool  isSequential(QIODevice* theWrappedObject) const;
   bool  isTextModeEnabled(QIODevice* theWrappedObject) const;
   bool  isWritable(QIODevice* theWrappedObject) const;
   bool  open(QIODevice* theWrappedObject, QIODevice::OpenMode  mode);
   QIODevice::OpenMode  openMode(QIODevice* theWrappedObject) const;
   QByteArray  peek(QIODevice* theWrappedObject, qint64  maxlen);
   qint64  pos(QIODevice* theWrappedObject) const;
   bool  putChar(QIODevice* theWrappedObject, char  c);
   QByteArray  read(QIODevice* theWrappedObject, qint64  maxlen);
   QByteArray  readAll(QIODevice* theWrappedObject);
   QByteArray  readLine(QIODevice* theWrappedObject, qint64  maxlen = 0);
   qint64  readLineData(QIODevice* theWrappedObject, char*  data, qint64  maxlen);
   bool  reset(QIODevice* theWrappedObject);
   bool  seek(QIODevice* theWrappedObject, qint64  pos);
   void setTextModeEnabled(QIODevice* theWrappedObject, bool  enabled);
   qint64  size(QIODevice* theWrappedObject) const;
   void ungetChar(QIODevice* theWrappedObject, char  c);
   bool  waitForBytesWritten(QIODevice* theWrappedObject, int  msecs);
   bool  waitForReadyRead(QIODevice* theWrappedObject, int  msecs);
   qint64  write(QIODevice* theWrappedObject, const QByteArray&  data);
   qint64  write(QIODevice* theWrappedObject, const char*  data);
};





class PythonQtWrapper_QLibraryInfo : public QObject
{ Q_OBJECT
public:
Q_ENUMS(LibraryLocation )
enum LibraryLocation{
  PrefixPath = QLibraryInfo::PrefixPath,   DocumentationPath = QLibraryInfo::DocumentationPath,   HeadersPath = QLibraryInfo::HeadersPath,   LibrariesPath = QLibraryInfo::LibrariesPath,   BinariesPath = QLibraryInfo::BinariesPath,   PluginsPath = QLibraryInfo::PluginsPath,   DataPath = QLibraryInfo::DataPath,   TranslationsPath = QLibraryInfo::TranslationsPath,   SettingsPath = QLibraryInfo::SettingsPath,   DemosPath = QLibraryInfo::DemosPath,   ExamplesPath = QLibraryInfo::ExamplesPath};
public slots:
void delete_QLibraryInfo(QLibraryInfo* obj) { delete obj; } 
   QDate  static_QLibraryInfo_buildDate();
   QString  static_QLibraryInfo_buildKey();
   QString  static_QLibraryInfo_licensedProducts();
   QString  static_QLibraryInfo_licensee();
   QString  static_QLibraryInfo_location(QLibraryInfo::LibraryLocation  arg__1);
};





class PythonQtShell_QMimeData : public QMimeData
{
public:
    PythonQtShell_QMimeData():QMimeData(),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual QStringList  formats() const;
virtual bool  hasFormat(const QString&  mimetype) const;
virtual QVariant  retrieveData(const QString&  mimetype, QVariant::Type  preferredType) const;
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QMimeData : public QMimeData
{ public:
inline QStringList  promoted_formats() const { return QMimeData::formats(); }
inline bool  promoted_hasFormat(const QString&  mimetype) const { return QMimeData::hasFormat(mimetype); }
inline QVariant  promoted_retrieveData(const QString&  mimetype, QVariant::Type  preferredType) const { return QMimeData::retrieveData(mimetype, preferredType); }
};

class PythonQtWrapper_QMimeData : public QObject
{ Q_OBJECT
public:
public slots:
QMimeData* new_QMimeData();
void delete_QMimeData(QMimeData* obj) { delete obj; } 
   void clear(QMimeData* theWrappedObject);
   QVariant  colorData(QMimeData* theWrappedObject) const;
   QByteArray  data(QMimeData* theWrappedObject, const QString&  mimetype) const;
   QStringList  formats(QMimeData* theWrappedObject) const;
   bool  hasColor(QMimeData* theWrappedObject) const;
   bool  hasFormat(QMimeData* theWrappedObject, const QString&  mimetype) const;
   bool  hasHtml(QMimeData* theWrappedObject) const;
   bool  hasImage(QMimeData* theWrappedObject) const;
   bool  hasText(QMimeData* theWrappedObject) const;
   bool  hasUrls(QMimeData* theWrappedObject) const;
   QString  html(QMimeData* theWrappedObject) const;
   QVariant  imageData(QMimeData* theWrappedObject) const;
   void removeFormat(QMimeData* theWrappedObject, const QString&  mimetype);
   QVariant  retrieveData(QMimeData* theWrappedObject, const QString&  mimetype, QVariant::Type  preferredType) const;
   void setColorData(QMimeData* theWrappedObject, const QVariant&  color);
   void setData(QMimeData* theWrappedObject, const QString&  mimetype, const QByteArray&  data);
   void setHtml(QMimeData* theWrappedObject, const QString&  html);
   void setImageData(QMimeData* theWrappedObject, const QVariant&  image);
   void setText(QMimeData* theWrappedObject, const QString&  text);
   void setUrls(QMimeData* theWrappedObject, const QList<QUrl >&  urls);
   QString  text(QMimeData* theWrappedObject) const;
   QList<QUrl >  urls(QMimeData* theWrappedObject) const;
};





class PythonQtWrapper_QModelIndex : public QObject
{ Q_OBJECT
public:
public slots:
QModelIndex* new_QModelIndex();
QModelIndex* new_QModelIndex(const QModelIndex&  other);
void delete_QModelIndex(QModelIndex* obj) { delete obj; } 
   QModelIndex  child(QModelIndex* theWrappedObject, int  row, int  column) const;
   int  column(QModelIndex* theWrappedObject) const;
   QVariant  data(QModelIndex* theWrappedObject, int  role = Qt::DisplayRole) const;
   Qt::ItemFlags  flags(QModelIndex* theWrappedObject) const;
   qint64  internalId(QModelIndex* theWrappedObject) const;
   void*  internalPointer(QModelIndex* theWrappedObject) const;
   bool  isValid(QModelIndex* theWrappedObject) const;
   const QAbstractItemModel*  model(QModelIndex* theWrappedObject) const;
   bool  __ne__(QModelIndex* theWrappedObject, const QModelIndex&  other) const;
   bool  __lt__(QModelIndex* theWrappedObject, const QModelIndex&  other) const;
   bool  __eq__(QModelIndex* theWrappedObject, const QModelIndex&  other) const;
   QModelIndex  parent(QModelIndex* theWrappedObject) const;
   int  row(QModelIndex* theWrappedObject) const;
   QModelIndex  sibling(QModelIndex* theWrappedObject, int  row, int  column) const;
    QString py_toString(QModelIndex*);
};


