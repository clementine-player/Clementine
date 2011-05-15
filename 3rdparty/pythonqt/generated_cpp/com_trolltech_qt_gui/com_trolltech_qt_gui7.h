#include <PythonQt.h>
#include <QObject>
#include <QSize>
#include <QStringList>
#include <QVariant>
#include <qabstractitemmodel.h>
#include <qaction.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qbrush.h>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qcursor.h>
#include <qdatastream.h>
#include <qevent.h>
#include <qfont.h>
#include <qgraphicseffect.h>
#include <qgraphicsproxywidget.h>
#include <qicon.h>
#include <qinputcontext.h>
#include <qkeysequence.h>
#include <qlayout.h>
#include <qlist.h>
#include <qlocale.h>
#include <qmargins.h>
#include <qmimedata.h>
#include <qobject.h>
#include <qpaintdevice.h>
#include <qpaintengine.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qrect.h>
#include <qregion.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qstackedwidget.h>
#include <qstandarditemmodel.h>
#include <qstatusbar.h>
#include <qstringlist.h>
#include <qstringlistmodel.h>
#include <qstyle.h>
#include <qstylefactory.h>
#include <qstyleoption.h>
#include <qtransform.h>
#include <qwidget.h>



class PythonQtShell_QStackedWidget : public QStackedWidget
{
public:
    PythonQtShell_QStackedWidget(QWidget*  parent = 0):QStackedWidget(parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  arg__1);
virtual void childEvent(QChildEvent*  arg__1);
virtual void closeEvent(QCloseEvent*  arg__1);
virtual void contextMenuEvent(QContextMenuEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual int  devType() const;
virtual void dragEnterEvent(QDragEnterEvent*  arg__1);
virtual void dragLeaveEvent(QDragLeaveEvent*  arg__1);
virtual void dragMoveEvent(QDragMoveEvent*  arg__1);
virtual void dropEvent(QDropEvent*  arg__1);
virtual void enterEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  e);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void focusInEvent(QFocusEvent*  arg__1);
virtual bool  focusNextPrevChild(bool  next);
virtual void focusOutEvent(QFocusEvent*  arg__1);
virtual int  heightForWidth(int  arg__1) const;
virtual void hideEvent(QHideEvent*  arg__1);
virtual void inputMethodEvent(QInputMethodEvent*  arg__1);
virtual QVariant  inputMethodQuery(Qt::InputMethodQuery  arg__1) const;
virtual void keyPressEvent(QKeyEvent*  arg__1);
virtual void keyReleaseEvent(QKeyEvent*  arg__1);
virtual void languageChange();
virtual void leaveEvent(QEvent*  arg__1);
virtual int  metric(QPaintDevice::PaintDeviceMetric  arg__1) const;
virtual QSize  minimumSizeHint() const;
virtual void mouseDoubleClickEvent(QMouseEvent*  arg__1);
virtual void mouseMoveEvent(QMouseEvent*  arg__1);
virtual void mousePressEvent(QMouseEvent*  arg__1);
virtual void mouseReleaseEvent(QMouseEvent*  arg__1);
virtual void moveEvent(QMoveEvent*  arg__1);
virtual QPaintEngine*  paintEngine() const;
virtual void paintEvent(QPaintEvent*  arg__1);
virtual void resizeEvent(QResizeEvent*  arg__1);
virtual void showEvent(QShowEvent*  arg__1);
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QStackedWidget : public QStackedWidget
{ public:
inline bool  promoted_event(QEvent*  e) { return QStackedWidget::event(e); }
};

class PythonQtWrapper_QStackedWidget : public QObject
{ Q_OBJECT
public:
public slots:
QStackedWidget* new_QStackedWidget(QWidget*  parent = 0);
void delete_QStackedWidget(QStackedWidget* obj) { delete obj; } 
   int  addWidget(QStackedWidget* theWrappedObject, QWidget*  w);
   int  count(QStackedWidget* theWrappedObject) const;
   int  currentIndex(QStackedWidget* theWrappedObject) const;
   QWidget*  currentWidget(QStackedWidget* theWrappedObject) const;
   bool  event(QStackedWidget* theWrappedObject, QEvent*  e);
   int  indexOf(QStackedWidget* theWrappedObject, QWidget*  arg__1) const;
   int  insertWidget(QStackedWidget* theWrappedObject, int  index, QWidget*  w);
   void removeWidget(QStackedWidget* theWrappedObject, QWidget*  w);
   QWidget*  widget(QStackedWidget* theWrappedObject, int  arg__1) const;
};





class PythonQtShell_QStandardItem : public QStandardItem
{
public:
    PythonQtShell_QStandardItem():QStandardItem(),_wrapper(NULL) {};
    PythonQtShell_QStandardItem(const QIcon&  icon, const QString&  text):QStandardItem(icon, text),_wrapper(NULL) {};
    PythonQtShell_QStandardItem(const QStandardItem&  other):QStandardItem(other),_wrapper(NULL) {};
    PythonQtShell_QStandardItem(const QString&  text):QStandardItem(text),_wrapper(NULL) {};
    PythonQtShell_QStandardItem(int  rows, int  columns = 1):QStandardItem(rows, columns),_wrapper(NULL) {};

virtual QStandardItem*  clone() const;
virtual QVariant  data(int  role = Qt::UserRole + 1) const;
virtual bool  __lt__(const QStandardItem&  other) const;
virtual void read(QDataStream&  in);
virtual void setData(const QVariant&  value, int  role = Qt::UserRole + 1);
virtual int  type() const;
virtual void write(QDataStream&  out) const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QStandardItem : public QStandardItem
{ public:
inline QStandardItem*  promoted_clone() const { return QStandardItem::clone(); }
inline QVariant  promoted_data(int  role = Qt::UserRole + 1) const { return QStandardItem::data(role); }
inline void promoted_setData(const QVariant&  value, int  role = Qt::UserRole + 1) { QStandardItem::setData(value, role); }
inline int  promoted_type() const { return QStandardItem::type(); }
};

class PythonQtWrapper_QStandardItem : public QObject
{ Q_OBJECT
public:
Q_ENUMS(ItemType )
enum ItemType{
  Type = QStandardItem::Type,   UserType = QStandardItem::UserType};
public slots:
QStandardItem* new_QStandardItem();
QStandardItem* new_QStandardItem(const QIcon&  icon, const QString&  text);
QStandardItem* new_QStandardItem(const QString&  text);
QStandardItem* new_QStandardItem(int  rows, int  columns = 1);
void delete_QStandardItem(QStandardItem* obj) { delete obj; } 
   QString  accessibleDescription(QStandardItem* theWrappedObject) const;
   QString  accessibleText(QStandardItem* theWrappedObject) const;
   void appendColumn(QStandardItem* theWrappedObject, const QList<QStandardItem* >&  items);
   void appendRow(QStandardItem* theWrappedObject, QStandardItem*  item);
   void appendRow(QStandardItem* theWrappedObject, const QList<QStandardItem* >&  items);
   void appendRows(QStandardItem* theWrappedObject, const QList<QStandardItem* >&  items);
   QBrush  background(QStandardItem* theWrappedObject) const;
   Qt::CheckState  checkState(QStandardItem* theWrappedObject) const;
   QStandardItem*  child(QStandardItem* theWrappedObject, int  row, int  column = 0) const;
   QStandardItem*  clone(QStandardItem* theWrappedObject) const;
   int  column(QStandardItem* theWrappedObject) const;
   int  columnCount(QStandardItem* theWrappedObject) const;
   QVariant  data(QStandardItem* theWrappedObject, int  role = Qt::UserRole + 1) const;
   Qt::ItemFlags  flags(QStandardItem* theWrappedObject) const;
   QFont  font(QStandardItem* theWrappedObject) const;
   QBrush  foreground(QStandardItem* theWrappedObject) const;
   bool  hasChildren(QStandardItem* theWrappedObject) const;
   QIcon  icon(QStandardItem* theWrappedObject) const;
   QModelIndex  index(QStandardItem* theWrappedObject) const;
   void insertColumn(QStandardItem* theWrappedObject, int  column, const QList<QStandardItem* >&  items);
   void insertColumns(QStandardItem* theWrappedObject, int  column, int  count);
   void insertRow(QStandardItem* theWrappedObject, int  row, QStandardItem*  item);
   void insertRow(QStandardItem* theWrappedObject, int  row, const QList<QStandardItem* >&  items);
   void insertRows(QStandardItem* theWrappedObject, int  row, const QList<QStandardItem* >&  items);
   void insertRows(QStandardItem* theWrappedObject, int  row, int  count);
   bool  isCheckable(QStandardItem* theWrappedObject) const;
   bool  isDragEnabled(QStandardItem* theWrappedObject) const;
   bool  isDropEnabled(QStandardItem* theWrappedObject) const;
   bool  isEditable(QStandardItem* theWrappedObject) const;
   bool  isEnabled(QStandardItem* theWrappedObject) const;
   bool  isSelectable(QStandardItem* theWrappedObject) const;
   bool  isTristate(QStandardItem* theWrappedObject) const;
   QStandardItemModel*  model(QStandardItem* theWrappedObject) const;
   void writeTo(QStandardItem* theWrappedObject, QDataStream&  out);
   void readFrom(QStandardItem* theWrappedObject, QDataStream&  in);
   QStandardItem*  parent(QStandardItem* theWrappedObject) const;
   void removeColumn(QStandardItem* theWrappedObject, int  column);
   void removeColumns(QStandardItem* theWrappedObject, int  column, int  count);
   void removeRow(QStandardItem* theWrappedObject, int  row);
   void removeRows(QStandardItem* theWrappedObject, int  row, int  count);
   int  row(QStandardItem* theWrappedObject) const;
   int  rowCount(QStandardItem* theWrappedObject) const;
   void setAccessibleDescription(QStandardItem* theWrappedObject, const QString&  accessibleDescription);
   void setAccessibleText(QStandardItem* theWrappedObject, const QString&  accessibleText);
   void setBackground(QStandardItem* theWrappedObject, const QBrush&  brush);
   void setCheckState(QStandardItem* theWrappedObject, Qt::CheckState  checkState);
   void setCheckable(QStandardItem* theWrappedObject, bool  checkable);
   void setChild(QStandardItem* theWrappedObject, int  row, QStandardItem*  item);
   void setChild(QStandardItem* theWrappedObject, int  row, int  column, QStandardItem*  item);
   void setColumnCount(QStandardItem* theWrappedObject, int  columns);
   void setData(QStandardItem* theWrappedObject, const QVariant&  value, int  role = Qt::UserRole + 1);
   void setDragEnabled(QStandardItem* theWrappedObject, bool  dragEnabled);
   void setDropEnabled(QStandardItem* theWrappedObject, bool  dropEnabled);
   void setEditable(QStandardItem* theWrappedObject, bool  editable);
   void setEnabled(QStandardItem* theWrappedObject, bool  enabled);
   void setFlags(QStandardItem* theWrappedObject, Qt::ItemFlags  flags);
   void setFont(QStandardItem* theWrappedObject, const QFont&  font);
   void setForeground(QStandardItem* theWrappedObject, const QBrush&  brush);
   void setIcon(QStandardItem* theWrappedObject, const QIcon&  icon);
   void setRowCount(QStandardItem* theWrappedObject, int  rows);
   void setSelectable(QStandardItem* theWrappedObject, bool  selectable);
   void setSizeHint(QStandardItem* theWrappedObject, const QSize&  sizeHint);
   void setStatusTip(QStandardItem* theWrappedObject, const QString&  statusTip);
   void setText(QStandardItem* theWrappedObject, const QString&  text);
   void setTextAlignment(QStandardItem* theWrappedObject, Qt::Alignment  textAlignment);
   void setToolTip(QStandardItem* theWrappedObject, const QString&  toolTip);
   void setTristate(QStandardItem* theWrappedObject, bool  tristate);
   void setWhatsThis(QStandardItem* theWrappedObject, const QString&  whatsThis);
   QSize  sizeHint(QStandardItem* theWrappedObject) const;
   void sortChildren(QStandardItem* theWrappedObject, int  column, Qt::SortOrder  order = Qt::AscendingOrder);
   QString  statusTip(QStandardItem* theWrappedObject) const;
   QStandardItem*  takeChild(QStandardItem* theWrappedObject, int  row, int  column = 0);
   QList<QStandardItem* >  takeColumn(QStandardItem* theWrappedObject, int  column);
   QList<QStandardItem* >  takeRow(QStandardItem* theWrappedObject, int  row);
   QString  text(QStandardItem* theWrappedObject) const;
   Qt::Alignment  textAlignment(QStandardItem* theWrappedObject) const;
   QString  toolTip(QStandardItem* theWrappedObject) const;
   int  type(QStandardItem* theWrappedObject) const;
   QString  whatsThis(QStandardItem* theWrappedObject) const;
};





class PythonQtShell_QStandardItemModel : public QStandardItemModel
{
public:
    PythonQtShell_QStandardItemModel(QObject*  parent = 0):QStandardItemModel(parent),_wrapper(NULL) {};
    PythonQtShell_QStandardItemModel(int  rows, int  columns, QObject*  parent = 0):QStandardItemModel(rows, columns, parent),_wrapper(NULL) {};

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
virtual QList<QModelIndex >  match(const QModelIndex&  start, int  role, const QVariant&  value, int  hits, Qt::MatchFlags  flags) const;
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

class PythonQtPublicPromoter_QStandardItemModel : public QStandardItemModel
{ public:
inline int  promoted_columnCount(const QModelIndex&  parent = QModelIndex()) const { return QStandardItemModel::columnCount(parent); }
inline QVariant  promoted_data(const QModelIndex&  index, int  role = Qt::DisplayRole) const { return QStandardItemModel::data(index, role); }
inline bool  promoted_dropMimeData(const QMimeData*  data, Qt::DropAction  action, int  row, int  column, const QModelIndex&  parent) { return QStandardItemModel::dropMimeData(data, action, row, column, parent); }
inline Qt::ItemFlags  promoted_flags(const QModelIndex&  index) const { return QStandardItemModel::flags(index); }
inline bool  promoted_hasChildren(const QModelIndex&  parent = QModelIndex()) const { return QStandardItemModel::hasChildren(parent); }
inline QVariant  promoted_headerData(int  section, Qt::Orientation  orientation, int  role = Qt::DisplayRole) const { return QStandardItemModel::headerData(section, orientation, role); }
inline QModelIndex  promoted_index(int  row, int  column, const QModelIndex&  parent = QModelIndex()) const { return QStandardItemModel::index(row, column, parent); }
inline bool  promoted_insertColumns(int  column, int  count, const QModelIndex&  parent = QModelIndex()) { return QStandardItemModel::insertColumns(column, count, parent); }
inline bool  promoted_insertRows(int  row, int  count, const QModelIndex&  parent = QModelIndex()) { return QStandardItemModel::insertRows(row, count, parent); }
inline QMap<int , QVariant >  promoted_itemData(const QModelIndex&  index) const { return QStandardItemModel::itemData(index); }
inline QMimeData*  promoted_mimeData(const QList<QModelIndex >&  indexes) const { return QStandardItemModel::mimeData(indexes); }
inline QStringList  promoted_mimeTypes() const { return QStandardItemModel::mimeTypes(); }
inline QModelIndex  promoted_parent(const QModelIndex&  child) const { return QStandardItemModel::parent(child); }
inline bool  promoted_removeColumns(int  column, int  count, const QModelIndex&  parent = QModelIndex()) { return QStandardItemModel::removeColumns(column, count, parent); }
inline bool  promoted_removeRows(int  row, int  count, const QModelIndex&  parent = QModelIndex()) { return QStandardItemModel::removeRows(row, count, parent); }
inline int  promoted_rowCount(const QModelIndex&  parent = QModelIndex()) const { return QStandardItemModel::rowCount(parent); }
inline bool  promoted_setData(const QModelIndex&  index, const QVariant&  value, int  role = Qt::EditRole) { return QStandardItemModel::setData(index, value, role); }
inline bool  promoted_setHeaderData(int  section, Qt::Orientation  orientation, const QVariant&  value, int  role = Qt::EditRole) { return QStandardItemModel::setHeaderData(section, orientation, value, role); }
inline bool  promoted_setItemData(const QModelIndex&  index, const QMap<int , QVariant >&  roles) { return QStandardItemModel::setItemData(index, roles); }
inline void promoted_sort(int  column, Qt::SortOrder  order = Qt::AscendingOrder) { QStandardItemModel::sort(column, order); }
inline Qt::DropActions  promoted_supportedDropActions() const { return QStandardItemModel::supportedDropActions(); }
};

class PythonQtWrapper_QStandardItemModel : public QObject
{ Q_OBJECT
public:
public slots:
QStandardItemModel* new_QStandardItemModel(QObject*  parent = 0);
QStandardItemModel* new_QStandardItemModel(int  rows, int  columns, QObject*  parent = 0);
void delete_QStandardItemModel(QStandardItemModel* obj) { delete obj; } 
   void appendColumn(QStandardItemModel* theWrappedObject, const QList<QStandardItem* >&  items);
   void appendRow(QStandardItemModel* theWrappedObject, QStandardItem*  item);
   void appendRow(QStandardItemModel* theWrappedObject, const QList<QStandardItem* >&  items);
   void clear(QStandardItemModel* theWrappedObject);
   int  columnCount(QStandardItemModel* theWrappedObject, const QModelIndex&  parent = QModelIndex()) const;
   QVariant  data(QStandardItemModel* theWrappedObject, const QModelIndex&  index, int  role = Qt::DisplayRole) const;
   bool  dropMimeData(QStandardItemModel* theWrappedObject, const QMimeData*  data, Qt::DropAction  action, int  row, int  column, const QModelIndex&  parent);
   QList<QStandardItem* >  findItems(QStandardItemModel* theWrappedObject, const QString&  text, Qt::MatchFlags  flags = Qt::MatchExactly, int  column = 0) const;
   Qt::ItemFlags  flags(QStandardItemModel* theWrappedObject, const QModelIndex&  index) const;
   bool  hasChildren(QStandardItemModel* theWrappedObject, const QModelIndex&  parent = QModelIndex()) const;
   QVariant  headerData(QStandardItemModel* theWrappedObject, int  section, Qt::Orientation  orientation, int  role = Qt::DisplayRole) const;
   QStandardItem*  horizontalHeaderItem(QStandardItemModel* theWrappedObject, int  column) const;
   QModelIndex  index(QStandardItemModel* theWrappedObject, int  row, int  column, const QModelIndex&  parent = QModelIndex()) const;
   QModelIndex  indexFromItem(QStandardItemModel* theWrappedObject, const QStandardItem*  item) const;
   void insertColumn(QStandardItemModel* theWrappedObject, int  column, const QList<QStandardItem* >&  items);
   bool  insertColumns(QStandardItemModel* theWrappedObject, int  column, int  count, const QModelIndex&  parent = QModelIndex());
   void insertRow(QStandardItemModel* theWrappedObject, int  row, QStandardItem*  item);
   void insertRow(QStandardItemModel* theWrappedObject, int  row, const QList<QStandardItem* >&  items);
   bool  insertRows(QStandardItemModel* theWrappedObject, int  row, int  count, const QModelIndex&  parent = QModelIndex());
   QStandardItem*  invisibleRootItem(QStandardItemModel* theWrappedObject) const;
   QStandardItem*  item(QStandardItemModel* theWrappedObject, int  row, int  column = 0) const;
   QMap<int , QVariant >  itemData(QStandardItemModel* theWrappedObject, const QModelIndex&  index) const;
   QStandardItem*  itemFromIndex(QStandardItemModel* theWrappedObject, const QModelIndex&  index) const;
   const QStandardItem*  itemPrototype(QStandardItemModel* theWrappedObject) const;
   QMimeData*  mimeData(QStandardItemModel* theWrappedObject, const QList<QModelIndex >&  indexes) const;
   QStringList  mimeTypes(QStandardItemModel* theWrappedObject) const;
   QModelIndex  parent(QStandardItemModel* theWrappedObject, const QModelIndex&  child) const;
   bool  removeColumns(QStandardItemModel* theWrappedObject, int  column, int  count, const QModelIndex&  parent = QModelIndex());
   bool  removeRows(QStandardItemModel* theWrappedObject, int  row, int  count, const QModelIndex&  parent = QModelIndex());
   int  rowCount(QStandardItemModel* theWrappedObject, const QModelIndex&  parent = QModelIndex()) const;
   void setColumnCount(QStandardItemModel* theWrappedObject, int  columns);
   bool  setData(QStandardItemModel* theWrappedObject, const QModelIndex&  index, const QVariant&  value, int  role = Qt::EditRole);
   bool  setHeaderData(QStandardItemModel* theWrappedObject, int  section, Qt::Orientation  orientation, const QVariant&  value, int  role = Qt::EditRole);
   void setHorizontalHeaderItem(QStandardItemModel* theWrappedObject, int  column, QStandardItem*  item);
   void setHorizontalHeaderLabels(QStandardItemModel* theWrappedObject, const QStringList&  labels);
   void setItem(QStandardItemModel* theWrappedObject, int  row, QStandardItem*  item);
   void setItem(QStandardItemModel* theWrappedObject, int  row, int  column, QStandardItem*  item);
   bool  setItemData(QStandardItemModel* theWrappedObject, const QModelIndex&  index, const QMap<int , QVariant >&  roles);
   void setItemPrototype(QStandardItemModel* theWrappedObject, const QStandardItem*  item);
   void setRowCount(QStandardItemModel* theWrappedObject, int  rows);
   void setSortRole(QStandardItemModel* theWrappedObject, int  role);
   void setVerticalHeaderItem(QStandardItemModel* theWrappedObject, int  row, QStandardItem*  item);
   void setVerticalHeaderLabels(QStandardItemModel* theWrappedObject, const QStringList&  labels);
   void sort(QStandardItemModel* theWrappedObject, int  column, Qt::SortOrder  order = Qt::AscendingOrder);
   int  sortRole(QStandardItemModel* theWrappedObject) const;
   Qt::DropActions  supportedDropActions(QStandardItemModel* theWrappedObject) const;
   QList<QStandardItem* >  takeColumn(QStandardItemModel* theWrappedObject, int  column);
   QStandardItem*  takeHorizontalHeaderItem(QStandardItemModel* theWrappedObject, int  column);
   QStandardItem*  takeItem(QStandardItemModel* theWrappedObject, int  row, int  column = 0);
   QList<QStandardItem* >  takeRow(QStandardItemModel* theWrappedObject, int  row);
   QStandardItem*  takeVerticalHeaderItem(QStandardItemModel* theWrappedObject, int  row);
   QStandardItem*  verticalHeaderItem(QStandardItemModel* theWrappedObject, int  row) const;
};





class PythonQtShell_QStatusBar : public QStatusBar
{
public:
    PythonQtShell_QStatusBar(QWidget*  parent = 0):QStatusBar(parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  arg__1);
virtual void childEvent(QChildEvent*  arg__1);
virtual void closeEvent(QCloseEvent*  arg__1);
virtual void contextMenuEvent(QContextMenuEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual int  devType() const;
virtual void dragEnterEvent(QDragEnterEvent*  arg__1);
virtual void dragLeaveEvent(QDragLeaveEvent*  arg__1);
virtual void dragMoveEvent(QDragMoveEvent*  arg__1);
virtual void dropEvent(QDropEvent*  arg__1);
virtual void enterEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void focusInEvent(QFocusEvent*  arg__1);
virtual bool  focusNextPrevChild(bool  next);
virtual void focusOutEvent(QFocusEvent*  arg__1);
virtual int  heightForWidth(int  arg__1) const;
virtual void hideEvent(QHideEvent*  arg__1);
virtual void inputMethodEvent(QInputMethodEvent*  arg__1);
virtual QVariant  inputMethodQuery(Qt::InputMethodQuery  arg__1) const;
virtual void keyPressEvent(QKeyEvent*  arg__1);
virtual void keyReleaseEvent(QKeyEvent*  arg__1);
virtual void languageChange();
virtual void leaveEvent(QEvent*  arg__1);
virtual int  metric(QPaintDevice::PaintDeviceMetric  arg__1) const;
virtual QSize  minimumSizeHint() const;
virtual void mouseDoubleClickEvent(QMouseEvent*  arg__1);
virtual void mouseMoveEvent(QMouseEvent*  arg__1);
virtual void mousePressEvent(QMouseEvent*  arg__1);
virtual void mouseReleaseEvent(QMouseEvent*  arg__1);
virtual void moveEvent(QMoveEvent*  arg__1);
virtual QPaintEngine*  paintEngine() const;
virtual void paintEvent(QPaintEvent*  arg__1);
virtual void resizeEvent(QResizeEvent*  arg__1);
virtual void showEvent(QShowEvent*  arg__1);
virtual QSize  sizeHint() const;
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QStatusBar : public QStatusBar
{ public:
inline bool  promoted_event(QEvent*  arg__1) { return QStatusBar::event(arg__1); }
inline void promoted_paintEvent(QPaintEvent*  arg__1) { QStatusBar::paintEvent(arg__1); }
inline void promoted_resizeEvent(QResizeEvent*  arg__1) { QStatusBar::resizeEvent(arg__1); }
inline void promoted_showEvent(QShowEvent*  arg__1) { QStatusBar::showEvent(arg__1); }
};

class PythonQtWrapper_QStatusBar : public QObject
{ Q_OBJECT
public:
public slots:
QStatusBar* new_QStatusBar(QWidget*  parent = 0);
void delete_QStatusBar(QStatusBar* obj) { delete obj; } 
   void addPermanentWidget(QStatusBar* theWrappedObject, QWidget*  widget, int  stretch = 0);
   void addWidget(QStatusBar* theWrappedObject, QWidget*  widget, int  stretch = 0);
   QString  currentMessage(QStatusBar* theWrappedObject) const;
   bool  event(QStatusBar* theWrappedObject, QEvent*  arg__1);
   int  insertPermanentWidget(QStatusBar* theWrappedObject, int  index, QWidget*  widget, int  stretch = 0);
   int  insertWidget(QStatusBar* theWrappedObject, int  index, QWidget*  widget, int  stretch = 0);
   bool  isSizeGripEnabled(QStatusBar* theWrappedObject) const;
   void paintEvent(QStatusBar* theWrappedObject, QPaintEvent*  arg__1);
   void removeWidget(QStatusBar* theWrappedObject, QWidget*  widget);
   void resizeEvent(QStatusBar* theWrappedObject, QResizeEvent*  arg__1);
   void setSizeGripEnabled(QStatusBar* theWrappedObject, bool  arg__1);
   void showEvent(QStatusBar* theWrappedObject, QShowEvent*  arg__1);
};





class PythonQtWrapper_QStatusTipEvent : public QObject
{ Q_OBJECT
public:
public slots:
QStatusTipEvent* new_QStatusTipEvent(const QString&  tip);
void delete_QStatusTipEvent(QStatusTipEvent* obj) { delete obj; } 
   QString  tip(QStatusTipEvent* theWrappedObject) const;
};





class PythonQtShell_QStringListModel : public QStringListModel
{
public:
    PythonQtShell_QStringListModel(QObject*  parent = 0):QStringListModel(parent),_wrapper(NULL) {};
    PythonQtShell_QStringListModel(const QStringList&  strings, QObject*  parent = 0):QStringListModel(strings, parent),_wrapper(NULL) {};

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
virtual QModelIndex  index(int  row, int  column, const QModelIndex&  parent) const;
virtual bool  insertColumns(int  column, int  count, const QModelIndex&  parent);
virtual bool  insertRows(int  row, int  count, const QModelIndex&  parent = QModelIndex());
virtual QMap<int , QVariant >  itemData(const QModelIndex&  index) const;
virtual QList<QModelIndex >  match(const QModelIndex&  start, int  role, const QVariant&  value, int  hits, Qt::MatchFlags  flags) const;
virtual QMimeData*  mimeData(const QList<QModelIndex >&  indexes) const;
virtual QStringList  mimeTypes() const;
virtual bool  removeColumns(int  column, int  count, const QModelIndex&  parent);
virtual bool  removeRows(int  row, int  count, const QModelIndex&  parent = QModelIndex());
virtual void revert();
virtual int  rowCount(const QModelIndex&  parent = QModelIndex()) const;
virtual bool  setData(const QModelIndex&  index, const QVariant&  value, int  role = Qt::EditRole);
virtual bool  setHeaderData(int  section, Qt::Orientation  orientation, const QVariant&  value, int  role);
virtual bool  setItemData(const QModelIndex&  index, const QMap<int , QVariant >&  roles);
virtual void sort(int  column, Qt::SortOrder  order = Qt::AscendingOrder);
virtual QSize  span(const QModelIndex&  index) const;
virtual bool  submit();
virtual Qt::DropActions  supportedDropActions() const;
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QStringListModel : public QStringListModel
{ public:
inline QVariant  promoted_data(const QModelIndex&  index, int  role) const { return QStringListModel::data(index, role); }
inline Qt::ItemFlags  promoted_flags(const QModelIndex&  index) const { return QStringListModel::flags(index); }
inline bool  promoted_insertRows(int  row, int  count, const QModelIndex&  parent = QModelIndex()) { return QStringListModel::insertRows(row, count, parent); }
inline bool  promoted_removeRows(int  row, int  count, const QModelIndex&  parent = QModelIndex()) { return QStringListModel::removeRows(row, count, parent); }
inline int  promoted_rowCount(const QModelIndex&  parent = QModelIndex()) const { return QStringListModel::rowCount(parent); }
inline bool  promoted_setData(const QModelIndex&  index, const QVariant&  value, int  role = Qt::EditRole) { return QStringListModel::setData(index, value, role); }
inline void promoted_sort(int  column, Qt::SortOrder  order = Qt::AscendingOrder) { QStringListModel::sort(column, order); }
inline Qt::DropActions  promoted_supportedDropActions() const { return QStringListModel::supportedDropActions(); }
};

class PythonQtWrapper_QStringListModel : public QObject
{ Q_OBJECT
public:
public slots:
QStringListModel* new_QStringListModel(QObject*  parent = 0);
QStringListModel* new_QStringListModel(const QStringList&  strings, QObject*  parent = 0);
void delete_QStringListModel(QStringListModel* obj) { delete obj; } 
   QVariant  data(QStringListModel* theWrappedObject, const QModelIndex&  index, int  role) const;
   Qt::ItemFlags  flags(QStringListModel* theWrappedObject, const QModelIndex&  index) const;
   bool  insertRows(QStringListModel* theWrappedObject, int  row, int  count, const QModelIndex&  parent = QModelIndex());
   bool  removeRows(QStringListModel* theWrappedObject, int  row, int  count, const QModelIndex&  parent = QModelIndex());
   int  rowCount(QStringListModel* theWrappedObject, const QModelIndex&  parent = QModelIndex()) const;
   bool  setData(QStringListModel* theWrappedObject, const QModelIndex&  index, const QVariant&  value, int  role = Qt::EditRole);
   void setStringList(QStringListModel* theWrappedObject, const QStringList&  strings);
   void sort(QStringListModel* theWrappedObject, int  column, Qt::SortOrder  order = Qt::AscendingOrder);
   QStringList  stringList(QStringListModel* theWrappedObject) const;
   Qt::DropActions  supportedDropActions(QStringListModel* theWrappedObject) const;
};





class PythonQtShell_QStyle : public QStyle
{
public:
    PythonQtShell_QStyle():QStyle(),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual void drawComplexControl(QStyle::ComplexControl  cc, const QStyleOptionComplex*  opt, QPainter*  p, const QWidget*  widget = 0) const;
virtual void drawControl(QStyle::ControlElement  element, const QStyleOption*  opt, QPainter*  p, const QWidget*  w = 0) const;
virtual void drawItemPixmap(QPainter*  painter, const QRect&  rect, int  alignment, const QPixmap&  pixmap) const;
virtual void drawItemText(QPainter*  painter, const QRect&  rect, int  flags, const QPalette&  pal, bool  enabled, const QString&  text, QPalette::ColorRole  textRole = QPalette::NoRole) const;
virtual void drawPrimitive(QStyle::PrimitiveElement  pe, const QStyleOption*  opt, QPainter*  p, const QWidget*  w = 0) const;
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual QPixmap  generatedIconPixmap(QIcon::Mode  iconMode, const QPixmap&  pixmap, const QStyleOption*  opt) const;
virtual QStyle::SubControl  hitTestComplexControl(QStyle::ComplexControl  cc, const QStyleOptionComplex*  opt, const QPoint&  pt, const QWidget*  widget = 0) const;
virtual QRect  itemPixmapRect(const QRect&  r, int  flags, const QPixmap&  pixmap) const;
virtual QRect  itemTextRect(const QFontMetrics&  fm, const QRect&  r, int  flags, bool  enabled, const QString&  text) const;
virtual int  pixelMetric(QStyle::PixelMetric  metric, const QStyleOption*  option = 0, const QWidget*  widget = 0) const;
virtual void polish(QApplication*  arg__1);
virtual void polish(QPalette&  arg__1);
virtual void polish(QWidget*  arg__1);
virtual QSize  sizeFromContents(QStyle::ContentsType  ct, const QStyleOption*  opt, const QSize&  contentsSize, const QWidget*  w = 0) const;
virtual QPalette  standardPalette() const;
virtual QPixmap  standardPixmap(QStyle::StandardPixmap  standardPixmap, const QStyleOption*  opt = 0, const QWidget*  widget = 0) const;
virtual int  styleHint(QStyle::StyleHint  stylehint, const QStyleOption*  opt = 0, const QWidget*  widget = 0, QStyleHintReturn*  returnData = 0) const;
virtual QRect  subControlRect(QStyle::ComplexControl  cc, const QStyleOptionComplex*  opt, QStyle::SubControl  sc, const QWidget*  widget = 0) const;
virtual QRect  subElementRect(QStyle::SubElement  subElement, const QStyleOption*  option, const QWidget*  widget = 0) const;
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void unpolish(QApplication*  arg__1);
virtual void unpolish(QWidget*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QStyle : public QStyle
{ public:
inline void promoted_drawItemPixmap(QPainter*  painter, const QRect&  rect, int  alignment, const QPixmap&  pixmap) const { QStyle::drawItemPixmap(painter, rect, alignment, pixmap); }
inline void promoted_drawItemText(QPainter*  painter, const QRect&  rect, int  flags, const QPalette&  pal, bool  enabled, const QString&  text, QPalette::ColorRole  textRole = QPalette::NoRole) const { QStyle::drawItemText(painter, rect, flags, pal, enabled, text, textRole); }
inline QRect  promoted_itemPixmapRect(const QRect&  r, int  flags, const QPixmap&  pixmap) const { return QStyle::itemPixmapRect(r, flags, pixmap); }
inline void promoted_polish(QApplication*  arg__1) { QStyle::polish(arg__1); }
inline void promoted_polish(QPalette&  arg__1) { QStyle::polish(arg__1); }
inline void promoted_polish(QWidget*  arg__1) { QStyle::polish(arg__1); }
inline QPalette  promoted_standardPalette() const { return QStyle::standardPalette(); }
inline void promoted_unpolish(QApplication*  arg__1) { QStyle::unpolish(arg__1); }
inline void promoted_unpolish(QWidget*  arg__1) { QStyle::unpolish(arg__1); }
};

class PythonQtWrapper_QStyle : public QObject
{ Q_OBJECT
public:
Q_ENUMS(PixelMetric SubControl StandardPixmap StyleHint PrimitiveElement ControlElement ContentsType StateFlag ComplexControl RequestSoftwareInputPanel SubElement )
Q_FLAGS(SubControls State )
enum PixelMetric{
  PM_ButtonMargin = QStyle::PM_ButtonMargin,   PM_ButtonDefaultIndicator = QStyle::PM_ButtonDefaultIndicator,   PM_MenuButtonIndicator = QStyle::PM_MenuButtonIndicator,   PM_ButtonShiftHorizontal = QStyle::PM_ButtonShiftHorizontal,   PM_ButtonShiftVertical = QStyle::PM_ButtonShiftVertical,   PM_DefaultFrameWidth = QStyle::PM_DefaultFrameWidth,   PM_SpinBoxFrameWidth = QStyle::PM_SpinBoxFrameWidth,   PM_ComboBoxFrameWidth = QStyle::PM_ComboBoxFrameWidth,   PM_MaximumDragDistance = QStyle::PM_MaximumDragDistance,   PM_ScrollBarExtent = QStyle::PM_ScrollBarExtent,   PM_ScrollBarSliderMin = QStyle::PM_ScrollBarSliderMin,   PM_SliderThickness = QStyle::PM_SliderThickness,   PM_SliderControlThickness = QStyle::PM_SliderControlThickness,   PM_SliderLength = QStyle::PM_SliderLength,   PM_SliderTickmarkOffset = QStyle::PM_SliderTickmarkOffset,   PM_SliderSpaceAvailable = QStyle::PM_SliderSpaceAvailable,   PM_DockWidgetSeparatorExtent = QStyle::PM_DockWidgetSeparatorExtent,   PM_DockWidgetHandleExtent = QStyle::PM_DockWidgetHandleExtent,   PM_DockWidgetFrameWidth = QStyle::PM_DockWidgetFrameWidth,   PM_TabBarTabOverlap = QStyle::PM_TabBarTabOverlap,   PM_TabBarTabHSpace = QStyle::PM_TabBarTabHSpace,   PM_TabBarTabVSpace = QStyle::PM_TabBarTabVSpace,   PM_TabBarBaseHeight = QStyle::PM_TabBarBaseHeight,   PM_TabBarBaseOverlap = QStyle::PM_TabBarBaseOverlap,   PM_ProgressBarChunkWidth = QStyle::PM_ProgressBarChunkWidth,   PM_SplitterWidth = QStyle::PM_SplitterWidth,   PM_TitleBarHeight = QStyle::PM_TitleBarHeight,   PM_MenuScrollerHeight = QStyle::PM_MenuScrollerHeight,   PM_MenuHMargin = QStyle::PM_MenuHMargin,   PM_MenuVMargin = QStyle::PM_MenuVMargin,   PM_MenuPanelWidth = QStyle::PM_MenuPanelWidth,   PM_MenuTearoffHeight = QStyle::PM_MenuTearoffHeight,   PM_MenuDesktopFrameWidth = QStyle::PM_MenuDesktopFrameWidth,   PM_MenuBarPanelWidth = QStyle::PM_MenuBarPanelWidth,   PM_MenuBarItemSpacing = QStyle::PM_MenuBarItemSpacing,   PM_MenuBarVMargin = QStyle::PM_MenuBarVMargin,   PM_MenuBarHMargin = QStyle::PM_MenuBarHMargin,   PM_IndicatorWidth = QStyle::PM_IndicatorWidth,   PM_IndicatorHeight = QStyle::PM_IndicatorHeight,   PM_ExclusiveIndicatorWidth = QStyle::PM_ExclusiveIndicatorWidth,   PM_ExclusiveIndicatorHeight = QStyle::PM_ExclusiveIndicatorHeight,   PM_CheckListButtonSize = QStyle::PM_CheckListButtonSize,   PM_CheckListControllerSize = QStyle::PM_CheckListControllerSize,   PM_DialogButtonsSeparator = QStyle::PM_DialogButtonsSeparator,   PM_DialogButtonsButtonWidth = QStyle::PM_DialogButtonsButtonWidth,   PM_DialogButtonsButtonHeight = QStyle::PM_DialogButtonsButtonHeight,   PM_MdiSubWindowFrameWidth = QStyle::PM_MdiSubWindowFrameWidth,   PM_MDIFrameWidth = QStyle::PM_MDIFrameWidth,   PM_MdiSubWindowMinimizedWidth = QStyle::PM_MdiSubWindowMinimizedWidth,   PM_MDIMinimizedWidth = QStyle::PM_MDIMinimizedWidth,   PM_HeaderMargin = QStyle::PM_HeaderMargin,   PM_HeaderMarkSize = QStyle::PM_HeaderMarkSize,   PM_HeaderGripMargin = QStyle::PM_HeaderGripMargin,   PM_TabBarTabShiftHorizontal = QStyle::PM_TabBarTabShiftHorizontal,   PM_TabBarTabShiftVertical = QStyle::PM_TabBarTabShiftVertical,   PM_TabBarScrollButtonWidth = QStyle::PM_TabBarScrollButtonWidth,   PM_ToolBarFrameWidth = QStyle::PM_ToolBarFrameWidth,   PM_ToolBarHandleExtent = QStyle::PM_ToolBarHandleExtent,   PM_ToolBarItemSpacing = QStyle::PM_ToolBarItemSpacing,   PM_ToolBarItemMargin = QStyle::PM_ToolBarItemMargin,   PM_ToolBarSeparatorExtent = QStyle::PM_ToolBarSeparatorExtent,   PM_ToolBarExtensionExtent = QStyle::PM_ToolBarExtensionExtent,   PM_SpinBoxSliderHeight = QStyle::PM_SpinBoxSliderHeight,   PM_DefaultTopLevelMargin = QStyle::PM_DefaultTopLevelMargin,   PM_DefaultChildMargin = QStyle::PM_DefaultChildMargin,   PM_DefaultLayoutSpacing = QStyle::PM_DefaultLayoutSpacing,   PM_ToolBarIconSize = QStyle::PM_ToolBarIconSize,   PM_ListViewIconSize = QStyle::PM_ListViewIconSize,   PM_IconViewIconSize = QStyle::PM_IconViewIconSize,   PM_SmallIconSize = QStyle::PM_SmallIconSize,   PM_LargeIconSize = QStyle::PM_LargeIconSize,   PM_FocusFrameVMargin = QStyle::PM_FocusFrameVMargin,   PM_FocusFrameHMargin = QStyle::PM_FocusFrameHMargin,   PM_ToolTipLabelFrameWidth = QStyle::PM_ToolTipLabelFrameWidth,   PM_CheckBoxLabelSpacing = QStyle::PM_CheckBoxLabelSpacing,   PM_TabBarIconSize = QStyle::PM_TabBarIconSize,   PM_SizeGripSize = QStyle::PM_SizeGripSize,   PM_DockWidgetTitleMargin = QStyle::PM_DockWidgetTitleMargin,   PM_MessageBoxIconSize = QStyle::PM_MessageBoxIconSize,   PM_ButtonIconSize = QStyle::PM_ButtonIconSize,   PM_DockWidgetTitleBarButtonMargin = QStyle::PM_DockWidgetTitleBarButtonMargin,   PM_RadioButtonLabelSpacing = QStyle::PM_RadioButtonLabelSpacing,   PM_LayoutLeftMargin = QStyle::PM_LayoutLeftMargin,   PM_LayoutTopMargin = QStyle::PM_LayoutTopMargin,   PM_LayoutRightMargin = QStyle::PM_LayoutRightMargin,   PM_LayoutBottomMargin = QStyle::PM_LayoutBottomMargin,   PM_LayoutHorizontalSpacing = QStyle::PM_LayoutHorizontalSpacing,   PM_LayoutVerticalSpacing = QStyle::PM_LayoutVerticalSpacing,   PM_TabBar_ScrollButtonOverlap = QStyle::PM_TabBar_ScrollButtonOverlap,   PM_TextCursorWidth = QStyle::PM_TextCursorWidth,   PM_TabCloseIndicatorWidth = QStyle::PM_TabCloseIndicatorWidth,   PM_TabCloseIndicatorHeight = QStyle::PM_TabCloseIndicatorHeight,   PM_ScrollView_ScrollBarSpacing = QStyle::PM_ScrollView_ScrollBarSpacing,   PM_SubMenuOverlap = QStyle::PM_SubMenuOverlap,   PM_CustomBase = QStyle::PM_CustomBase};
enum SubControl{
  SC_None = QStyle::SC_None,   SC_ScrollBarAddLine = QStyle::SC_ScrollBarAddLine,   SC_ScrollBarSubLine = QStyle::SC_ScrollBarSubLine,   SC_ScrollBarAddPage = QStyle::SC_ScrollBarAddPage,   SC_ScrollBarSubPage = QStyle::SC_ScrollBarSubPage,   SC_ScrollBarFirst = QStyle::SC_ScrollBarFirst,   SC_ScrollBarLast = QStyle::SC_ScrollBarLast,   SC_ScrollBarSlider = QStyle::SC_ScrollBarSlider,   SC_ScrollBarGroove = QStyle::SC_ScrollBarGroove,   SC_SpinBoxUp = QStyle::SC_SpinBoxUp,   SC_SpinBoxDown = QStyle::SC_SpinBoxDown,   SC_SpinBoxFrame = QStyle::SC_SpinBoxFrame,   SC_SpinBoxEditField = QStyle::SC_SpinBoxEditField,   SC_ComboBoxFrame = QStyle::SC_ComboBoxFrame,   SC_ComboBoxEditField = QStyle::SC_ComboBoxEditField,   SC_ComboBoxArrow = QStyle::SC_ComboBoxArrow,   SC_ComboBoxListBoxPopup = QStyle::SC_ComboBoxListBoxPopup,   SC_SliderGroove = QStyle::SC_SliderGroove,   SC_SliderHandle = QStyle::SC_SliderHandle,   SC_SliderTickmarks = QStyle::SC_SliderTickmarks,   SC_ToolButton = QStyle::SC_ToolButton,   SC_ToolButtonMenu = QStyle::SC_ToolButtonMenu,   SC_TitleBarSysMenu = QStyle::SC_TitleBarSysMenu,   SC_TitleBarMinButton = QStyle::SC_TitleBarMinButton,   SC_TitleBarMaxButton = QStyle::SC_TitleBarMaxButton,   SC_TitleBarCloseButton = QStyle::SC_TitleBarCloseButton,   SC_TitleBarNormalButton = QStyle::SC_TitleBarNormalButton,   SC_TitleBarShadeButton = QStyle::SC_TitleBarShadeButton,   SC_TitleBarUnshadeButton = QStyle::SC_TitleBarUnshadeButton,   SC_TitleBarContextHelpButton = QStyle::SC_TitleBarContextHelpButton,   SC_TitleBarLabel = QStyle::SC_TitleBarLabel,   SC_Q3ListView = QStyle::SC_Q3ListView,   SC_Q3ListViewBranch = QStyle::SC_Q3ListViewBranch,   SC_Q3ListViewExpand = QStyle::SC_Q3ListViewExpand,   SC_DialGroove = QStyle::SC_DialGroove,   SC_DialHandle = QStyle::SC_DialHandle,   SC_DialTickmarks = QStyle::SC_DialTickmarks,   SC_GroupBoxCheckBox = QStyle::SC_GroupBoxCheckBox,   SC_GroupBoxLabel = QStyle::SC_GroupBoxLabel,   SC_GroupBoxContents = QStyle::SC_GroupBoxContents,   SC_GroupBoxFrame = QStyle::SC_GroupBoxFrame,   SC_MdiMinButton = QStyle::SC_MdiMinButton,   SC_MdiNormalButton = QStyle::SC_MdiNormalButton,   SC_MdiCloseButton = QStyle::SC_MdiCloseButton,   SC_CustomBase = QStyle::SC_CustomBase,   SC_All = QStyle::SC_All};
enum StandardPixmap{
  SP_TitleBarMenuButton = QStyle::SP_TitleBarMenuButton,   SP_TitleBarMinButton = QStyle::SP_TitleBarMinButton,   SP_TitleBarMaxButton = QStyle::SP_TitleBarMaxButton,   SP_TitleBarCloseButton = QStyle::SP_TitleBarCloseButton,   SP_TitleBarNormalButton = QStyle::SP_TitleBarNormalButton,   SP_TitleBarShadeButton = QStyle::SP_TitleBarShadeButton,   SP_TitleBarUnshadeButton = QStyle::SP_TitleBarUnshadeButton,   SP_TitleBarContextHelpButton = QStyle::SP_TitleBarContextHelpButton,   SP_DockWidgetCloseButton = QStyle::SP_DockWidgetCloseButton,   SP_MessageBoxInformation = QStyle::SP_MessageBoxInformation,   SP_MessageBoxWarning = QStyle::SP_MessageBoxWarning,   SP_MessageBoxCritical = QStyle::SP_MessageBoxCritical,   SP_MessageBoxQuestion = QStyle::SP_MessageBoxQuestion,   SP_DesktopIcon = QStyle::SP_DesktopIcon,   SP_TrashIcon = QStyle::SP_TrashIcon,   SP_ComputerIcon = QStyle::SP_ComputerIcon,   SP_DriveFDIcon = QStyle::SP_DriveFDIcon,   SP_DriveHDIcon = QStyle::SP_DriveHDIcon,   SP_DriveCDIcon = QStyle::SP_DriveCDIcon,   SP_DriveDVDIcon = QStyle::SP_DriveDVDIcon,   SP_DriveNetIcon = QStyle::SP_DriveNetIcon,   SP_DirOpenIcon = QStyle::SP_DirOpenIcon,   SP_DirClosedIcon = QStyle::SP_DirClosedIcon,   SP_DirLinkIcon = QStyle::SP_DirLinkIcon,   SP_FileIcon = QStyle::SP_FileIcon,   SP_FileLinkIcon = QStyle::SP_FileLinkIcon,   SP_ToolBarHorizontalExtensionButton = QStyle::SP_ToolBarHorizontalExtensionButton,   SP_ToolBarVerticalExtensionButton = QStyle::SP_ToolBarVerticalExtensionButton,   SP_FileDialogStart = QStyle::SP_FileDialogStart,   SP_FileDialogEnd = QStyle::SP_FileDialogEnd,   SP_FileDialogToParent = QStyle::SP_FileDialogToParent,   SP_FileDialogNewFolder = QStyle::SP_FileDialogNewFolder,   SP_FileDialogDetailedView = QStyle::SP_FileDialogDetailedView,   SP_FileDialogInfoView = QStyle::SP_FileDialogInfoView,   SP_FileDialogContentsView = QStyle::SP_FileDialogContentsView,   SP_FileDialogListView = QStyle::SP_FileDialogListView,   SP_FileDialogBack = QStyle::SP_FileDialogBack,   SP_DirIcon = QStyle::SP_DirIcon,   SP_DialogOkButton = QStyle::SP_DialogOkButton,   SP_DialogCancelButton = QStyle::SP_DialogCancelButton,   SP_DialogHelpButton = QStyle::SP_DialogHelpButton,   SP_DialogOpenButton = QStyle::SP_DialogOpenButton,   SP_DialogSaveButton = QStyle::SP_DialogSaveButton,   SP_DialogCloseButton = QStyle::SP_DialogCloseButton,   SP_DialogApplyButton = QStyle::SP_DialogApplyButton,   SP_DialogResetButton = QStyle::SP_DialogResetButton,   SP_DialogDiscardButton = QStyle::SP_DialogDiscardButton,   SP_DialogYesButton = QStyle::SP_DialogYesButton,   SP_DialogNoButton = QStyle::SP_DialogNoButton,   SP_ArrowUp = QStyle::SP_ArrowUp,   SP_ArrowDown = QStyle::SP_ArrowDown,   SP_ArrowLeft = QStyle::SP_ArrowLeft,   SP_ArrowRight = QStyle::SP_ArrowRight,   SP_ArrowBack = QStyle::SP_ArrowBack,   SP_ArrowForward = QStyle::SP_ArrowForward,   SP_DirHomeIcon = QStyle::SP_DirHomeIcon,   SP_CommandLink = QStyle::SP_CommandLink,   SP_VistaShield = QStyle::SP_VistaShield,   SP_BrowserReload = QStyle::SP_BrowserReload,   SP_BrowserStop = QStyle::SP_BrowserStop,   SP_MediaPlay = QStyle::SP_MediaPlay,   SP_MediaStop = QStyle::SP_MediaStop,   SP_MediaPause = QStyle::SP_MediaPause,   SP_MediaSkipForward = QStyle::SP_MediaSkipForward,   SP_MediaSkipBackward = QStyle::SP_MediaSkipBackward,   SP_MediaSeekForward = QStyle::SP_MediaSeekForward,   SP_MediaSeekBackward = QStyle::SP_MediaSeekBackward,   SP_MediaVolume = QStyle::SP_MediaVolume,   SP_MediaVolumeMuted = QStyle::SP_MediaVolumeMuted,   SP_CustomBase = QStyle::SP_CustomBase};
enum StyleHint{
  SH_EtchDisabledText = QStyle::SH_EtchDisabledText,   SH_DitherDisabledText = QStyle::SH_DitherDisabledText,   SH_ScrollBar_MiddleClickAbsolutePosition = QStyle::SH_ScrollBar_MiddleClickAbsolutePosition,   SH_ScrollBar_ScrollWhenPointerLeavesControl = QStyle::SH_ScrollBar_ScrollWhenPointerLeavesControl,   SH_TabBar_SelectMouseType = QStyle::SH_TabBar_SelectMouseType,   SH_TabBar_Alignment = QStyle::SH_TabBar_Alignment,   SH_Header_ArrowAlignment = QStyle::SH_Header_ArrowAlignment,   SH_Slider_SnapToValue = QStyle::SH_Slider_SnapToValue,   SH_Slider_SloppyKeyEvents = QStyle::SH_Slider_SloppyKeyEvents,   SH_ProgressDialog_CenterCancelButton = QStyle::SH_ProgressDialog_CenterCancelButton,   SH_ProgressDialog_TextLabelAlignment = QStyle::SH_ProgressDialog_TextLabelAlignment,   SH_PrintDialog_RightAlignButtons = QStyle::SH_PrintDialog_RightAlignButtons,   SH_MainWindow_SpaceBelowMenuBar = QStyle::SH_MainWindow_SpaceBelowMenuBar,   SH_FontDialog_SelectAssociatedText = QStyle::SH_FontDialog_SelectAssociatedText,   SH_Menu_AllowActiveAndDisabled = QStyle::SH_Menu_AllowActiveAndDisabled,   SH_Menu_SpaceActivatesItem = QStyle::SH_Menu_SpaceActivatesItem,   SH_Menu_SubMenuPopupDelay = QStyle::SH_Menu_SubMenuPopupDelay,   SH_ScrollView_FrameOnlyAroundContents = QStyle::SH_ScrollView_FrameOnlyAroundContents,   SH_MenuBar_AltKeyNavigation = QStyle::SH_MenuBar_AltKeyNavigation,   SH_ComboBox_ListMouseTracking = QStyle::SH_ComboBox_ListMouseTracking,   SH_Menu_MouseTracking = QStyle::SH_Menu_MouseTracking,   SH_MenuBar_MouseTracking = QStyle::SH_MenuBar_MouseTracking,   SH_ItemView_ChangeHighlightOnFocus = QStyle::SH_ItemView_ChangeHighlightOnFocus,   SH_Widget_ShareActivation = QStyle::SH_Widget_ShareActivation,   SH_Workspace_FillSpaceOnMaximize = QStyle::SH_Workspace_FillSpaceOnMaximize,   SH_ComboBox_Popup = QStyle::SH_ComboBox_Popup,   SH_TitleBar_NoBorder = QStyle::SH_TitleBar_NoBorder,   SH_Slider_StopMouseOverSlider = QStyle::SH_Slider_StopMouseOverSlider,   SH_ScrollBar_StopMouseOverSlider = QStyle::SH_ScrollBar_StopMouseOverSlider,   SH_BlinkCursorWhenTextSelected = QStyle::SH_BlinkCursorWhenTextSelected,   SH_RichText_FullWidthSelection = QStyle::SH_RichText_FullWidthSelection,   SH_Menu_Scrollable = QStyle::SH_Menu_Scrollable,   SH_GroupBox_TextLabelVerticalAlignment = QStyle::SH_GroupBox_TextLabelVerticalAlignment,   SH_GroupBox_TextLabelColor = QStyle::SH_GroupBox_TextLabelColor,   SH_Menu_SloppySubMenus = QStyle::SH_Menu_SloppySubMenus,   SH_Table_GridLineColor = QStyle::SH_Table_GridLineColor,   SH_LineEdit_PasswordCharacter = QStyle::SH_LineEdit_PasswordCharacter,   SH_DialogButtons_DefaultButton = QStyle::SH_DialogButtons_DefaultButton,   SH_ToolBox_SelectedPageTitleBold = QStyle::SH_ToolBox_SelectedPageTitleBold,   SH_TabBar_PreferNoArrows = QStyle::SH_TabBar_PreferNoArrows,   SH_ScrollBar_LeftClickAbsolutePosition = QStyle::SH_ScrollBar_LeftClickAbsolutePosition,   SH_Q3ListViewExpand_SelectMouseType = QStyle::SH_Q3ListViewExpand_SelectMouseType,   SH_UnderlineShortcut = QStyle::SH_UnderlineShortcut,   SH_SpinBox_AnimateButton = QStyle::SH_SpinBox_AnimateButton,   SH_SpinBox_KeyPressAutoRepeatRate = QStyle::SH_SpinBox_KeyPressAutoRepeatRate,   SH_SpinBox_ClickAutoRepeatRate = QStyle::SH_SpinBox_ClickAutoRepeatRate,   SH_Menu_FillScreenWithScroll = QStyle::SH_Menu_FillScreenWithScroll,   SH_ToolTipLabel_Opacity = QStyle::SH_ToolTipLabel_Opacity,   SH_DrawMenuBarSeparator = QStyle::SH_DrawMenuBarSeparator,   SH_TitleBar_ModifyNotification = QStyle::SH_TitleBar_ModifyNotification,   SH_Button_FocusPolicy = QStyle::SH_Button_FocusPolicy,   SH_MenuBar_DismissOnSecondClick = QStyle::SH_MenuBar_DismissOnSecondClick,   SH_MessageBox_UseBorderForButtonSpacing = QStyle::SH_MessageBox_UseBorderForButtonSpacing,   SH_TitleBar_AutoRaise = QStyle::SH_TitleBar_AutoRaise,   SH_ToolButton_PopupDelay = QStyle::SH_ToolButton_PopupDelay,   SH_FocusFrame_Mask = QStyle::SH_FocusFrame_Mask,   SH_RubberBand_Mask = QStyle::SH_RubberBand_Mask,   SH_WindowFrame_Mask = QStyle::SH_WindowFrame_Mask,   SH_SpinControls_DisableOnBounds = QStyle::SH_SpinControls_DisableOnBounds,   SH_Dial_BackgroundRole = QStyle::SH_Dial_BackgroundRole,   SH_ComboBox_LayoutDirection = QStyle::SH_ComboBox_LayoutDirection,   SH_ItemView_EllipsisLocation = QStyle::SH_ItemView_EllipsisLocation,   SH_ItemView_ShowDecorationSelected = QStyle::SH_ItemView_ShowDecorationSelected,   SH_ItemView_ActivateItemOnSingleClick = QStyle::SH_ItemView_ActivateItemOnSingleClick,   SH_ScrollBar_ContextMenu = QStyle::SH_ScrollBar_ContextMenu,   SH_ScrollBar_RollBetweenButtons = QStyle::SH_ScrollBar_RollBetweenButtons,   SH_Slider_AbsoluteSetButtons = QStyle::SH_Slider_AbsoluteSetButtons,   SH_Slider_PageSetButtons = QStyle::SH_Slider_PageSetButtons,   SH_Menu_KeyboardSearch = QStyle::SH_Menu_KeyboardSearch,   SH_TabBar_ElideMode = QStyle::SH_TabBar_ElideMode,   SH_DialogButtonLayout = QStyle::SH_DialogButtonLayout,   SH_ComboBox_PopupFrameStyle = QStyle::SH_ComboBox_PopupFrameStyle,   SH_MessageBox_TextInteractionFlags = QStyle::SH_MessageBox_TextInteractionFlags,   SH_DialogButtonBox_ButtonsHaveIcons = QStyle::SH_DialogButtonBox_ButtonsHaveIcons,   SH_SpellCheckUnderlineStyle = QStyle::SH_SpellCheckUnderlineStyle,   SH_MessageBox_CenterButtons = QStyle::SH_MessageBox_CenterButtons,   SH_Menu_SelectionWrap = QStyle::SH_Menu_SelectionWrap,   SH_ItemView_MovementWithoutUpdatingSelection = QStyle::SH_ItemView_MovementWithoutUpdatingSelection,   SH_ToolTip_Mask = QStyle::SH_ToolTip_Mask,   SH_FocusFrame_AboveWidget = QStyle::SH_FocusFrame_AboveWidget,   SH_TextControl_FocusIndicatorTextCharFormat = QStyle::SH_TextControl_FocusIndicatorTextCharFormat,   SH_WizardStyle = QStyle::SH_WizardStyle,   SH_ItemView_ArrowKeysNavigateIntoChildren = QStyle::SH_ItemView_ArrowKeysNavigateIntoChildren,   SH_Menu_Mask = QStyle::SH_Menu_Mask,   SH_Menu_FlashTriggeredItem = QStyle::SH_Menu_FlashTriggeredItem,   SH_Menu_FadeOutOnHide = QStyle::SH_Menu_FadeOutOnHide,   SH_SpinBox_ClickAutoRepeatThreshold = QStyle::SH_SpinBox_ClickAutoRepeatThreshold,   SH_ItemView_PaintAlternatingRowColorsForEmptyArea = QStyle::SH_ItemView_PaintAlternatingRowColorsForEmptyArea,   SH_FormLayoutWrapPolicy = QStyle::SH_FormLayoutWrapPolicy,   SH_TabWidget_DefaultTabPosition = QStyle::SH_TabWidget_DefaultTabPosition,   SH_ToolBar_Movable = QStyle::SH_ToolBar_Movable,   SH_FormLayoutFieldGrowthPolicy = QStyle::SH_FormLayoutFieldGrowthPolicy,   SH_FormLayoutFormAlignment = QStyle::SH_FormLayoutFormAlignment,   SH_FormLayoutLabelAlignment = QStyle::SH_FormLayoutLabelAlignment,   SH_ItemView_DrawDelegateFrame = QStyle::SH_ItemView_DrawDelegateFrame,   SH_TabBar_CloseButtonPosition = QStyle::SH_TabBar_CloseButtonPosition,   SH_DockWidget_ButtonsHaveFrame = QStyle::SH_DockWidget_ButtonsHaveFrame,   SH_ToolButtonStyle = QStyle::SH_ToolButtonStyle,   SH_RequestSoftwareInputPanel = QStyle::SH_RequestSoftwareInputPanel,   SH_CustomBase = QStyle::SH_CustomBase};
enum PrimitiveElement{
  PE_Q3CheckListController = QStyle::PE_Q3CheckListController,   PE_Q3CheckListExclusiveIndicator = QStyle::PE_Q3CheckListExclusiveIndicator,   PE_Q3CheckListIndicator = QStyle::PE_Q3CheckListIndicator,   PE_Q3DockWindowSeparator = QStyle::PE_Q3DockWindowSeparator,   PE_Q3Separator = QStyle::PE_Q3Separator,   PE_Frame = QStyle::PE_Frame,   PE_FrameDefaultButton = QStyle::PE_FrameDefaultButton,   PE_FrameDockWidget = QStyle::PE_FrameDockWidget,   PE_FrameFocusRect = QStyle::PE_FrameFocusRect,   PE_FrameGroupBox = QStyle::PE_FrameGroupBox,   PE_FrameLineEdit = QStyle::PE_FrameLineEdit,   PE_FrameMenu = QStyle::PE_FrameMenu,   PE_FrameStatusBar = QStyle::PE_FrameStatusBar,   PE_FrameStatusBarItem = QStyle::PE_FrameStatusBarItem,   PE_FrameTabWidget = QStyle::PE_FrameTabWidget,   PE_FrameWindow = QStyle::PE_FrameWindow,   PE_FrameButtonBevel = QStyle::PE_FrameButtonBevel,   PE_FrameButtonTool = QStyle::PE_FrameButtonTool,   PE_FrameTabBarBase = QStyle::PE_FrameTabBarBase,   PE_PanelButtonCommand = QStyle::PE_PanelButtonCommand,   PE_PanelButtonBevel = QStyle::PE_PanelButtonBevel,   PE_PanelButtonTool = QStyle::PE_PanelButtonTool,   PE_PanelMenuBar = QStyle::PE_PanelMenuBar,   PE_PanelToolBar = QStyle::PE_PanelToolBar,   PE_PanelLineEdit = QStyle::PE_PanelLineEdit,   PE_IndicatorArrowDown = QStyle::PE_IndicatorArrowDown,   PE_IndicatorArrowLeft = QStyle::PE_IndicatorArrowLeft,   PE_IndicatorArrowRight = QStyle::PE_IndicatorArrowRight,   PE_IndicatorArrowUp = QStyle::PE_IndicatorArrowUp,   PE_IndicatorBranch = QStyle::PE_IndicatorBranch,   PE_IndicatorButtonDropDown = QStyle::PE_IndicatorButtonDropDown,   PE_IndicatorViewItemCheck = QStyle::PE_IndicatorViewItemCheck,   PE_IndicatorItemViewItemCheck = QStyle::PE_IndicatorItemViewItemCheck,   PE_IndicatorCheckBox = QStyle::PE_IndicatorCheckBox,   PE_IndicatorDockWidgetResizeHandle = QStyle::PE_IndicatorDockWidgetResizeHandle,   PE_IndicatorHeaderArrow = QStyle::PE_IndicatorHeaderArrow,   PE_IndicatorMenuCheckMark = QStyle::PE_IndicatorMenuCheckMark,   PE_IndicatorProgressChunk = QStyle::PE_IndicatorProgressChunk,   PE_IndicatorRadioButton = QStyle::PE_IndicatorRadioButton,   PE_IndicatorSpinDown = QStyle::PE_IndicatorSpinDown,   PE_IndicatorSpinMinus = QStyle::PE_IndicatorSpinMinus,   PE_IndicatorSpinPlus = QStyle::PE_IndicatorSpinPlus,   PE_IndicatorSpinUp = QStyle::PE_IndicatorSpinUp,   PE_IndicatorToolBarHandle = QStyle::PE_IndicatorToolBarHandle,   PE_IndicatorToolBarSeparator = QStyle::PE_IndicatorToolBarSeparator,   PE_PanelTipLabel = QStyle::PE_PanelTipLabel,   PE_IndicatorTabTear = QStyle::PE_IndicatorTabTear,   PE_PanelScrollAreaCorner = QStyle::PE_PanelScrollAreaCorner,   PE_Widget = QStyle::PE_Widget,   PE_IndicatorColumnViewArrow = QStyle::PE_IndicatorColumnViewArrow,   PE_IndicatorItemViewItemDrop = QStyle::PE_IndicatorItemViewItemDrop,   PE_PanelItemViewItem = QStyle::PE_PanelItemViewItem,   PE_PanelItemViewRow = QStyle::PE_PanelItemViewRow,   PE_PanelStatusBar = QStyle::PE_PanelStatusBar,   PE_IndicatorTabClose = QStyle::PE_IndicatorTabClose,   PE_PanelMenu = QStyle::PE_PanelMenu,   PE_CustomBase = QStyle::PE_CustomBase};
enum ControlElement{
  CE_PushButton = QStyle::CE_PushButton,   CE_PushButtonBevel = QStyle::CE_PushButtonBevel,   CE_PushButtonLabel = QStyle::CE_PushButtonLabel,   CE_CheckBox = QStyle::CE_CheckBox,   CE_CheckBoxLabel = QStyle::CE_CheckBoxLabel,   CE_RadioButton = QStyle::CE_RadioButton,   CE_RadioButtonLabel = QStyle::CE_RadioButtonLabel,   CE_TabBarTab = QStyle::CE_TabBarTab,   CE_TabBarTabShape = QStyle::CE_TabBarTabShape,   CE_TabBarTabLabel = QStyle::CE_TabBarTabLabel,   CE_ProgressBar = QStyle::CE_ProgressBar,   CE_ProgressBarGroove = QStyle::CE_ProgressBarGroove,   CE_ProgressBarContents = QStyle::CE_ProgressBarContents,   CE_ProgressBarLabel = QStyle::CE_ProgressBarLabel,   CE_MenuItem = QStyle::CE_MenuItem,   CE_MenuScroller = QStyle::CE_MenuScroller,   CE_MenuVMargin = QStyle::CE_MenuVMargin,   CE_MenuHMargin = QStyle::CE_MenuHMargin,   CE_MenuTearoff = QStyle::CE_MenuTearoff,   CE_MenuEmptyArea = QStyle::CE_MenuEmptyArea,   CE_MenuBarItem = QStyle::CE_MenuBarItem,   CE_MenuBarEmptyArea = QStyle::CE_MenuBarEmptyArea,   CE_ToolButtonLabel = QStyle::CE_ToolButtonLabel,   CE_Header = QStyle::CE_Header,   CE_HeaderSection = QStyle::CE_HeaderSection,   CE_HeaderLabel = QStyle::CE_HeaderLabel,   CE_Q3DockWindowEmptyArea = QStyle::CE_Q3DockWindowEmptyArea,   CE_ToolBoxTab = QStyle::CE_ToolBoxTab,   CE_SizeGrip = QStyle::CE_SizeGrip,   CE_Splitter = QStyle::CE_Splitter,   CE_RubberBand = QStyle::CE_RubberBand,   CE_DockWidgetTitle = QStyle::CE_DockWidgetTitle,   CE_ScrollBarAddLine = QStyle::CE_ScrollBarAddLine,   CE_ScrollBarSubLine = QStyle::CE_ScrollBarSubLine,   CE_ScrollBarAddPage = QStyle::CE_ScrollBarAddPage,   CE_ScrollBarSubPage = QStyle::CE_ScrollBarSubPage,   CE_ScrollBarSlider = QStyle::CE_ScrollBarSlider,   CE_ScrollBarFirst = QStyle::CE_ScrollBarFirst,   CE_ScrollBarLast = QStyle::CE_ScrollBarLast,   CE_FocusFrame = QStyle::CE_FocusFrame,   CE_ComboBoxLabel = QStyle::CE_ComboBoxLabel,   CE_ToolBar = QStyle::CE_ToolBar,   CE_ToolBoxTabShape = QStyle::CE_ToolBoxTabShape,   CE_ToolBoxTabLabel = QStyle::CE_ToolBoxTabLabel,   CE_HeaderEmptyArea = QStyle::CE_HeaderEmptyArea,   CE_ColumnViewGrip = QStyle::CE_ColumnViewGrip,   CE_ItemViewItem = QStyle::CE_ItemViewItem,   CE_ShapedFrame = QStyle::CE_ShapedFrame,   CE_CustomBase = QStyle::CE_CustomBase};
enum ContentsType{
  CT_PushButton = QStyle::CT_PushButton,   CT_CheckBox = QStyle::CT_CheckBox,   CT_RadioButton = QStyle::CT_RadioButton,   CT_ToolButton = QStyle::CT_ToolButton,   CT_ComboBox = QStyle::CT_ComboBox,   CT_Splitter = QStyle::CT_Splitter,   CT_Q3DockWindow = QStyle::CT_Q3DockWindow,   CT_ProgressBar = QStyle::CT_ProgressBar,   CT_MenuItem = QStyle::CT_MenuItem,   CT_MenuBarItem = QStyle::CT_MenuBarItem,   CT_MenuBar = QStyle::CT_MenuBar,   CT_Menu = QStyle::CT_Menu,   CT_TabBarTab = QStyle::CT_TabBarTab,   CT_Slider = QStyle::CT_Slider,   CT_ScrollBar = QStyle::CT_ScrollBar,   CT_Q3Header = QStyle::CT_Q3Header,   CT_LineEdit = QStyle::CT_LineEdit,   CT_SpinBox = QStyle::CT_SpinBox,   CT_SizeGrip = QStyle::CT_SizeGrip,   CT_TabWidget = QStyle::CT_TabWidget,   CT_DialogButtons = QStyle::CT_DialogButtons,   CT_HeaderSection = QStyle::CT_HeaderSection,   CT_GroupBox = QStyle::CT_GroupBox,   CT_MdiControls = QStyle::CT_MdiControls,   CT_ItemViewItem = QStyle::CT_ItemViewItem,   CT_CustomBase = QStyle::CT_CustomBase};
enum StateFlag{
  State_None = QStyle::State_None,   State_Enabled = QStyle::State_Enabled,   State_Raised = QStyle::State_Raised,   State_Sunken = QStyle::State_Sunken,   State_Off = QStyle::State_Off,   State_NoChange = QStyle::State_NoChange,   State_On = QStyle::State_On,   State_DownArrow = QStyle::State_DownArrow,   State_Horizontal = QStyle::State_Horizontal,   State_HasFocus = QStyle::State_HasFocus,   State_Top = QStyle::State_Top,   State_Bottom = QStyle::State_Bottom,   State_FocusAtBorder = QStyle::State_FocusAtBorder,   State_AutoRaise = QStyle::State_AutoRaise,   State_MouseOver = QStyle::State_MouseOver,   State_UpArrow = QStyle::State_UpArrow,   State_Selected = QStyle::State_Selected,   State_Active = QStyle::State_Active,   State_Window = QStyle::State_Window,   State_Open = QStyle::State_Open,   State_Children = QStyle::State_Children,   State_Item = QStyle::State_Item,   State_Sibling = QStyle::State_Sibling,   State_Editing = QStyle::State_Editing,   State_KeyboardFocusChange = QStyle::State_KeyboardFocusChange,   State_ReadOnly = QStyle::State_ReadOnly,   State_Small = QStyle::State_Small,   State_Mini = QStyle::State_Mini};
enum ComplexControl{
  CC_SpinBox = QStyle::CC_SpinBox,   CC_ComboBox = QStyle::CC_ComboBox,   CC_ScrollBar = QStyle::CC_ScrollBar,   CC_Slider = QStyle::CC_Slider,   CC_ToolButton = QStyle::CC_ToolButton,   CC_TitleBar = QStyle::CC_TitleBar,   CC_Q3ListView = QStyle::CC_Q3ListView,   CC_Dial = QStyle::CC_Dial,   CC_GroupBox = QStyle::CC_GroupBox,   CC_MdiControls = QStyle::CC_MdiControls,   CC_CustomBase = QStyle::CC_CustomBase};
enum RequestSoftwareInputPanel{
  RSIP_OnMouseClickAndAlreadyFocused = QStyle::RSIP_OnMouseClickAndAlreadyFocused,   RSIP_OnMouseClick = QStyle::RSIP_OnMouseClick};
enum SubElement{
  SE_PushButtonContents = QStyle::SE_PushButtonContents,   SE_PushButtonFocusRect = QStyle::SE_PushButtonFocusRect,   SE_CheckBoxIndicator = QStyle::SE_CheckBoxIndicator,   SE_CheckBoxContents = QStyle::SE_CheckBoxContents,   SE_CheckBoxFocusRect = QStyle::SE_CheckBoxFocusRect,   SE_CheckBoxClickRect = QStyle::SE_CheckBoxClickRect,   SE_RadioButtonIndicator = QStyle::SE_RadioButtonIndicator,   SE_RadioButtonContents = QStyle::SE_RadioButtonContents,   SE_RadioButtonFocusRect = QStyle::SE_RadioButtonFocusRect,   SE_RadioButtonClickRect = QStyle::SE_RadioButtonClickRect,   SE_ComboBoxFocusRect = QStyle::SE_ComboBoxFocusRect,   SE_SliderFocusRect = QStyle::SE_SliderFocusRect,   SE_Q3DockWindowHandleRect = QStyle::SE_Q3DockWindowHandleRect,   SE_ProgressBarGroove = QStyle::SE_ProgressBarGroove,   SE_ProgressBarContents = QStyle::SE_ProgressBarContents,   SE_ProgressBarLabel = QStyle::SE_ProgressBarLabel,   SE_DialogButtonAccept = QStyle::SE_DialogButtonAccept,   SE_DialogButtonReject = QStyle::SE_DialogButtonReject,   SE_DialogButtonApply = QStyle::SE_DialogButtonApply,   SE_DialogButtonHelp = QStyle::SE_DialogButtonHelp,   SE_DialogButtonAll = QStyle::SE_DialogButtonAll,   SE_DialogButtonAbort = QStyle::SE_DialogButtonAbort,   SE_DialogButtonIgnore = QStyle::SE_DialogButtonIgnore,   SE_DialogButtonRetry = QStyle::SE_DialogButtonRetry,   SE_DialogButtonCustom = QStyle::SE_DialogButtonCustom,   SE_ToolBoxTabContents = QStyle::SE_ToolBoxTabContents,   SE_HeaderLabel = QStyle::SE_HeaderLabel,   SE_HeaderArrow = QStyle::SE_HeaderArrow,   SE_TabWidgetTabBar = QStyle::SE_TabWidgetTabBar,   SE_TabWidgetTabPane = QStyle::SE_TabWidgetTabPane,   SE_TabWidgetTabContents = QStyle::SE_TabWidgetTabContents,   SE_TabWidgetLeftCorner = QStyle::SE_TabWidgetLeftCorner,   SE_TabWidgetRightCorner = QStyle::SE_TabWidgetRightCorner,   SE_ViewItemCheckIndicator = QStyle::SE_ViewItemCheckIndicator,   SE_ItemViewItemCheckIndicator = QStyle::SE_ItemViewItemCheckIndicator,   SE_TabBarTearIndicator = QStyle::SE_TabBarTearIndicator,   SE_TreeViewDisclosureItem = QStyle::SE_TreeViewDisclosureItem,   SE_LineEditContents = QStyle::SE_LineEditContents,   SE_FrameContents = QStyle::SE_FrameContents,   SE_DockWidgetCloseButton = QStyle::SE_DockWidgetCloseButton,   SE_DockWidgetFloatButton = QStyle::SE_DockWidgetFloatButton,   SE_DockWidgetTitleBarText = QStyle::SE_DockWidgetTitleBarText,   SE_DockWidgetIcon = QStyle::SE_DockWidgetIcon,   SE_CheckBoxLayoutItem = QStyle::SE_CheckBoxLayoutItem,   SE_ComboBoxLayoutItem = QStyle::SE_ComboBoxLayoutItem,   SE_DateTimeEditLayoutItem = QStyle::SE_DateTimeEditLayoutItem,   SE_DialogButtonBoxLayoutItem = QStyle::SE_DialogButtonBoxLayoutItem,   SE_LabelLayoutItem = QStyle::SE_LabelLayoutItem,   SE_ProgressBarLayoutItem = QStyle::SE_ProgressBarLayoutItem,   SE_PushButtonLayoutItem = QStyle::SE_PushButtonLayoutItem,   SE_RadioButtonLayoutItem = QStyle::SE_RadioButtonLayoutItem,   SE_SliderLayoutItem = QStyle::SE_SliderLayoutItem,   SE_SpinBoxLayoutItem = QStyle::SE_SpinBoxLayoutItem,   SE_ToolButtonLayoutItem = QStyle::SE_ToolButtonLayoutItem,   SE_FrameLayoutItem = QStyle::SE_FrameLayoutItem,   SE_GroupBoxLayoutItem = QStyle::SE_GroupBoxLayoutItem,   SE_TabWidgetLayoutItem = QStyle::SE_TabWidgetLayoutItem,   SE_ItemViewItemDecoration = QStyle::SE_ItemViewItemDecoration,   SE_ItemViewItemText = QStyle::SE_ItemViewItemText,   SE_ItemViewItemFocusRect = QStyle::SE_ItemViewItemFocusRect,   SE_TabBarTabLeftButton = QStyle::SE_TabBarTabLeftButton,   SE_TabBarTabRightButton = QStyle::SE_TabBarTabRightButton,   SE_TabBarTabText = QStyle::SE_TabBarTabText,   SE_ShapedFrameContents = QStyle::SE_ShapedFrameContents,   SE_ToolBarHandle = QStyle::SE_ToolBarHandle,   SE_CustomBase = QStyle::SE_CustomBase};
Q_DECLARE_FLAGS(SubControls, SubControl)
Q_DECLARE_FLAGS(State, StateFlag)
public slots:
QStyle* new_QStyle();
void delete_QStyle(QStyle* obj) { delete obj; } 
   QRect  static_QStyle_alignedRect(Qt::LayoutDirection  direction, Qt::Alignment  alignment, const QSize&  size, const QRect&  rectangle);
   int  combinedLayoutSpacing(QStyle* theWrappedObject, QSizePolicy::ControlTypes  controls1, QSizePolicy::ControlTypes  controls2, Qt::Orientation  orientation, QStyleOption*  option = 0, QWidget*  widget = 0) const;
   void drawItemPixmap(QStyle* theWrappedObject, QPainter*  painter, const QRect&  rect, int  alignment, const QPixmap&  pixmap) const;
   void drawItemText(QStyle* theWrappedObject, QPainter*  painter, const QRect&  rect, int  flags, const QPalette&  pal, bool  enabled, const QString&  text, QPalette::ColorRole  textRole = QPalette::NoRole) const;
   QRect  itemPixmapRect(QStyle* theWrappedObject, const QRect&  r, int  flags, const QPixmap&  pixmap) const;
   int  layoutSpacing(QStyle* theWrappedObject, QSizePolicy::ControlType  control1, QSizePolicy::ControlType  control2, Qt::Orientation  orientation, const QStyleOption*  option = 0, const QWidget*  widget = 0) const;
   void polish(QStyle* theWrappedObject, QApplication*  arg__1);
   void polish(QStyle* theWrappedObject, QPalette&  arg__1);
   void polish(QStyle* theWrappedObject, QWidget*  arg__1);
   const QStyle*  proxy(QStyle* theWrappedObject) const;
   int  static_QStyle_sliderPositionFromValue(int  min, int  max, int  val, int  space, bool  upsideDown = false);
   int  static_QStyle_sliderValueFromPosition(int  min, int  max, int  pos, int  space, bool  upsideDown = false);
   QIcon  standardIcon(QStyle* theWrappedObject, QStyle::StandardPixmap  standardIcon, const QStyleOption*  option = 0, const QWidget*  widget = 0) const;
   QPalette  standardPalette(QStyle* theWrappedObject) const;
   void unpolish(QStyle* theWrappedObject, QApplication*  arg__1);
   void unpolish(QStyle* theWrappedObject, QWidget*  arg__1);
   Qt::Alignment  static_QStyle_visualAlignment(Qt::LayoutDirection  direction, Qt::Alignment  alignment);
   QPoint  static_QStyle_visualPos(Qt::LayoutDirection  direction, const QRect&  boundingRect, const QPoint&  logicalPos);
   QRect  static_QStyle_visualRect(Qt::LayoutDirection  direction, const QRect&  boundingRect, const QRect&  logicalRect);
};





class PythonQtShell_QStyleFactory : public QStyleFactory
{
public:
    PythonQtShell_QStyleFactory():QStyleFactory(),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleFactory : public QObject
{ Q_OBJECT
public:
public slots:
QStyleFactory* new_QStyleFactory();
void delete_QStyleFactory(QStyleFactory* obj) { delete obj; } 
   QStyle*  static_QStyleFactory_create(const QString&  arg__1);
   QStringList  static_QStyleFactory_keys();
};





class PythonQtShell_QStyleHintReturn : public QStyleHintReturn
{
public:
    PythonQtShell_QStyleHintReturn(int  version = QStyleOption::Version, int  type = SH_Default):QStyleHintReturn(version, type),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleHintReturn : public QObject
{ Q_OBJECT
public:
Q_ENUMS(HintReturnType StyleOptionVersion StyleOptionType )
enum HintReturnType{
  SH_Default = QStyleHintReturn::SH_Default,   SH_Mask = QStyleHintReturn::SH_Mask,   SH_Variant = QStyleHintReturn::SH_Variant};
enum StyleOptionVersion{
  Version = QStyleHintReturn::Version};
enum StyleOptionType{
  Type = QStyleHintReturn::Type};
public slots:
QStyleHintReturn* new_QStyleHintReturn(int  version = QStyleOption::Version, int  type = SH_Default);
void delete_QStyleHintReturn(QStyleHintReturn* obj) { delete obj; } 
void py_set_version(QStyleHintReturn* theWrappedObject, int  version){ theWrappedObject->version = version; }
int  py_get_version(QStyleHintReturn* theWrappedObject){ return theWrappedObject->version; }
void py_set_type(QStyleHintReturn* theWrappedObject, int  type){ theWrappedObject->type = type; }
int  py_get_type(QStyleHintReturn* theWrappedObject){ return theWrappedObject->type; }
};





class PythonQtShell_QStyleHintReturnMask : public QStyleHintReturnMask
{
public:
    PythonQtShell_QStyleHintReturnMask():QStyleHintReturnMask(),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleHintReturnMask : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion StyleOptionType )
enum StyleOptionVersion{
  Version = QStyleHintReturnMask::Version};
enum StyleOptionType{
  Type = QStyleHintReturnMask::Type};
public slots:
QStyleHintReturnMask* new_QStyleHintReturnMask();
void delete_QStyleHintReturnMask(QStyleHintReturnMask* obj) { delete obj; } 
void py_set_region(QStyleHintReturnMask* theWrappedObject, QRegion  region){ theWrappedObject->region = region; }
QRegion  py_get_region(QStyleHintReturnMask* theWrappedObject){ return theWrappedObject->region; }
};





class PythonQtShell_QStyleHintReturnVariant : public QStyleHintReturnVariant
{
public:
    PythonQtShell_QStyleHintReturnVariant():QStyleHintReturnVariant(),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleHintReturnVariant : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion StyleOptionType )
enum StyleOptionVersion{
  Version = QStyleHintReturnVariant::Version};
enum StyleOptionType{
  Type = QStyleHintReturnVariant::Type};
public slots:
QStyleHintReturnVariant* new_QStyleHintReturnVariant();
void delete_QStyleHintReturnVariant(QStyleHintReturnVariant* obj) { delete obj; } 
void py_set_variant(QStyleHintReturnVariant* theWrappedObject, QVariant  variant){ theWrappedObject->variant = variant; }
QVariant  py_get_variant(QStyleHintReturnVariant* theWrappedObject){ return theWrappedObject->variant; }
};





class PythonQtShell_QStyleOption : public QStyleOption
{
public:
    PythonQtShell_QStyleOption(const QStyleOption&  other):QStyleOption(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOption(int  version = QStyleOption::Version, int  type = SO_Default):QStyleOption(version, type),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOption : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion StyleOptionType OptionType )
enum StyleOptionVersion{
  Version = QStyleOption::Version};
enum StyleOptionType{
  Type = QStyleOption::Type};
enum OptionType{
  SO_Default = QStyleOption::SO_Default,   SO_FocusRect = QStyleOption::SO_FocusRect,   SO_Button = QStyleOption::SO_Button,   SO_Tab = QStyleOption::SO_Tab,   SO_MenuItem = QStyleOption::SO_MenuItem,   SO_Frame = QStyleOption::SO_Frame,   SO_ProgressBar = QStyleOption::SO_ProgressBar,   SO_ToolBox = QStyleOption::SO_ToolBox,   SO_Header = QStyleOption::SO_Header,   SO_Q3DockWindow = QStyleOption::SO_Q3DockWindow,   SO_DockWidget = QStyleOption::SO_DockWidget,   SO_Q3ListViewItem = QStyleOption::SO_Q3ListViewItem,   SO_ViewItem = QStyleOption::SO_ViewItem,   SO_TabWidgetFrame = QStyleOption::SO_TabWidgetFrame,   SO_TabBarBase = QStyleOption::SO_TabBarBase,   SO_RubberBand = QStyleOption::SO_RubberBand,   SO_ToolBar = QStyleOption::SO_ToolBar,   SO_GraphicsItem = QStyleOption::SO_GraphicsItem,   SO_Complex = QStyleOption::SO_Complex,   SO_Slider = QStyleOption::SO_Slider,   SO_SpinBox = QStyleOption::SO_SpinBox,   SO_ToolButton = QStyleOption::SO_ToolButton,   SO_ComboBox = QStyleOption::SO_ComboBox,   SO_Q3ListView = QStyleOption::SO_Q3ListView,   SO_TitleBar = QStyleOption::SO_TitleBar,   SO_GroupBox = QStyleOption::SO_GroupBox,   SO_SizeGrip = QStyleOption::SO_SizeGrip,   SO_CustomBase = QStyleOption::SO_CustomBase,   SO_ComplexCustomBase = QStyleOption::SO_ComplexCustomBase};
public slots:
QStyleOption* new_QStyleOption(const QStyleOption&  other);
QStyleOption* new_QStyleOption(int  version = QStyleOption::Version, int  type = SO_Default);
void delete_QStyleOption(QStyleOption* obj) { delete obj; } 
   void initFrom(QStyleOption* theWrappedObject, const QWidget*  w);
    QString py_toString(QStyleOption*);
void py_set_palette(QStyleOption* theWrappedObject, QPalette  palette){ theWrappedObject->palette = palette; }
QPalette  py_get_palette(QStyleOption* theWrappedObject){ return theWrappedObject->palette; }
void py_set_version(QStyleOption* theWrappedObject, int  version){ theWrappedObject->version = version; }
int  py_get_version(QStyleOption* theWrappedObject){ return theWrappedObject->version; }
void py_set_direction(QStyleOption* theWrappedObject, Qt::LayoutDirection  direction){ theWrappedObject->direction = direction; }
Qt::LayoutDirection  py_get_direction(QStyleOption* theWrappedObject){ return theWrappedObject->direction; }
void py_set_rect(QStyleOption* theWrappedObject, QRect  rect){ theWrappedObject->rect = rect; }
QRect  py_get_rect(QStyleOption* theWrappedObject){ return theWrappedObject->rect; }
void py_set_type(QStyleOption* theWrappedObject, int  type){ theWrappedObject->type = type; }
int  py_get_type(QStyleOption* theWrappedObject){ return theWrappedObject->type; }
void py_set_state(QStyleOption* theWrappedObject, QStyle::State  state){ theWrappedObject->state = state; }
QStyle::State  py_get_state(QStyleOption* theWrappedObject){ return theWrappedObject->state; }
void py_set_fontMetrics(QStyleOption* theWrappedObject, QFontMetrics  fontMetrics){ theWrappedObject->fontMetrics = fontMetrics; }
QFontMetrics  py_get_fontMetrics(QStyleOption* theWrappedObject){ return theWrappedObject->fontMetrics; }
};





class PythonQtShell_QStyleOptionButton : public QStyleOptionButton
{
public:
    PythonQtShell_QStyleOptionButton():QStyleOptionButton(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionButton(const QStyleOptionButton&  other):QStyleOptionButton(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionButton(int  version):QStyleOptionButton(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionButton : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion ButtonFeature StyleOptionType )
Q_FLAGS(ButtonFeatures )
enum StyleOptionVersion{
  Version = QStyleOptionButton::Version};
enum ButtonFeature{
  None = QStyleOptionButton::None,   Flat = QStyleOptionButton::Flat,   HasMenu = QStyleOptionButton::HasMenu,   DefaultButton = QStyleOptionButton::DefaultButton,   AutoDefaultButton = QStyleOptionButton::AutoDefaultButton,   CommandLinkButton = QStyleOptionButton::CommandLinkButton};
enum StyleOptionType{
  Type = QStyleOptionButton::Type};
Q_DECLARE_FLAGS(ButtonFeatures, ButtonFeature)
public slots:
QStyleOptionButton* new_QStyleOptionButton();
QStyleOptionButton* new_QStyleOptionButton(const QStyleOptionButton&  other);
void delete_QStyleOptionButton(QStyleOptionButton* obj) { delete obj; } 
void py_set_iconSize(QStyleOptionButton* theWrappedObject, QSize  iconSize){ theWrappedObject->iconSize = iconSize; }
QSize  py_get_iconSize(QStyleOptionButton* theWrappedObject){ return theWrappedObject->iconSize; }
void py_set_features(QStyleOptionButton* theWrappedObject, QStyleOptionButton::ButtonFeatures  features){ theWrappedObject->features = features; }
QStyleOptionButton::ButtonFeatures  py_get_features(QStyleOptionButton* theWrappedObject){ return theWrappedObject->features; }
void py_set_icon(QStyleOptionButton* theWrappedObject, QIcon  icon){ theWrappedObject->icon = icon; }
QIcon  py_get_icon(QStyleOptionButton* theWrappedObject){ return theWrappedObject->icon; }
void py_set_text(QStyleOptionButton* theWrappedObject, QString  text){ theWrappedObject->text = text; }
QString  py_get_text(QStyleOptionButton* theWrappedObject){ return theWrappedObject->text; }
};





class PythonQtShell_QStyleOptionComboBox : public QStyleOptionComboBox
{
public:
    PythonQtShell_QStyleOptionComboBox():QStyleOptionComboBox(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionComboBox(const QStyleOptionComboBox&  other):QStyleOptionComboBox(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionComboBox(int  version):QStyleOptionComboBox(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionComboBox : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion StyleOptionType )
enum StyleOptionVersion{
  Version = QStyleOptionComboBox::Version};
enum StyleOptionType{
  Type = QStyleOptionComboBox::Type};
public slots:
QStyleOptionComboBox* new_QStyleOptionComboBox();
QStyleOptionComboBox* new_QStyleOptionComboBox(const QStyleOptionComboBox&  other);
void delete_QStyleOptionComboBox(QStyleOptionComboBox* obj) { delete obj; } 
void py_set_currentText(QStyleOptionComboBox* theWrappedObject, QString  currentText){ theWrappedObject->currentText = currentText; }
QString  py_get_currentText(QStyleOptionComboBox* theWrappedObject){ return theWrappedObject->currentText; }
void py_set_iconSize(QStyleOptionComboBox* theWrappedObject, QSize  iconSize){ theWrappedObject->iconSize = iconSize; }
QSize  py_get_iconSize(QStyleOptionComboBox* theWrappedObject){ return theWrappedObject->iconSize; }
void py_set_editable(QStyleOptionComboBox* theWrappedObject, bool  editable){ theWrappedObject->editable = editable; }
bool  py_get_editable(QStyleOptionComboBox* theWrappedObject){ return theWrappedObject->editable; }
void py_set_frame(QStyleOptionComboBox* theWrappedObject, bool  frame){ theWrappedObject->frame = frame; }
bool  py_get_frame(QStyleOptionComboBox* theWrappedObject){ return theWrappedObject->frame; }
void py_set_popupRect(QStyleOptionComboBox* theWrappedObject, QRect  popupRect){ theWrappedObject->popupRect = popupRect; }
QRect  py_get_popupRect(QStyleOptionComboBox* theWrappedObject){ return theWrappedObject->popupRect; }
void py_set_currentIcon(QStyleOptionComboBox* theWrappedObject, QIcon  currentIcon){ theWrappedObject->currentIcon = currentIcon; }
QIcon  py_get_currentIcon(QStyleOptionComboBox* theWrappedObject){ return theWrappedObject->currentIcon; }
};





class PythonQtShell_QStyleOptionDockWidget : public QStyleOptionDockWidget
{
public:
    PythonQtShell_QStyleOptionDockWidget():QStyleOptionDockWidget(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionDockWidget(const QStyleOptionDockWidget&  other):QStyleOptionDockWidget(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionDockWidget(int  version):QStyleOptionDockWidget(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionDockWidget : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion StyleOptionType )
enum StyleOptionVersion{
  Version = QStyleOptionDockWidget::Version};
enum StyleOptionType{
  Type = QStyleOptionDockWidget::Type};
public slots:
QStyleOptionDockWidget* new_QStyleOptionDockWidget();
QStyleOptionDockWidget* new_QStyleOptionDockWidget(const QStyleOptionDockWidget&  other);
void delete_QStyleOptionDockWidget(QStyleOptionDockWidget* obj) { delete obj; } 
void py_set_title(QStyleOptionDockWidget* theWrappedObject, QString  title){ theWrappedObject->title = title; }
QString  py_get_title(QStyleOptionDockWidget* theWrappedObject){ return theWrappedObject->title; }
void py_set_floatable(QStyleOptionDockWidget* theWrappedObject, bool  floatable){ theWrappedObject->floatable = floatable; }
bool  py_get_floatable(QStyleOptionDockWidget* theWrappedObject){ return theWrappedObject->floatable; }
void py_set_closable(QStyleOptionDockWidget* theWrappedObject, bool  closable){ theWrappedObject->closable = closable; }
bool  py_get_closable(QStyleOptionDockWidget* theWrappedObject){ return theWrappedObject->closable; }
void py_set_movable(QStyleOptionDockWidget* theWrappedObject, bool  movable){ theWrappedObject->movable = movable; }
bool  py_get_movable(QStyleOptionDockWidget* theWrappedObject){ return theWrappedObject->movable; }
};





class PythonQtShell_QStyleOptionDockWidgetV2 : public QStyleOptionDockWidgetV2
{
public:
    PythonQtShell_QStyleOptionDockWidgetV2():QStyleOptionDockWidgetV2(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionDockWidgetV2(const QStyleOptionDockWidget&  other):QStyleOptionDockWidgetV2(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionDockWidgetV2(const QStyleOptionDockWidgetV2&  other):QStyleOptionDockWidgetV2(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionDockWidgetV2(int  version):QStyleOptionDockWidgetV2(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionDockWidgetV2 : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion )
enum StyleOptionVersion{
  Version = QStyleOptionDockWidgetV2::Version};
public slots:
QStyleOptionDockWidgetV2* new_QStyleOptionDockWidgetV2();
QStyleOptionDockWidgetV2* new_QStyleOptionDockWidgetV2(const QStyleOptionDockWidget&  other);
QStyleOptionDockWidgetV2* new_QStyleOptionDockWidgetV2(const QStyleOptionDockWidgetV2&  other);
void delete_QStyleOptionDockWidgetV2(QStyleOptionDockWidgetV2* obj) { delete obj; } 
void py_set_verticalTitleBar(QStyleOptionDockWidgetV2* theWrappedObject, bool  verticalTitleBar){ theWrappedObject->verticalTitleBar = verticalTitleBar; }
bool  py_get_verticalTitleBar(QStyleOptionDockWidgetV2* theWrappedObject){ return theWrappedObject->verticalTitleBar; }
};





class PythonQtShell_QStyleOptionFocusRect : public QStyleOptionFocusRect
{
public:
    PythonQtShell_QStyleOptionFocusRect():QStyleOptionFocusRect(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionFocusRect(const QStyleOptionFocusRect&  other):QStyleOptionFocusRect(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionFocusRect(int  version):QStyleOptionFocusRect(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionFocusRect : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion StyleOptionType )
enum StyleOptionVersion{
  Version = QStyleOptionFocusRect::Version};
enum StyleOptionType{
  Type = QStyleOptionFocusRect::Type};
public slots:
QStyleOptionFocusRect* new_QStyleOptionFocusRect();
QStyleOptionFocusRect* new_QStyleOptionFocusRect(const QStyleOptionFocusRect&  other);
void delete_QStyleOptionFocusRect(QStyleOptionFocusRect* obj) { delete obj; } 
void py_set_backgroundColor(QStyleOptionFocusRect* theWrappedObject, QColor  backgroundColor){ theWrappedObject->backgroundColor = backgroundColor; }
QColor  py_get_backgroundColor(QStyleOptionFocusRect* theWrappedObject){ return theWrappedObject->backgroundColor; }
};





class PythonQtShell_QStyleOptionFrame : public QStyleOptionFrame
{
public:
    PythonQtShell_QStyleOptionFrame():QStyleOptionFrame(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionFrame(const QStyleOptionFrame&  other):QStyleOptionFrame(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionFrame(int  version):QStyleOptionFrame(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionFrame : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion StyleOptionType )
enum StyleOptionVersion{
  Version = QStyleOptionFrame::Version};
enum StyleOptionType{
  Type = QStyleOptionFrame::Type};
public slots:
QStyleOptionFrame* new_QStyleOptionFrame();
QStyleOptionFrame* new_QStyleOptionFrame(const QStyleOptionFrame&  other);
void delete_QStyleOptionFrame(QStyleOptionFrame* obj) { delete obj; } 
void py_set_lineWidth(QStyleOptionFrame* theWrappedObject, int  lineWidth){ theWrappedObject->lineWidth = lineWidth; }
int  py_get_lineWidth(QStyleOptionFrame* theWrappedObject){ return theWrappedObject->lineWidth; }
void py_set_midLineWidth(QStyleOptionFrame* theWrappedObject, int  midLineWidth){ theWrappedObject->midLineWidth = midLineWidth; }
int  py_get_midLineWidth(QStyleOptionFrame* theWrappedObject){ return theWrappedObject->midLineWidth; }
};





class PythonQtShell_QStyleOptionFrameV2 : public QStyleOptionFrameV2
{
public:
    PythonQtShell_QStyleOptionFrameV2():QStyleOptionFrameV2(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionFrameV2(const QStyleOptionFrame&  other):QStyleOptionFrameV2(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionFrameV2(const QStyleOptionFrameV2&  other):QStyleOptionFrameV2(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionFrameV2(int  version):QStyleOptionFrameV2(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionFrameV2 : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion FrameFeature )
Q_FLAGS(FrameFeatures )
enum StyleOptionVersion{
  Version = QStyleOptionFrameV2::Version};
enum FrameFeature{
  None = QStyleOptionFrameV2::None,   Flat = QStyleOptionFrameV2::Flat};
Q_DECLARE_FLAGS(FrameFeatures, FrameFeature)
public slots:
QStyleOptionFrameV2* new_QStyleOptionFrameV2();
QStyleOptionFrameV2* new_QStyleOptionFrameV2(const QStyleOptionFrame&  other);
QStyleOptionFrameV2* new_QStyleOptionFrameV2(const QStyleOptionFrameV2&  other);
void delete_QStyleOptionFrameV2(QStyleOptionFrameV2* obj) { delete obj; } 
void py_set_features(QStyleOptionFrameV2* theWrappedObject, QStyleOptionFrameV2::FrameFeatures  features){ theWrappedObject->features = features; }
QStyleOptionFrameV2::FrameFeatures  py_get_features(QStyleOptionFrameV2* theWrappedObject){ return theWrappedObject->features; }
};





class PythonQtShell_QStyleOptionFrameV3 : public QStyleOptionFrameV3
{
public:
    PythonQtShell_QStyleOptionFrameV3():QStyleOptionFrameV3(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionFrameV3(const QStyleOptionFrame&  other):QStyleOptionFrameV3(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionFrameV3(const QStyleOptionFrameV3&  other):QStyleOptionFrameV3(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionFrameV3(int  version):QStyleOptionFrameV3(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionFrameV3 : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion )
enum StyleOptionVersion{
  Version = QStyleOptionFrameV3::Version};
public slots:
QStyleOptionFrameV3* new_QStyleOptionFrameV3();
QStyleOptionFrameV3* new_QStyleOptionFrameV3(const QStyleOptionFrame&  other);
QStyleOptionFrameV3* new_QStyleOptionFrameV3(const QStyleOptionFrameV3&  other);
void delete_QStyleOptionFrameV3(QStyleOptionFrameV3* obj) { delete obj; } 
void py_set_frameShape(QStyleOptionFrameV3* theWrappedObject, QFrame::Shape  frameShape){ theWrappedObject->frameShape = frameShape; }
QFrame::Shape  py_get_frameShape(QStyleOptionFrameV3* theWrappedObject){ return theWrappedObject->frameShape; }
void py_set_unused(QStyleOptionFrameV3* theWrappedObject, uint  unused){ theWrappedObject->unused = unused; }
uint  py_get_unused(QStyleOptionFrameV3* theWrappedObject){ return theWrappedObject->unused; }
};





class PythonQtShell_QStyleOptionGraphicsItem : public QStyleOptionGraphicsItem
{
public:
    PythonQtShell_QStyleOptionGraphicsItem():QStyleOptionGraphicsItem(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionGraphicsItem(const QStyleOptionGraphicsItem&  other):QStyleOptionGraphicsItem(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionGraphicsItem(int  version):QStyleOptionGraphicsItem(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionGraphicsItem : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion StyleOptionType )
enum StyleOptionVersion{
  Version = QStyleOptionGraphicsItem::Version};
enum StyleOptionType{
  Type = QStyleOptionGraphicsItem::Type};
public slots:
QStyleOptionGraphicsItem* new_QStyleOptionGraphicsItem();
QStyleOptionGraphicsItem* new_QStyleOptionGraphicsItem(const QStyleOptionGraphicsItem&  other);
void delete_QStyleOptionGraphicsItem(QStyleOptionGraphicsItem* obj) { delete obj; } 
   qreal  static_QStyleOptionGraphicsItem_levelOfDetailFromTransform(const QTransform&  worldTransform);
void py_set_exposedRect(QStyleOptionGraphicsItem* theWrappedObject, QRectF  exposedRect){ theWrappedObject->exposedRect = exposedRect; }
QRectF  py_get_exposedRect(QStyleOptionGraphicsItem* theWrappedObject){ return theWrappedObject->exposedRect; }
void py_set_matrix(QStyleOptionGraphicsItem* theWrappedObject, QMatrix  matrix){ theWrappedObject->matrix = matrix; }
QMatrix  py_get_matrix(QStyleOptionGraphicsItem* theWrappedObject){ return theWrappedObject->matrix; }
void py_set_levelOfDetail(QStyleOptionGraphicsItem* theWrappedObject, qreal  levelOfDetail){ theWrappedObject->levelOfDetail = levelOfDetail; }
qreal  py_get_levelOfDetail(QStyleOptionGraphicsItem* theWrappedObject){ return theWrappedObject->levelOfDetail; }
};





class PythonQtShell_QStyleOptionGroupBox : public QStyleOptionGroupBox
{
public:
    PythonQtShell_QStyleOptionGroupBox():QStyleOptionGroupBox(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionGroupBox(const QStyleOptionGroupBox&  other):QStyleOptionGroupBox(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionGroupBox(int  version):QStyleOptionGroupBox(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionGroupBox : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion StyleOptionType )
enum StyleOptionVersion{
  Version = QStyleOptionGroupBox::Version};
enum StyleOptionType{
  Type = QStyleOptionGroupBox::Type};
public slots:
QStyleOptionGroupBox* new_QStyleOptionGroupBox();
QStyleOptionGroupBox* new_QStyleOptionGroupBox(const QStyleOptionGroupBox&  other);
void delete_QStyleOptionGroupBox(QStyleOptionGroupBox* obj) { delete obj; } 
void py_set_lineWidth(QStyleOptionGroupBox* theWrappedObject, int  lineWidth){ theWrappedObject->lineWidth = lineWidth; }
int  py_get_lineWidth(QStyleOptionGroupBox* theWrappedObject){ return theWrappedObject->lineWidth; }
void py_set_features(QStyleOptionGroupBox* theWrappedObject, QStyleOptionFrameV2::FrameFeatures  features){ theWrappedObject->features = features; }
QStyleOptionFrameV2::FrameFeatures  py_get_features(QStyleOptionGroupBox* theWrappedObject){ return theWrappedObject->features; }
void py_set_textAlignment(QStyleOptionGroupBox* theWrappedObject, Qt::Alignment  textAlignment){ theWrappedObject->textAlignment = textAlignment; }
Qt::Alignment  py_get_textAlignment(QStyleOptionGroupBox* theWrappedObject){ return theWrappedObject->textAlignment; }
void py_set_midLineWidth(QStyleOptionGroupBox* theWrappedObject, int  midLineWidth){ theWrappedObject->midLineWidth = midLineWidth; }
int  py_get_midLineWidth(QStyleOptionGroupBox* theWrappedObject){ return theWrappedObject->midLineWidth; }
void py_set_text(QStyleOptionGroupBox* theWrappedObject, QString  text){ theWrappedObject->text = text; }
QString  py_get_text(QStyleOptionGroupBox* theWrappedObject){ return theWrappedObject->text; }
void py_set_textColor(QStyleOptionGroupBox* theWrappedObject, QColor  textColor){ theWrappedObject->textColor = textColor; }
QColor  py_get_textColor(QStyleOptionGroupBox* theWrappedObject){ return theWrappedObject->textColor; }
};





class PythonQtShell_QStyleOptionHeader : public QStyleOptionHeader
{
public:
    PythonQtShell_QStyleOptionHeader():QStyleOptionHeader(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionHeader(const QStyleOptionHeader&  other):QStyleOptionHeader(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionHeader(int  version):QStyleOptionHeader(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionHeader : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion SectionPosition SelectedPosition StyleOptionType SortIndicator )
enum StyleOptionVersion{
  Version = QStyleOptionHeader::Version};
enum SectionPosition{
  Beginning = QStyleOptionHeader::Beginning,   Middle = QStyleOptionHeader::Middle,   End = QStyleOptionHeader::End,   OnlyOneSection = QStyleOptionHeader::OnlyOneSection};
enum SelectedPosition{
  NotAdjacent = QStyleOptionHeader::NotAdjacent,   NextIsSelected = QStyleOptionHeader::NextIsSelected,   PreviousIsSelected = QStyleOptionHeader::PreviousIsSelected,   NextAndPreviousAreSelected = QStyleOptionHeader::NextAndPreviousAreSelected};
enum StyleOptionType{
  Type = QStyleOptionHeader::Type};
enum SortIndicator{
  None = QStyleOptionHeader::None,   SortUp = QStyleOptionHeader::SortUp,   SortDown = QStyleOptionHeader::SortDown};
public slots:
QStyleOptionHeader* new_QStyleOptionHeader();
QStyleOptionHeader* new_QStyleOptionHeader(const QStyleOptionHeader&  other);
void delete_QStyleOptionHeader(QStyleOptionHeader* obj) { delete obj; } 
void py_set_section(QStyleOptionHeader* theWrappedObject, int  section){ theWrappedObject->section = section; }
int  py_get_section(QStyleOptionHeader* theWrappedObject){ return theWrappedObject->section; }
void py_set_sortIndicator(QStyleOptionHeader* theWrappedObject, QStyleOptionHeader::SortIndicator  sortIndicator){ theWrappedObject->sortIndicator = sortIndicator; }
QStyleOptionHeader::SortIndicator  py_get_sortIndicator(QStyleOptionHeader* theWrappedObject){ return theWrappedObject->sortIndicator; }
void py_set_position(QStyleOptionHeader* theWrappedObject, QStyleOptionHeader::SectionPosition  position){ theWrappedObject->position = position; }
QStyleOptionHeader::SectionPosition  py_get_position(QStyleOptionHeader* theWrappedObject){ return theWrappedObject->position; }
void py_set_icon(QStyleOptionHeader* theWrappedObject, QIcon  icon){ theWrappedObject->icon = icon; }
QIcon  py_get_icon(QStyleOptionHeader* theWrappedObject){ return theWrappedObject->icon; }
void py_set_textAlignment(QStyleOptionHeader* theWrappedObject, Qt::Alignment  textAlignment){ theWrappedObject->textAlignment = textAlignment; }
Qt::Alignment  py_get_textAlignment(QStyleOptionHeader* theWrappedObject){ return theWrappedObject->textAlignment; }
void py_set_text(QStyleOptionHeader* theWrappedObject, QString  text){ theWrappedObject->text = text; }
QString  py_get_text(QStyleOptionHeader* theWrappedObject){ return theWrappedObject->text; }
void py_set_selectedPosition(QStyleOptionHeader* theWrappedObject, QStyleOptionHeader::SelectedPosition  selectedPosition){ theWrappedObject->selectedPosition = selectedPosition; }
QStyleOptionHeader::SelectedPosition  py_get_selectedPosition(QStyleOptionHeader* theWrappedObject){ return theWrappedObject->selectedPosition; }
void py_set_iconAlignment(QStyleOptionHeader* theWrappedObject, Qt::Alignment  iconAlignment){ theWrappedObject->iconAlignment = iconAlignment; }
Qt::Alignment  py_get_iconAlignment(QStyleOptionHeader* theWrappedObject){ return theWrappedObject->iconAlignment; }
void py_set_orientation(QStyleOptionHeader* theWrappedObject, Qt::Orientation  orientation){ theWrappedObject->orientation = orientation; }
Qt::Orientation  py_get_orientation(QStyleOptionHeader* theWrappedObject){ return theWrappedObject->orientation; }
};





class PythonQtShell_QStyleOptionMenuItem : public QStyleOptionMenuItem
{
public:
    PythonQtShell_QStyleOptionMenuItem():QStyleOptionMenuItem(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionMenuItem(const QStyleOptionMenuItem&  other):QStyleOptionMenuItem(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionMenuItem(int  version):QStyleOptionMenuItem(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionMenuItem : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion StyleOptionType CheckType MenuItemType )
enum StyleOptionVersion{
  Version = QStyleOptionMenuItem::Version};
enum StyleOptionType{
  Type = QStyleOptionMenuItem::Type};
enum CheckType{
  NotCheckable = QStyleOptionMenuItem::NotCheckable,   Exclusive = QStyleOptionMenuItem::Exclusive,   NonExclusive = QStyleOptionMenuItem::NonExclusive};
enum MenuItemType{
  Normal = QStyleOptionMenuItem::Normal,   DefaultItem = QStyleOptionMenuItem::DefaultItem,   Separator = QStyleOptionMenuItem::Separator,   SubMenu = QStyleOptionMenuItem::SubMenu,   Scroller = QStyleOptionMenuItem::Scroller,   TearOff = QStyleOptionMenuItem::TearOff,   Margin = QStyleOptionMenuItem::Margin,   EmptyArea = QStyleOptionMenuItem::EmptyArea};
public slots:
QStyleOptionMenuItem* new_QStyleOptionMenuItem();
QStyleOptionMenuItem* new_QStyleOptionMenuItem(const QStyleOptionMenuItem&  other);
void delete_QStyleOptionMenuItem(QStyleOptionMenuItem* obj) { delete obj; } 
void py_set_menuHasCheckableItems(QStyleOptionMenuItem* theWrappedObject, bool  menuHasCheckableItems){ theWrappedObject->menuHasCheckableItems = menuHasCheckableItems; }
bool  py_get_menuHasCheckableItems(QStyleOptionMenuItem* theWrappedObject){ return theWrappedObject->menuHasCheckableItems; }
void py_set_menuRect(QStyleOptionMenuItem* theWrappedObject, QRect  menuRect){ theWrappedObject->menuRect = menuRect; }
QRect  py_get_menuRect(QStyleOptionMenuItem* theWrappedObject){ return theWrappedObject->menuRect; }
void py_set_checkType(QStyleOptionMenuItem* theWrappedObject, QStyleOptionMenuItem::CheckType  checkType){ theWrappedObject->checkType = checkType; }
QStyleOptionMenuItem::CheckType  py_get_checkType(QStyleOptionMenuItem* theWrappedObject){ return theWrappedObject->checkType; }
void py_set_icon(QStyleOptionMenuItem* theWrappedObject, QIcon  icon){ theWrappedObject->icon = icon; }
QIcon  py_get_icon(QStyleOptionMenuItem* theWrappedObject){ return theWrappedObject->icon; }
void py_set_checked(QStyleOptionMenuItem* theWrappedObject, bool  checked){ theWrappedObject->checked = checked; }
bool  py_get_checked(QStyleOptionMenuItem* theWrappedObject){ return theWrappedObject->checked; }
void py_set_maxIconWidth(QStyleOptionMenuItem* theWrappedObject, int  maxIconWidth){ theWrappedObject->maxIconWidth = maxIconWidth; }
int  py_get_maxIconWidth(QStyleOptionMenuItem* theWrappedObject){ return theWrappedObject->maxIconWidth; }
void py_set_tabWidth(QStyleOptionMenuItem* theWrappedObject, int  tabWidth){ theWrappedObject->tabWidth = tabWidth; }
int  py_get_tabWidth(QStyleOptionMenuItem* theWrappedObject){ return theWrappedObject->tabWidth; }
void py_set_text(QStyleOptionMenuItem* theWrappedObject, QString  text){ theWrappedObject->text = text; }
QString  py_get_text(QStyleOptionMenuItem* theWrappedObject){ return theWrappedObject->text; }
void py_set_font(QStyleOptionMenuItem* theWrappedObject, QFont  font){ theWrappedObject->font = font; }
QFont  py_get_font(QStyleOptionMenuItem* theWrappedObject){ return theWrappedObject->font; }
void py_set_menuItemType(QStyleOptionMenuItem* theWrappedObject, QStyleOptionMenuItem::MenuItemType  menuItemType){ theWrappedObject->menuItemType = menuItemType; }
QStyleOptionMenuItem::MenuItemType  py_get_menuItemType(QStyleOptionMenuItem* theWrappedObject){ return theWrappedObject->menuItemType; }
};





class PythonQtShell_QStyleOptionProgressBar : public QStyleOptionProgressBar
{
public:
    PythonQtShell_QStyleOptionProgressBar():QStyleOptionProgressBar(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionProgressBar(const QStyleOptionProgressBar&  other):QStyleOptionProgressBar(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionProgressBar(int  version):QStyleOptionProgressBar(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionProgressBar : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion StyleOptionType )
enum StyleOptionVersion{
  Version = QStyleOptionProgressBar::Version};
enum StyleOptionType{
  Type = QStyleOptionProgressBar::Type};
public slots:
QStyleOptionProgressBar* new_QStyleOptionProgressBar();
QStyleOptionProgressBar* new_QStyleOptionProgressBar(const QStyleOptionProgressBar&  other);
void delete_QStyleOptionProgressBar(QStyleOptionProgressBar* obj) { delete obj; } 
void py_set_minimum(QStyleOptionProgressBar* theWrappedObject, int  minimum){ theWrappedObject->minimum = minimum; }
int  py_get_minimum(QStyleOptionProgressBar* theWrappedObject){ return theWrappedObject->minimum; }
void py_set_maximum(QStyleOptionProgressBar* theWrappedObject, int  maximum){ theWrappedObject->maximum = maximum; }
int  py_get_maximum(QStyleOptionProgressBar* theWrappedObject){ return theWrappedObject->maximum; }
void py_set_progress(QStyleOptionProgressBar* theWrappedObject, int  progress){ theWrappedObject->progress = progress; }
int  py_get_progress(QStyleOptionProgressBar* theWrappedObject){ return theWrappedObject->progress; }
void py_set_textAlignment(QStyleOptionProgressBar* theWrappedObject, Qt::Alignment  textAlignment){ theWrappedObject->textAlignment = textAlignment; }
Qt::Alignment  py_get_textAlignment(QStyleOptionProgressBar* theWrappedObject){ return theWrappedObject->textAlignment; }
void py_set_text(QStyleOptionProgressBar* theWrappedObject, QString  text){ theWrappedObject->text = text; }
QString  py_get_text(QStyleOptionProgressBar* theWrappedObject){ return theWrappedObject->text; }
void py_set_textVisible(QStyleOptionProgressBar* theWrappedObject, bool  textVisible){ theWrappedObject->textVisible = textVisible; }
bool  py_get_textVisible(QStyleOptionProgressBar* theWrappedObject){ return theWrappedObject->textVisible; }
};





class PythonQtShell_QStyleOptionProgressBarV2 : public QStyleOptionProgressBarV2
{
public:
    PythonQtShell_QStyleOptionProgressBarV2():QStyleOptionProgressBarV2(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionProgressBarV2(const QStyleOptionProgressBar&  other):QStyleOptionProgressBarV2(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionProgressBarV2(const QStyleOptionProgressBarV2&  other):QStyleOptionProgressBarV2(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionProgressBarV2(int  version):QStyleOptionProgressBarV2(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionProgressBarV2 : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion StyleOptionType )
enum StyleOptionVersion{
  Version = QStyleOptionProgressBarV2::Version};
enum StyleOptionType{
  Type = QStyleOptionProgressBarV2::Type};
public slots:
QStyleOptionProgressBarV2* new_QStyleOptionProgressBarV2();
QStyleOptionProgressBarV2* new_QStyleOptionProgressBarV2(const QStyleOptionProgressBar&  other);
QStyleOptionProgressBarV2* new_QStyleOptionProgressBarV2(const QStyleOptionProgressBarV2&  other);
void delete_QStyleOptionProgressBarV2(QStyleOptionProgressBarV2* obj) { delete obj; } 
void py_set_bottomToTop(QStyleOptionProgressBarV2* theWrappedObject, bool  bottomToTop){ theWrappedObject->bottomToTop = bottomToTop; }
bool  py_get_bottomToTop(QStyleOptionProgressBarV2* theWrappedObject){ return theWrappedObject->bottomToTop; }
void py_set_invertedAppearance(QStyleOptionProgressBarV2* theWrappedObject, bool  invertedAppearance){ theWrappedObject->invertedAppearance = invertedAppearance; }
bool  py_get_invertedAppearance(QStyleOptionProgressBarV2* theWrappedObject){ return theWrappedObject->invertedAppearance; }
void py_set_orientation(QStyleOptionProgressBarV2* theWrappedObject, Qt::Orientation  orientation){ theWrappedObject->orientation = orientation; }
Qt::Orientation  py_get_orientation(QStyleOptionProgressBarV2* theWrappedObject){ return theWrappedObject->orientation; }
};





class PythonQtShell_QStyleOptionRubberBand : public QStyleOptionRubberBand
{
public:
    PythonQtShell_QStyleOptionRubberBand():QStyleOptionRubberBand(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionRubberBand(const QStyleOptionRubberBand&  other):QStyleOptionRubberBand(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionRubberBand(int  version):QStyleOptionRubberBand(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionRubberBand : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion StyleOptionType )
enum StyleOptionVersion{
  Version = QStyleOptionRubberBand::Version};
enum StyleOptionType{
  Type = QStyleOptionRubberBand::Type};
public slots:
QStyleOptionRubberBand* new_QStyleOptionRubberBand();
QStyleOptionRubberBand* new_QStyleOptionRubberBand(const QStyleOptionRubberBand&  other);
void delete_QStyleOptionRubberBand(QStyleOptionRubberBand* obj) { delete obj; } 
void py_set_opaque(QStyleOptionRubberBand* theWrappedObject, bool  opaque){ theWrappedObject->opaque = opaque; }
bool  py_get_opaque(QStyleOptionRubberBand* theWrappedObject){ return theWrappedObject->opaque; }
void py_set_shape(QStyleOptionRubberBand* theWrappedObject, QRubberBand::Shape  shape){ theWrappedObject->shape = shape; }
QRubberBand::Shape  py_get_shape(QStyleOptionRubberBand* theWrappedObject){ return theWrappedObject->shape; }
};





class PythonQtShell_QStyleOptionSizeGrip : public QStyleOptionSizeGrip
{
public:
    PythonQtShell_QStyleOptionSizeGrip():QStyleOptionSizeGrip(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionSizeGrip(const QStyleOptionSizeGrip&  other):QStyleOptionSizeGrip(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionSizeGrip(int  version):QStyleOptionSizeGrip(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionSizeGrip : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion StyleOptionType )
enum StyleOptionVersion{
  Version = QStyleOptionSizeGrip::Version};
enum StyleOptionType{
  Type = QStyleOptionSizeGrip::Type};
public slots:
QStyleOptionSizeGrip* new_QStyleOptionSizeGrip();
QStyleOptionSizeGrip* new_QStyleOptionSizeGrip(const QStyleOptionSizeGrip&  other);
void delete_QStyleOptionSizeGrip(QStyleOptionSizeGrip* obj) { delete obj; } 
void py_set_corner(QStyleOptionSizeGrip* theWrappedObject, Qt::Corner  corner){ theWrappedObject->corner = corner; }
Qt::Corner  py_get_corner(QStyleOptionSizeGrip* theWrappedObject){ return theWrappedObject->corner; }
};





class PythonQtShell_QStyleOptionSlider : public QStyleOptionSlider
{
public:
    PythonQtShell_QStyleOptionSlider():QStyleOptionSlider(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionSlider(const QStyleOptionSlider&  other):QStyleOptionSlider(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionSlider(int  version):QStyleOptionSlider(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionSlider : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion StyleOptionType )
enum StyleOptionVersion{
  Version = QStyleOptionSlider::Version};
enum StyleOptionType{
  Type = QStyleOptionSlider::Type};
public slots:
QStyleOptionSlider* new_QStyleOptionSlider();
QStyleOptionSlider* new_QStyleOptionSlider(const QStyleOptionSlider&  other);
void delete_QStyleOptionSlider(QStyleOptionSlider* obj) { delete obj; } 
void py_set_tickInterval(QStyleOptionSlider* theWrappedObject, int  tickInterval){ theWrappedObject->tickInterval = tickInterval; }
int  py_get_tickInterval(QStyleOptionSlider* theWrappedObject){ return theWrappedObject->tickInterval; }
void py_set_minimum(QStyleOptionSlider* theWrappedObject, int  minimum){ theWrappedObject->minimum = minimum; }
int  py_get_minimum(QStyleOptionSlider* theWrappedObject){ return theWrappedObject->minimum; }
void py_set_notchTarget(QStyleOptionSlider* theWrappedObject, qreal  notchTarget){ theWrappedObject->notchTarget = notchTarget; }
qreal  py_get_notchTarget(QStyleOptionSlider* theWrappedObject){ return theWrappedObject->notchTarget; }
void py_set_maximum(QStyleOptionSlider* theWrappedObject, int  maximum){ theWrappedObject->maximum = maximum; }
int  py_get_maximum(QStyleOptionSlider* theWrappedObject){ return theWrappedObject->maximum; }
void py_set_sliderPosition(QStyleOptionSlider* theWrappedObject, int  sliderPosition){ theWrappedObject->sliderPosition = sliderPosition; }
int  py_get_sliderPosition(QStyleOptionSlider* theWrappedObject){ return theWrappedObject->sliderPosition; }
void py_set_dialWrapping(QStyleOptionSlider* theWrappedObject, bool  dialWrapping){ theWrappedObject->dialWrapping = dialWrapping; }
bool  py_get_dialWrapping(QStyleOptionSlider* theWrappedObject){ return theWrappedObject->dialWrapping; }
void py_set_sliderValue(QStyleOptionSlider* theWrappedObject, int  sliderValue){ theWrappedObject->sliderValue = sliderValue; }
int  py_get_sliderValue(QStyleOptionSlider* theWrappedObject){ return theWrappedObject->sliderValue; }
void py_set_singleStep(QStyleOptionSlider* theWrappedObject, int  singleStep){ theWrappedObject->singleStep = singleStep; }
int  py_get_singleStep(QStyleOptionSlider* theWrappedObject){ return theWrappedObject->singleStep; }
void py_set_tickPosition(QStyleOptionSlider* theWrappedObject, QSlider::TickPosition  tickPosition){ theWrappedObject->tickPosition = tickPosition; }
QSlider::TickPosition  py_get_tickPosition(QStyleOptionSlider* theWrappedObject){ return theWrappedObject->tickPosition; }
void py_set_pageStep(QStyleOptionSlider* theWrappedObject, int  pageStep){ theWrappedObject->pageStep = pageStep; }
int  py_get_pageStep(QStyleOptionSlider* theWrappedObject){ return theWrappedObject->pageStep; }
void py_set_orientation(QStyleOptionSlider* theWrappedObject, Qt::Orientation  orientation){ theWrappedObject->orientation = orientation; }
Qt::Orientation  py_get_orientation(QStyleOptionSlider* theWrappedObject){ return theWrappedObject->orientation; }
void py_set_upsideDown(QStyleOptionSlider* theWrappedObject, bool  upsideDown){ theWrappedObject->upsideDown = upsideDown; }
bool  py_get_upsideDown(QStyleOptionSlider* theWrappedObject){ return theWrappedObject->upsideDown; }
};





class PythonQtShell_QStyleOptionSpinBox : public QStyleOptionSpinBox
{
public:
    PythonQtShell_QStyleOptionSpinBox():QStyleOptionSpinBox(),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionSpinBox(const QStyleOptionSpinBox&  other):QStyleOptionSpinBox(other),_wrapper(NULL) {};
    PythonQtShell_QStyleOptionSpinBox(int  version):QStyleOptionSpinBox(version),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QStyleOptionSpinBox : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StyleOptionVersion StyleOptionType )
enum StyleOptionVersion{
  Version = QStyleOptionSpinBox::Version};
enum StyleOptionType{
  Type = QStyleOptionSpinBox::Type};
public slots:
QStyleOptionSpinBox* new_QStyleOptionSpinBox();
QStyleOptionSpinBox* new_QStyleOptionSpinBox(const QStyleOptionSpinBox&  other);
void delete_QStyleOptionSpinBox(QStyleOptionSpinBox* obj) { delete obj; } 
void py_set_stepEnabled(QStyleOptionSpinBox* theWrappedObject, QAbstractSpinBox::StepEnabled  stepEnabled){ theWrappedObject->stepEnabled = stepEnabled; }
QAbstractSpinBox::StepEnabled  py_get_stepEnabled(QStyleOptionSpinBox* theWrappedObject){ return theWrappedObject->stepEnabled; }
void py_set_buttonSymbols(QStyleOptionSpinBox* theWrappedObject, QAbstractSpinBox::ButtonSymbols  buttonSymbols){ theWrappedObject->buttonSymbols = buttonSymbols; }
QAbstractSpinBox::ButtonSymbols  py_get_buttonSymbols(QStyleOptionSpinBox* theWrappedObject){ return theWrappedObject->buttonSymbols; }
void py_set_frame(QStyleOptionSpinBox* theWrappedObject, bool  frame){ theWrappedObject->frame = frame; }
bool  py_get_frame(QStyleOptionSpinBox* theWrappedObject){ return theWrappedObject->frame; }
};


