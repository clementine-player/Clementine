#include <PythonQt.h>
#include <QIcon>
#include <QMessageBox>
#include <QObject>
#include <QVariant>
#include <qabstractbutton.h>
#include <qabstractitemdelegate.h>
#include <qabstractitemmodel.h>
#include <qaction.h>
#include <qactiongroup.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qboxlayout.h>
#include <qbrush.h>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qcursor.h>
#include <qdatastream.h>
#include <qevent.h>
#include <qfont.h>
#include <qgraphicseffect.h>
#include <qgraphicsproxywidget.h>
#include <qgraphicswidget.h>
#include <qheaderview.h>
#include <qicon.h>
#include <qinputcontext.h>
#include <qitemselectionmodel.h>
#include <qkeysequence.h>
#include <qlayout.h>
#include <qlayoutitem.h>
#include <qline.h>
#include <qlist.h>
#include <qlocale.h>
#include <qmargins.h>
#include <qmatrix.h>
#include <qmatrix4x4.h>
#include <qmenu.h>
#include <qmimedata.h>
#include <qobject.h>
#include <qpaintdevice.h>
#include <qpaintengine.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qpalette.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpolygon.h>
#include <qrect.h>
#include <qregion.h>
#include <qscrollbar.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qstringlist.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtooltip.h>
#include <qtransform.h>
#include <qtreeview.h>
#include <qtreewidget.h>
#include <qundogroup.h>
#include <qundostack.h>
#include <qundoview.h>
#include <qvalidator.h>
#include <qvector2d.h>
#include <qvector3d.h>
#include <qvector4d.h>
#include <qwhatsthis.h>
#include <qwidget.h>
#include <qwidgetaction.h>
#include <qwindowsstyle.h>
#include <qwizard.h>
#include <qworkspace.h>



class PythonQtWrapper_QToolTip : public QObject
{ Q_OBJECT
public:
public slots:
void delete_QToolTip(QToolTip* obj) { delete obj; } 
   QFont  static_QToolTip_font();
   void static_QToolTip_hideText();
   bool  static_QToolTip_isVisible();
   QPalette  static_QToolTip_palette();
   void static_QToolTip_setFont(const QFont&  arg__1);
   void static_QToolTip_setPalette(const QPalette&  arg__1);
   void static_QToolTip_showText(const QPoint&  pos, const QString&  text, QWidget*  w = 0);
   void static_QToolTip_showText(const QPoint&  pos, const QString&  text, QWidget*  w, const QRect&  rect);
   QString  static_QToolTip_text();
};





class PythonQtShell_QTouchEvent : public QTouchEvent
{
public:
    PythonQtShell_QTouchEvent(QEvent::Type  eventType, QTouchEvent::DeviceType  deviceType = QTouchEvent::TouchScreen, Qt::KeyboardModifiers  modifiers = Qt::NoModifier, Qt::TouchPointStates  touchPointStates = 0, const QList<QTouchEvent::TouchPoint >&  touchPoints = QList<QTouchEvent::TouchPoint>()):QTouchEvent(eventType, deviceType, modifiers, touchPointStates, touchPoints),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QTouchEvent : public QObject
{ Q_OBJECT
public:
Q_ENUMS(DeviceType )
enum DeviceType{
  TouchScreen = QTouchEvent::TouchScreen,   TouchPad = QTouchEvent::TouchPad};
public slots:
QTouchEvent* new_QTouchEvent(QEvent::Type  eventType, QTouchEvent::DeviceType  deviceType = QTouchEvent::TouchScreen, Qt::KeyboardModifiers  modifiers = Qt::NoModifier, Qt::TouchPointStates  touchPointStates = 0, const QList<QTouchEvent::TouchPoint >&  touchPoints = QList<QTouchEvent::TouchPoint>());
void delete_QTouchEvent(QTouchEvent* obj) { delete obj; } 
   QTouchEvent::DeviceType  deviceType(QTouchEvent* theWrappedObject) const;
   void setDeviceType(QTouchEvent* theWrappedObject, QTouchEvent::DeviceType  adeviceType);
   void setTouchPointStates(QTouchEvent* theWrappedObject, Qt::TouchPointStates  aTouchPointStates);
   void setTouchPoints(QTouchEvent* theWrappedObject, const QList<QTouchEvent::TouchPoint >&  atouchPoints);
   void setWidget(QTouchEvent* theWrappedObject, QWidget*  awidget);
   Qt::TouchPointStates  touchPointStates(QTouchEvent* theWrappedObject) const;
   const QList<QTouchEvent::TouchPoint >*  touchPoints(QTouchEvent* theWrappedObject) const;
   QWidget*  widget(QTouchEvent* theWrappedObject) const;
};





class PythonQtWrapper_QTouchEvent_TouchPoint : public QObject
{ Q_OBJECT
public:
public slots:
QTouchEvent::TouchPoint* new_QTouchEvent_TouchPoint(const QTouchEvent::TouchPoint&  other);
QTouchEvent::TouchPoint* new_QTouchEvent_TouchPoint(int  id = -1);
void delete_QTouchEvent_TouchPoint(QTouchEvent::TouchPoint* obj) { delete obj; } 
   int  id(QTouchEvent::TouchPoint* theWrappedObject) const;
   bool  isPrimary(QTouchEvent::TouchPoint* theWrappedObject) const;
   QPointF  lastNormalizedPos(QTouchEvent::TouchPoint* theWrappedObject) const;
   QPointF  lastPos(QTouchEvent::TouchPoint* theWrappedObject) const;
   QPointF  lastScenePos(QTouchEvent::TouchPoint* theWrappedObject) const;
   QPointF  lastScreenPos(QTouchEvent::TouchPoint* theWrappedObject) const;
   QPointF  normalizedPos(QTouchEvent::TouchPoint* theWrappedObject) const;
   QTouchEvent::TouchPoint*  operator_assign(QTouchEvent::TouchPoint* theWrappedObject, const QTouchEvent::TouchPoint&  other);
   QPointF  pos(QTouchEvent::TouchPoint* theWrappedObject) const;
   qreal  pressure(QTouchEvent::TouchPoint* theWrappedObject) const;
   QRectF  rect(QTouchEvent::TouchPoint* theWrappedObject) const;
   QPointF  scenePos(QTouchEvent::TouchPoint* theWrappedObject) const;
   QRectF  sceneRect(QTouchEvent::TouchPoint* theWrappedObject) const;
   QPointF  screenPos(QTouchEvent::TouchPoint* theWrappedObject) const;
   QRectF  screenRect(QTouchEvent::TouchPoint* theWrappedObject) const;
   void setId(QTouchEvent::TouchPoint* theWrappedObject, int  id);
   void setLastNormalizedPos(QTouchEvent::TouchPoint* theWrappedObject, const QPointF&  lastNormalizedPos);
   void setLastPos(QTouchEvent::TouchPoint* theWrappedObject, const QPointF&  lastPos);
   void setLastScenePos(QTouchEvent::TouchPoint* theWrappedObject, const QPointF&  lastScenePos);
   void setLastScreenPos(QTouchEvent::TouchPoint* theWrappedObject, const QPointF&  lastScreenPos);
   void setNormalizedPos(QTouchEvent::TouchPoint* theWrappedObject, const QPointF&  normalizedPos);
   void setPos(QTouchEvent::TouchPoint* theWrappedObject, const QPointF&  pos);
   void setPressure(QTouchEvent::TouchPoint* theWrappedObject, qreal  pressure);
   void setRect(QTouchEvent::TouchPoint* theWrappedObject, const QRectF&  rect);
   void setScenePos(QTouchEvent::TouchPoint* theWrappedObject, const QPointF&  scenePos);
   void setSceneRect(QTouchEvent::TouchPoint* theWrappedObject, const QRectF&  sceneRect);
   void setScreenPos(QTouchEvent::TouchPoint* theWrappedObject, const QPointF&  screenPos);
   void setScreenRect(QTouchEvent::TouchPoint* theWrappedObject, const QRectF&  screenRect);
   void setStartNormalizedPos(QTouchEvent::TouchPoint* theWrappedObject, const QPointF&  startNormalizedPos);
   void setStartPos(QTouchEvent::TouchPoint* theWrappedObject, const QPointF&  startPos);
   void setStartScenePos(QTouchEvent::TouchPoint* theWrappedObject, const QPointF&  startScenePos);
   void setStartScreenPos(QTouchEvent::TouchPoint* theWrappedObject, const QPointF&  startScreenPos);
   void setState(QTouchEvent::TouchPoint* theWrappedObject, Qt::TouchPointStates  state);
   QPointF  startNormalizedPos(QTouchEvent::TouchPoint* theWrappedObject) const;
   QPointF  startPos(QTouchEvent::TouchPoint* theWrappedObject) const;
   QPointF  startScenePos(QTouchEvent::TouchPoint* theWrappedObject) const;
   QPointF  startScreenPos(QTouchEvent::TouchPoint* theWrappedObject) const;
   Qt::TouchPointState  state(QTouchEvent::TouchPoint* theWrappedObject) const;
};





class PythonQtWrapper_QTransform : public QObject
{ Q_OBJECT
public:
Q_ENUMS(TransformationType )
enum TransformationType{
  TxNone = QTransform::TxNone,   TxTranslate = QTransform::TxTranslate,   TxScale = QTransform::TxScale,   TxRotate = QTransform::TxRotate,   TxShear = QTransform::TxShear,   TxProject = QTransform::TxProject};
public slots:
QTransform* new_QTransform();
QTransform* new_QTransform(const QMatrix&  mtx);
QTransform* new_QTransform(qreal  h11, qreal  h12, qreal  h13, qreal  h21, qreal  h22, qreal  h23, qreal  h31, qreal  h32, qreal  h33 = 1.0);
QTransform* new_QTransform(qreal  h11, qreal  h12, qreal  h21, qreal  h22, qreal  dx, qreal  dy);
QTransform* new_QTransform(const QTransform& other) {
QTransform* a = new QTransform();
*((QTransform*)a) = other;
return a; }
void delete_QTransform(QTransform* obj) { delete obj; } 
   QTransform  adjoint(QTransform* theWrappedObject) const;
   qreal  det(QTransform* theWrappedObject) const;
   qreal  determinant(QTransform* theWrappedObject) const;
   qreal  dx(QTransform* theWrappedObject) const;
   qreal  dy(QTransform* theWrappedObject) const;
   QTransform  static_QTransform_fromScale(qreal  dx, qreal  dy);
   QTransform  static_QTransform_fromTranslate(qreal  dx, qreal  dy);
   QTransform  inverted(QTransform* theWrappedObject, bool*  invertible = 0) const;
   bool  isAffine(QTransform* theWrappedObject) const;
   bool  isIdentity(QTransform* theWrappedObject) const;
   bool  isInvertible(QTransform* theWrappedObject) const;
   bool  isRotating(QTransform* theWrappedObject) const;
   bool  isScaling(QTransform* theWrappedObject) const;
   bool  isTranslating(QTransform* theWrappedObject) const;
   qreal  m11(QTransform* theWrappedObject) const;
   qreal  m12(QTransform* theWrappedObject) const;
   qreal  m13(QTransform* theWrappedObject) const;
   qreal  m21(QTransform* theWrappedObject) const;
   qreal  m22(QTransform* theWrappedObject) const;
   qreal  m23(QTransform* theWrappedObject) const;
   qreal  m31(QTransform* theWrappedObject) const;
   qreal  m32(QTransform* theWrappedObject) const;
   qreal  m33(QTransform* theWrappedObject) const;
   QLine  map(QTransform* theWrappedObject, const QLine&  l) const;
   QLineF  map(QTransform* theWrappedObject, const QLineF&  l) const;
   QPainterPath  map(QTransform* theWrappedObject, const QPainterPath&  p) const;
   QPoint  map(QTransform* theWrappedObject, const QPoint&  p) const;
   QPointF  map(QTransform* theWrappedObject, const QPointF&  p) const;
   QPolygon  map(QTransform* theWrappedObject, const QPolygon&  a) const;
   QPolygonF  map(QTransform* theWrappedObject, const QPolygonF&  a) const;
   QRegion  map(QTransform* theWrappedObject, const QRegion&  r) const;
   void map(QTransform* theWrappedObject, qreal  x, qreal  y, qreal*  tx, qreal*  ty) const;
   QRect  mapRect(QTransform* theWrappedObject, const QRect&  arg__1) const;
   QRectF  mapRect(QTransform* theWrappedObject, const QRectF&  arg__1) const;
   QPolygon  mapToPolygon(QTransform* theWrappedObject, const QRect&  r) const;
   bool  __ne__(QTransform* theWrappedObject, const QTransform&  arg__1) const;
   QTransform  multiplied(QTransform* theWrappedObject, const QTransform&  o) const;
   QTransform  __mul__(QTransform* theWrappedObject, qreal  n);
   QTransform*  __imul__(QTransform* theWrappedObject, const QTransform&  arg__1);
   QTransform*  __imul__(QTransform* theWrappedObject, qreal  div);
   QTransform  __add__(QTransform* theWrappedObject, qreal  n);
   QTransform*  __iadd__(QTransform* theWrappedObject, qreal  div);
   QTransform  __sub__(QTransform* theWrappedObject, qreal  n);
   QTransform*  __isub__(QTransform* theWrappedObject, qreal  div);
   QTransform  __div__(QTransform* theWrappedObject, qreal  n);
   QTransform*  __idiv__(QTransform* theWrappedObject, qreal  div);
   void writeTo(QTransform* theWrappedObject, QDataStream&  arg__1);
   bool  __eq__(QTransform* theWrappedObject, const QTransform&  arg__1) const;
   void readFrom(QTransform* theWrappedObject, QDataStream&  arg__1);
   bool  static_QTransform_quadToQuad(const QPolygonF&  one, const QPolygonF&  two, QTransform&  result);
   bool  static_QTransform_quadToSquare(const QPolygonF&  quad, QTransform&  result);
   void reset(QTransform* theWrappedObject);
   QTransform*  rotate(QTransform* theWrappedObject, qreal  a, Qt::Axis  axis = Qt::ZAxis);
   QTransform*  rotateRadians(QTransform* theWrappedObject, qreal  a, Qt::Axis  axis = Qt::ZAxis);
   QTransform*  scale(QTransform* theWrappedObject, qreal  sx, qreal  sy);
   void setMatrix(QTransform* theWrappedObject, qreal  m11, qreal  m12, qreal  m13, qreal  m21, qreal  m22, qreal  m23, qreal  m31, qreal  m32, qreal  m33);
   QTransform*  shear(QTransform* theWrappedObject, qreal  sh, qreal  sv);
   bool  static_QTransform_squareToQuad(const QPolygonF&  square, QTransform&  result);
   const QMatrix*  toAffine(QTransform* theWrappedObject) const;
   QTransform*  translate(QTransform* theWrappedObject, qreal  dx, qreal  dy);
   QTransform  transposed(QTransform* theWrappedObject) const;
   QTransform::TransformationType  type(QTransform* theWrappedObject) const;
    QString py_toString(QTransform*);
};





class PythonQtShell_QTreeView : public QTreeView
{
public:
    PythonQtShell_QTreeView(QWidget*  parent = 0):QTreeView(parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  arg__1);
virtual void childEvent(QChildEvent*  arg__1);
virtual void closeEditor(QWidget*  editor, QAbstractItemDelegate::EndEditHint  hint);
virtual void closeEvent(QCloseEvent*  arg__1);
virtual void commitData(QWidget*  editor);
virtual void contextMenuEvent(QContextMenuEvent*  arg__1);
virtual void currentChanged(const QModelIndex&  current, const QModelIndex&  previous);
virtual void customEvent(QEvent*  arg__1);
virtual void dataChanged(const QModelIndex&  topLeft, const QModelIndex&  bottomRight);
virtual int  devType() const;
virtual void doItemsLayout();
virtual void dragEnterEvent(QDragEnterEvent*  event);
virtual void dragLeaveEvent(QDragLeaveEvent*  event);
virtual void dragMoveEvent(QDragMoveEvent*  event);
virtual void drawBranches(QPainter*  painter, const QRect&  rect, const QModelIndex&  index) const;
virtual void drawRow(QPainter*  painter, const QStyleOptionViewItem&  options, const QModelIndex&  index) const;
virtual void dropEvent(QDropEvent*  event);
virtual bool  edit(const QModelIndex&  index, QAbstractItemView::EditTrigger  trigger, QEvent*  event);
virtual void editorDestroyed(QObject*  editor);
virtual void enterEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  event);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void focusInEvent(QFocusEvent*  event);
virtual bool  focusNextPrevChild(bool  next);
virtual void focusOutEvent(QFocusEvent*  event);
virtual int  heightForWidth(int  arg__1) const;
virtual void hideEvent(QHideEvent*  arg__1);
virtual int  horizontalOffset() const;
virtual void horizontalScrollbarAction(int  action);
virtual void horizontalScrollbarValueChanged(int  value);
virtual QModelIndex  indexAt(const QPoint&  p) const;
virtual void inputMethodEvent(QInputMethodEvent*  event);
virtual QVariant  inputMethodQuery(Qt::InputMethodQuery  query) const;
virtual bool  isIndexHidden(const QModelIndex&  index) const;
virtual void keyPressEvent(QKeyEvent*  event);
virtual void keyReleaseEvent(QKeyEvent*  arg__1);
virtual void keyboardSearch(const QString&  search);
virtual void languageChange();
virtual void leaveEvent(QEvent*  arg__1);
virtual int  metric(QPaintDevice::PaintDeviceMetric  arg__1) const;
virtual void mouseDoubleClickEvent(QMouseEvent*  event);
virtual void mouseMoveEvent(QMouseEvent*  event);
virtual void mousePressEvent(QMouseEvent*  event);
virtual void mouseReleaseEvent(QMouseEvent*  event);
virtual void moveEvent(QMoveEvent*  arg__1);
virtual QPaintEngine*  paintEngine() const;
virtual void paintEvent(QPaintEvent*  event);
virtual void reset();
virtual void resizeEvent(QResizeEvent*  event);
virtual void rowsAboutToBeRemoved(const QModelIndex&  parent, int  start, int  end);
virtual void rowsInserted(const QModelIndex&  parent, int  start, int  end);
virtual void scrollContentsBy(int  dx, int  dy);
virtual void scrollTo(const QModelIndex&  index, QAbstractItemView::ScrollHint  hint = QAbstractItemView::EnsureVisible);
virtual void selectAll();
virtual QList<QModelIndex >  selectedIndexes() const;
virtual void selectionChanged(const QItemSelection&  selected, const QItemSelection&  deselected);
virtual QItemSelectionModel::SelectionFlags  selectionCommand(const QModelIndex&  index, const QEvent*  event) const;
virtual void setModel(QAbstractItemModel*  model);
virtual void setRootIndex(const QModelIndex&  index);
virtual void setSelection(const QRect&  rect, QItemSelectionModel::SelectionFlags  command);
virtual void setSelectionModel(QItemSelectionModel*  selectionModel);
virtual void showEvent(QShowEvent*  arg__1);
virtual int  sizeHintForColumn(int  column) const;
virtual int  sizeHintForRow(int  row) const;
virtual void startDrag(Qt::DropActions  supportedActions);
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  event);
virtual void updateEditorData();
virtual void updateEditorGeometries();
virtual void updateGeometries();
virtual int  verticalOffset() const;
virtual void verticalScrollbarAction(int  action);
virtual void verticalScrollbarValueChanged(int  value);
virtual QStyleOptionViewItem  viewOptions() const;
virtual bool  viewportEvent(QEvent*  event);
virtual QRect  visualRect(const QModelIndex&  index) const;
virtual QRegion  visualRegionForSelection(const QItemSelection&  selection) const;
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QTreeView : public QTreeView
{ public:
inline void promoted_currentChanged(const QModelIndex&  current, const QModelIndex&  previous) { QTreeView::currentChanged(current, previous); }
inline void promoted_dataChanged(const QModelIndex&  topLeft, const QModelIndex&  bottomRight) { QTreeView::dataChanged(topLeft, bottomRight); }
inline void promoted_doItemsLayout() { QTreeView::doItemsLayout(); }
inline void promoted_dragMoveEvent(QDragMoveEvent*  event) { QTreeView::dragMoveEvent(event); }
inline void promoted_drawBranches(QPainter*  painter, const QRect&  rect, const QModelIndex&  index) const { QTreeView::drawBranches(painter, rect, index); }
inline void promoted_drawRow(QPainter*  painter, const QStyleOptionViewItem&  options, const QModelIndex&  index) const { QTreeView::drawRow(painter, options, index); }
inline int  promoted_horizontalOffset() const { return QTreeView::horizontalOffset(); }
inline void promoted_horizontalScrollbarAction(int  action) { QTreeView::horizontalScrollbarAction(action); }
inline QModelIndex  promoted_indexAt(const QPoint&  p) const { return QTreeView::indexAt(p); }
inline bool  promoted_isIndexHidden(const QModelIndex&  index) const { return QTreeView::isIndexHidden(index); }
inline void promoted_keyPressEvent(QKeyEvent*  event) { QTreeView::keyPressEvent(event); }
inline void promoted_keyboardSearch(const QString&  search) { QTreeView::keyboardSearch(search); }
inline void promoted_mouseDoubleClickEvent(QMouseEvent*  event) { QTreeView::mouseDoubleClickEvent(event); }
inline void promoted_mouseMoveEvent(QMouseEvent*  event) { QTreeView::mouseMoveEvent(event); }
inline void promoted_mousePressEvent(QMouseEvent*  event) { QTreeView::mousePressEvent(event); }
inline void promoted_mouseReleaseEvent(QMouseEvent*  event) { QTreeView::mouseReleaseEvent(event); }
inline void promoted_paintEvent(QPaintEvent*  event) { QTreeView::paintEvent(event); }
inline void promoted_reset() { QTreeView::reset(); }
inline void promoted_rowsAboutToBeRemoved(const QModelIndex&  parent, int  start, int  end) { QTreeView::rowsAboutToBeRemoved(parent, start, end); }
inline void promoted_rowsInserted(const QModelIndex&  parent, int  start, int  end) { QTreeView::rowsInserted(parent, start, end); }
inline void promoted_scrollContentsBy(int  dx, int  dy) { QTreeView::scrollContentsBy(dx, dy); }
inline void promoted_scrollTo(const QModelIndex&  index, QAbstractItemView::ScrollHint  hint = QAbstractItemView::EnsureVisible) { QTreeView::scrollTo(index, hint); }
inline void promoted_selectAll() { QTreeView::selectAll(); }
inline QList<QModelIndex >  promoted_selectedIndexes() const { return QTreeView::selectedIndexes(); }
inline void promoted_selectionChanged(const QItemSelection&  selected, const QItemSelection&  deselected) { QTreeView::selectionChanged(selected, deselected); }
inline void promoted_setModel(QAbstractItemModel*  model) { QTreeView::setModel(model); }
inline void promoted_setRootIndex(const QModelIndex&  index) { QTreeView::setRootIndex(index); }
inline void promoted_setSelection(const QRect&  rect, QItemSelectionModel::SelectionFlags  command) { QTreeView::setSelection(rect, command); }
inline void promoted_setSelectionModel(QItemSelectionModel*  selectionModel) { QTreeView::setSelectionModel(selectionModel); }
inline int  promoted_sizeHintForColumn(int  column) const { return QTreeView::sizeHintForColumn(column); }
inline void promoted_timerEvent(QTimerEvent*  event) { QTreeView::timerEvent(event); }
inline void promoted_updateGeometries() { QTreeView::updateGeometries(); }
inline int  promoted_verticalOffset() const { return QTreeView::verticalOffset(); }
inline bool  promoted_viewportEvent(QEvent*  event) { return QTreeView::viewportEvent(event); }
inline QRect  promoted_visualRect(const QModelIndex&  index) const { return QTreeView::visualRect(index); }
inline QRegion  promoted_visualRegionForSelection(const QItemSelection&  selection) const { return QTreeView::visualRegionForSelection(selection); }
};

class PythonQtWrapper_QTreeView : public QObject
{ Q_OBJECT
public:
public slots:
QTreeView* new_QTreeView(QWidget*  parent = 0);
void delete_QTreeView(QTreeView* obj) { delete obj; } 
   bool  allColumnsShowFocus(QTreeView* theWrappedObject) const;
   int  autoExpandDelay(QTreeView* theWrappedObject) const;
   int  columnAt(QTreeView* theWrappedObject, int  x) const;
   int  columnViewportPosition(QTreeView* theWrappedObject, int  column) const;
   int  columnWidth(QTreeView* theWrappedObject, int  column) const;
   void currentChanged(QTreeView* theWrappedObject, const QModelIndex&  current, const QModelIndex&  previous);
   void dataChanged(QTreeView* theWrappedObject, const QModelIndex&  topLeft, const QModelIndex&  bottomRight);
   void doItemsLayout(QTreeView* theWrappedObject);
   void dragMoveEvent(QTreeView* theWrappedObject, QDragMoveEvent*  event);
   void drawBranches(QTreeView* theWrappedObject, QPainter*  painter, const QRect&  rect, const QModelIndex&  index) const;
   void drawRow(QTreeView* theWrappedObject, QPainter*  painter, const QStyleOptionViewItem&  options, const QModelIndex&  index) const;
   bool  expandsOnDoubleClick(QTreeView* theWrappedObject) const;
   QHeaderView*  header(QTreeView* theWrappedObject) const;
   int  horizontalOffset(QTreeView* theWrappedObject) const;
   void horizontalScrollbarAction(QTreeView* theWrappedObject, int  action);
   int  indentation(QTreeView* theWrappedObject) const;
   QModelIndex  indexAbove(QTreeView* theWrappedObject, const QModelIndex&  index) const;
   QModelIndex  indexAt(QTreeView* theWrappedObject, const QPoint&  p) const;
   QModelIndex  indexBelow(QTreeView* theWrappedObject, const QModelIndex&  index) const;
   bool  isAnimated(QTreeView* theWrappedObject) const;
   bool  isColumnHidden(QTreeView* theWrappedObject, int  column) const;
   bool  isExpanded(QTreeView* theWrappedObject, const QModelIndex&  index) const;
   bool  isFirstColumnSpanned(QTreeView* theWrappedObject, int  row, const QModelIndex&  parent) const;
   bool  isHeaderHidden(QTreeView* theWrappedObject) const;
   bool  isIndexHidden(QTreeView* theWrappedObject, const QModelIndex&  index) const;
   bool  isRowHidden(QTreeView* theWrappedObject, int  row, const QModelIndex&  parent) const;
   bool  isSortingEnabled(QTreeView* theWrappedObject) const;
   bool  itemsExpandable(QTreeView* theWrappedObject) const;
   void keyPressEvent(QTreeView* theWrappedObject, QKeyEvent*  event);
   void keyboardSearch(QTreeView* theWrappedObject, const QString&  search);
   void mouseDoubleClickEvent(QTreeView* theWrappedObject, QMouseEvent*  event);
   void mouseMoveEvent(QTreeView* theWrappedObject, QMouseEvent*  event);
   void mousePressEvent(QTreeView* theWrappedObject, QMouseEvent*  event);
   void mouseReleaseEvent(QTreeView* theWrappedObject, QMouseEvent*  event);
   void paintEvent(QTreeView* theWrappedObject, QPaintEvent*  event);
   void reset(QTreeView* theWrappedObject);
   bool  rootIsDecorated(QTreeView* theWrappedObject) const;
   void rowsAboutToBeRemoved(QTreeView* theWrappedObject, const QModelIndex&  parent, int  start, int  end);
   void rowsInserted(QTreeView* theWrappedObject, const QModelIndex&  parent, int  start, int  end);
   void scrollContentsBy(QTreeView* theWrappedObject, int  dx, int  dy);
   void scrollTo(QTreeView* theWrappedObject, const QModelIndex&  index, QAbstractItemView::ScrollHint  hint = QAbstractItemView::EnsureVisible);
   void selectAll(QTreeView* theWrappedObject);
   QList<QModelIndex >  selectedIndexes(QTreeView* theWrappedObject) const;
   void selectionChanged(QTreeView* theWrappedObject, const QItemSelection&  selected, const QItemSelection&  deselected);
   void setAllColumnsShowFocus(QTreeView* theWrappedObject, bool  enable);
   void setAnimated(QTreeView* theWrappedObject, bool  enable);
   void setAutoExpandDelay(QTreeView* theWrappedObject, int  delay);
   void setColumnHidden(QTreeView* theWrappedObject, int  column, bool  hide);
   void setColumnWidth(QTreeView* theWrappedObject, int  column, int  width);
   void setExpanded(QTreeView* theWrappedObject, const QModelIndex&  index, bool  expand);
   void setExpandsOnDoubleClick(QTreeView* theWrappedObject, bool  enable);
   void setFirstColumnSpanned(QTreeView* theWrappedObject, int  row, const QModelIndex&  parent, bool  span);
   void setHeader(QTreeView* theWrappedObject, QHeaderView*  header);
   void setHeaderHidden(QTreeView* theWrappedObject, bool  hide);
   void setIndentation(QTreeView* theWrappedObject, int  i);
   void setItemsExpandable(QTreeView* theWrappedObject, bool  enable);
   void setModel(QTreeView* theWrappedObject, QAbstractItemModel*  model);
   void setRootIndex(QTreeView* theWrappedObject, const QModelIndex&  index);
   void setRootIsDecorated(QTreeView* theWrappedObject, bool  show);
   void setRowHidden(QTreeView* theWrappedObject, int  row, const QModelIndex&  parent, bool  hide);
   void setSelection(QTreeView* theWrappedObject, const QRect&  rect, QItemSelectionModel::SelectionFlags  command);
   void setSelectionModel(QTreeView* theWrappedObject, QItemSelectionModel*  selectionModel);
   void setSortingEnabled(QTreeView* theWrappedObject, bool  enable);
   void setUniformRowHeights(QTreeView* theWrappedObject, bool  uniform);
   void setWordWrap(QTreeView* theWrappedObject, bool  on);
   int  sizeHintForColumn(QTreeView* theWrappedObject, int  column) const;
   void sortByColumn(QTreeView* theWrappedObject, int  column, Qt::SortOrder  order);
   void timerEvent(QTreeView* theWrappedObject, QTimerEvent*  event);
   bool  uniformRowHeights(QTreeView* theWrappedObject) const;
   void updateGeometries(QTreeView* theWrappedObject);
   int  verticalOffset(QTreeView* theWrappedObject) const;
   bool  viewportEvent(QTreeView* theWrappedObject, QEvent*  event);
   QRect  visualRect(QTreeView* theWrappedObject, const QModelIndex&  index) const;
   QRegion  visualRegionForSelection(QTreeView* theWrappedObject, const QItemSelection&  selection) const;
   bool  wordWrap(QTreeView* theWrappedObject) const;
};





class PythonQtShell_QTreeWidget : public QTreeWidget
{
public:
    PythonQtShell_QTreeWidget(QWidget*  parent = 0):QTreeWidget(parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  arg__1);
virtual void childEvent(QChildEvent*  arg__1);
virtual void closeEditor(QWidget*  editor, QAbstractItemDelegate::EndEditHint  hint);
virtual void closeEvent(QCloseEvent*  arg__1);
virtual void commitData(QWidget*  editor);
virtual void contextMenuEvent(QContextMenuEvent*  arg__1);
virtual void currentChanged(const QModelIndex&  current, const QModelIndex&  previous);
virtual void customEvent(QEvent*  arg__1);
virtual void dataChanged(const QModelIndex&  topLeft, const QModelIndex&  bottomRight);
virtual int  devType() const;
virtual void doItemsLayout();
virtual void dragEnterEvent(QDragEnterEvent*  event);
virtual void dragLeaveEvent(QDragLeaveEvent*  event);
virtual void dragMoveEvent(QDragMoveEvent*  event);
virtual void drawBranches(QPainter*  painter, const QRect&  rect, const QModelIndex&  index) const;
virtual void drawRow(QPainter*  painter, const QStyleOptionViewItem&  options, const QModelIndex&  index) const;
virtual void dropEvent(QDropEvent*  event);
virtual bool  dropMimeData(QTreeWidgetItem*  parent, int  index, const QMimeData*  data, Qt::DropAction  action);
virtual bool  edit(const QModelIndex&  index, QAbstractItemView::EditTrigger  trigger, QEvent*  event);
virtual void editorDestroyed(QObject*  editor);
virtual void enterEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  e);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void focusInEvent(QFocusEvent*  event);
virtual bool  focusNextPrevChild(bool  next);
virtual void focusOutEvent(QFocusEvent*  event);
virtual int  heightForWidth(int  arg__1) const;
virtual void hideEvent(QHideEvent*  arg__1);
virtual int  horizontalOffset() const;
virtual void horizontalScrollbarAction(int  action);
virtual void horizontalScrollbarValueChanged(int  value);
virtual QModelIndex  indexAt(const QPoint&  p) const;
virtual void inputMethodEvent(QInputMethodEvent*  event);
virtual QVariant  inputMethodQuery(Qt::InputMethodQuery  query) const;
virtual bool  isIndexHidden(const QModelIndex&  index) const;
virtual void keyPressEvent(QKeyEvent*  event);
virtual void keyReleaseEvent(QKeyEvent*  arg__1);
virtual void keyboardSearch(const QString&  search);
virtual void languageChange();
virtual void leaveEvent(QEvent*  arg__1);
virtual int  metric(QPaintDevice::PaintDeviceMetric  arg__1) const;
virtual QMimeData*  mimeData(const QList<QTreeWidgetItem* >  items) const;
virtual QStringList  mimeTypes() const;
virtual void mouseDoubleClickEvent(QMouseEvent*  event);
virtual void mouseMoveEvent(QMouseEvent*  event);
virtual void mousePressEvent(QMouseEvent*  event);
virtual void mouseReleaseEvent(QMouseEvent*  event);
virtual void moveEvent(QMoveEvent*  arg__1);
virtual QPaintEngine*  paintEngine() const;
virtual void paintEvent(QPaintEvent*  event);
virtual void reset();
virtual void resizeEvent(QResizeEvent*  event);
virtual void rowsAboutToBeRemoved(const QModelIndex&  parent, int  start, int  end);
virtual void rowsInserted(const QModelIndex&  parent, int  start, int  end);
virtual void scrollContentsBy(int  dx, int  dy);
virtual void scrollTo(const QModelIndex&  index, QAbstractItemView::ScrollHint  hint);
virtual void selectAll();
virtual QList<QModelIndex >  selectedIndexes() const;
virtual void selectionChanged(const QItemSelection&  selected, const QItemSelection&  deselected);
virtual QItemSelectionModel::SelectionFlags  selectionCommand(const QModelIndex&  index, const QEvent*  event) const;
virtual void setRootIndex(const QModelIndex&  index);
virtual void setSelection(const QRect&  rect, QItemSelectionModel::SelectionFlags  command);
virtual void setSelectionModel(QItemSelectionModel*  selectionModel);
virtual void showEvent(QShowEvent*  arg__1);
virtual int  sizeHintForColumn(int  column) const;
virtual int  sizeHintForRow(int  row) const;
virtual void startDrag(Qt::DropActions  supportedActions);
virtual Qt::DropActions  supportedDropActions() const;
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  event);
virtual void updateEditorData();
virtual void updateEditorGeometries();
virtual void updateGeometries();
virtual int  verticalOffset() const;
virtual void verticalScrollbarAction(int  action);
virtual void verticalScrollbarValueChanged(int  value);
virtual QStyleOptionViewItem  viewOptions() const;
virtual bool  viewportEvent(QEvent*  event);
virtual QRect  visualRect(const QModelIndex&  index) const;
virtual QRegion  visualRegionForSelection(const QItemSelection&  selection) const;
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QTreeWidget : public QTreeWidget
{ public:
inline void promoted_dropEvent(QDropEvent*  event) { QTreeWidget::dropEvent(event); }
inline bool  promoted_dropMimeData(QTreeWidgetItem*  parent, int  index, const QMimeData*  data, Qt::DropAction  action) { return QTreeWidget::dropMimeData(parent, index, data, action); }
inline bool  promoted_event(QEvent*  e) { return QTreeWidget::event(e); }
inline QStringList  promoted_mimeTypes() const { return QTreeWidget::mimeTypes(); }
inline void promoted_setSelectionModel(QItemSelectionModel*  selectionModel) { QTreeWidget::setSelectionModel(selectionModel); }
inline Qt::DropActions  promoted_supportedDropActions() const { return QTreeWidget::supportedDropActions(); }
};

class PythonQtWrapper_QTreeWidget : public QObject
{ Q_OBJECT
public:
public slots:
QTreeWidget* new_QTreeWidget(QWidget*  parent = 0);
void delete_QTreeWidget(QTreeWidget* obj) { delete obj; } 
   void addTopLevelItem(QTreeWidget* theWrappedObject, QTreeWidgetItem*  item);
   void addTopLevelItems(QTreeWidget* theWrappedObject, const QList<QTreeWidgetItem* >&  items);
   void closePersistentEditor(QTreeWidget* theWrappedObject, QTreeWidgetItem*  item, int  column = 0);
   int  columnCount(QTreeWidget* theWrappedObject) const;
   int  currentColumn(QTreeWidget* theWrappedObject) const;
   QTreeWidgetItem*  currentItem(QTreeWidget* theWrappedObject) const;
   void dropEvent(QTreeWidget* theWrappedObject, QDropEvent*  event);
   bool  dropMimeData(QTreeWidget* theWrappedObject, QTreeWidgetItem*  parent, int  index, const QMimeData*  data, Qt::DropAction  action);
   void editItem(QTreeWidget* theWrappedObject, QTreeWidgetItem*  item, int  column = 0);
   bool  event(QTreeWidget* theWrappedObject, QEvent*  e);
   QList<QTreeWidgetItem* >  findItems(QTreeWidget* theWrappedObject, const QString&  text, Qt::MatchFlags  flags, int  column = 0) const;
   QTreeWidgetItem*  headerItem(QTreeWidget* theWrappedObject) const;
   int  indexOfTopLevelItem(QTreeWidget* theWrappedObject, QTreeWidgetItem*  item) const;
   void insertTopLevelItem(QTreeWidget* theWrappedObject, int  index, QTreeWidgetItem*  item);
   void insertTopLevelItems(QTreeWidget* theWrappedObject, int  index, const QList<QTreeWidgetItem* >&  items);
   QTreeWidgetItem*  invisibleRootItem(QTreeWidget* theWrappedObject) const;
   bool  isFirstItemColumnSpanned(QTreeWidget* theWrappedObject, const QTreeWidgetItem*  item) const;
   QTreeWidgetItem*  itemAbove(QTreeWidget* theWrappedObject, const QTreeWidgetItem*  item) const;
   QTreeWidgetItem*  itemAt(QTreeWidget* theWrappedObject, const QPoint&  p) const;
   QTreeWidgetItem*  itemAt(QTreeWidget* theWrappedObject, int  x, int  y) const;
   QTreeWidgetItem*  itemBelow(QTreeWidget* theWrappedObject, const QTreeWidgetItem*  item) const;
   QWidget*  itemWidget(QTreeWidget* theWrappedObject, QTreeWidgetItem*  item, int  column) const;
   QStringList  mimeTypes(QTreeWidget* theWrappedObject) const;
   void openPersistentEditor(QTreeWidget* theWrappedObject, QTreeWidgetItem*  item, int  column = 0);
   void removeItemWidget(QTreeWidget* theWrappedObject, QTreeWidgetItem*  item, int  column);
   QList<QTreeWidgetItem* >  selectedItems(QTreeWidget* theWrappedObject) const;
   void setColumnCount(QTreeWidget* theWrappedObject, int  columns);
   void setCurrentItem(QTreeWidget* theWrappedObject, QTreeWidgetItem*  item);
   void setCurrentItem(QTreeWidget* theWrappedObject, QTreeWidgetItem*  item, int  column);
   void setCurrentItem(QTreeWidget* theWrappedObject, QTreeWidgetItem*  item, int  column, QItemSelectionModel::SelectionFlags  command);
   void setFirstItemColumnSpanned(QTreeWidget* theWrappedObject, const QTreeWidgetItem*  item, bool  span);
   void setHeaderItem(QTreeWidget* theWrappedObject, QTreeWidgetItem*  item);
   void setHeaderLabel(QTreeWidget* theWrappedObject, const QString&  label);
   void setHeaderLabels(QTreeWidget* theWrappedObject, const QStringList&  labels);
   void setItemWidget(QTreeWidget* theWrappedObject, QTreeWidgetItem*  item, int  column, QWidget*  widget);
   void setSelectionModel(QTreeWidget* theWrappedObject, QItemSelectionModel*  selectionModel);
   int  sortColumn(QTreeWidget* theWrappedObject) const;
   void sortItems(QTreeWidget* theWrappedObject, int  column, Qt::SortOrder  order);
   Qt::DropActions  supportedDropActions(QTreeWidget* theWrappedObject) const;
   QTreeWidgetItem*  takeTopLevelItem(QTreeWidget* theWrappedObject, int  index);
   QTreeWidgetItem*  topLevelItem(QTreeWidget* theWrappedObject, int  index) const;
   int  topLevelItemCount(QTreeWidget* theWrappedObject) const;
   QRect  visualItemRect(QTreeWidget* theWrappedObject, const QTreeWidgetItem*  item) const;
};





class PythonQtShell_QTreeWidgetItem : public QTreeWidgetItem
{
public:
    PythonQtShell_QTreeWidgetItem(QTreeWidget*  view, QTreeWidgetItem*  after, int  type = Type):QTreeWidgetItem(view, after, type),_wrapper(NULL) {};
    PythonQtShell_QTreeWidgetItem(QTreeWidget*  view, const QStringList&  strings, int  type = Type):QTreeWidgetItem(view, strings, type),_wrapper(NULL) {};
    PythonQtShell_QTreeWidgetItem(QTreeWidget*  view, int  type = Type):QTreeWidgetItem(view, type),_wrapper(NULL) {};
    PythonQtShell_QTreeWidgetItem(QTreeWidgetItem*  parent, QTreeWidgetItem*  after, int  type = Type):QTreeWidgetItem(parent, after, type),_wrapper(NULL) {};
    PythonQtShell_QTreeWidgetItem(QTreeWidgetItem*  parent, const QStringList&  strings, int  type = Type):QTreeWidgetItem(parent, strings, type),_wrapper(NULL) {};
    PythonQtShell_QTreeWidgetItem(QTreeWidgetItem*  parent, int  type = Type):QTreeWidgetItem(parent, type),_wrapper(NULL) {};
    PythonQtShell_QTreeWidgetItem(const QStringList&  strings, int  type = Type):QTreeWidgetItem(strings, type),_wrapper(NULL) {};
    PythonQtShell_QTreeWidgetItem(int  type = Type):QTreeWidgetItem(type),_wrapper(NULL) {};

virtual QTreeWidgetItem*  clone() const;
virtual QVariant  data(int  column, int  role) const;
virtual bool  __lt__(const QTreeWidgetItem&  other) const;
virtual void read(QDataStream&  in);
virtual void setData(int  column, int  role, const QVariant&  value);
virtual void write(QDataStream&  out) const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QTreeWidgetItem : public QTreeWidgetItem
{ public:
inline QTreeWidgetItem*  promoted_clone() const { return QTreeWidgetItem::clone(); }
inline QVariant  promoted_data(int  column, int  role) const { return QTreeWidgetItem::data(column, role); }
inline void promoted_setData(int  column, int  role, const QVariant&  value) { QTreeWidgetItem::setData(column, role, value); }
};

class PythonQtWrapper_QTreeWidgetItem : public QObject
{ Q_OBJECT
public:
Q_ENUMS(ItemType ChildIndicatorPolicy )
enum ItemType{
  Type = QTreeWidgetItem::Type,   UserType = QTreeWidgetItem::UserType};
enum ChildIndicatorPolicy{
  ShowIndicator = QTreeWidgetItem::ShowIndicator,   DontShowIndicator = QTreeWidgetItem::DontShowIndicator,   DontShowIndicatorWhenChildless = QTreeWidgetItem::DontShowIndicatorWhenChildless};
public slots:
QTreeWidgetItem* new_QTreeWidgetItem(QTreeWidget*  view, QTreeWidgetItem*  after, int  type = Type);
QTreeWidgetItem* new_QTreeWidgetItem(QTreeWidget*  view, const QStringList&  strings, int  type = Type);
QTreeWidgetItem* new_QTreeWidgetItem(QTreeWidget*  view, int  type = Type);
QTreeWidgetItem* new_QTreeWidgetItem(QTreeWidgetItem*  parent, QTreeWidgetItem*  after, int  type = Type);
QTreeWidgetItem* new_QTreeWidgetItem(QTreeWidgetItem*  parent, const QStringList&  strings, int  type = Type);
QTreeWidgetItem* new_QTreeWidgetItem(QTreeWidgetItem*  parent, int  type = Type);
QTreeWidgetItem* new_QTreeWidgetItem(const QStringList&  strings, int  type = Type);
QTreeWidgetItem* new_QTreeWidgetItem(int  type = Type);
void delete_QTreeWidgetItem(QTreeWidgetItem* obj) { delete obj; } 
bool py_hasOwner(QTreeWidgetItem* theWrappedObject) { return theWrappedObject->treeWidget()!=NULL || theWrappedObject->parent()!=NULL; }
   void addChild(QTreeWidgetItem* theWrappedObject, QTreeWidgetItem*  child);
   void addChildren(QTreeWidgetItem* theWrappedObject, const QList<QTreeWidgetItem* >&  children);
   QBrush  background(QTreeWidgetItem* theWrappedObject, int  column) const;
   Qt::CheckState  checkState(QTreeWidgetItem* theWrappedObject, int  column) const;
   QTreeWidgetItem*  child(QTreeWidgetItem* theWrappedObject, int  index) const;
   int  childCount(QTreeWidgetItem* theWrappedObject) const;
   QTreeWidgetItem::ChildIndicatorPolicy  childIndicatorPolicy(QTreeWidgetItem* theWrappedObject) const;
   QTreeWidgetItem*  clone(QTreeWidgetItem* theWrappedObject) const;
   int  columnCount(QTreeWidgetItem* theWrappedObject) const;
   QVariant  data(QTreeWidgetItem* theWrappedObject, int  column, int  role) const;
   Qt::ItemFlags  flags(QTreeWidgetItem* theWrappedObject) const;
   QFont  font(QTreeWidgetItem* theWrappedObject, int  column) const;
   QBrush  foreground(QTreeWidgetItem* theWrappedObject, int  column) const;
   QIcon  icon(QTreeWidgetItem* theWrappedObject, int  column) const;
   int  indexOfChild(QTreeWidgetItem* theWrappedObject, QTreeWidgetItem*  child) const;
   void insertChild(QTreeWidgetItem* theWrappedObject, int  index, QTreeWidgetItem*  child);
   void insertChildren(QTreeWidgetItem* theWrappedObject, int  index, const QList<QTreeWidgetItem* >&  children);
   bool  isDisabled(QTreeWidgetItem* theWrappedObject) const;
   bool  isExpanded(QTreeWidgetItem* theWrappedObject) const;
   bool  isFirstColumnSpanned(QTreeWidgetItem* theWrappedObject) const;
   bool  isHidden(QTreeWidgetItem* theWrappedObject) const;
   bool  isSelected(QTreeWidgetItem* theWrappedObject) const;
   void writeTo(QTreeWidgetItem* theWrappedObject, QDataStream&  out);
   void readFrom(QTreeWidgetItem* theWrappedObject, QDataStream&  in);
   QTreeWidgetItem*  parent(QTreeWidgetItem* theWrappedObject) const;
   void removeChild(QTreeWidgetItem* theWrappedObject, QTreeWidgetItem*  child);
   void setBackground(QTreeWidgetItem* theWrappedObject, int  column, const QBrush&  brush);
   void setCheckState(QTreeWidgetItem* theWrappedObject, int  column, Qt::CheckState  state);
   void setChildIndicatorPolicy(QTreeWidgetItem* theWrappedObject, QTreeWidgetItem::ChildIndicatorPolicy  policy);
   void setData(QTreeWidgetItem* theWrappedObject, int  column, int  role, const QVariant&  value);
   void setDisabled(QTreeWidgetItem* theWrappedObject, bool  disabled);
   void setExpanded(QTreeWidgetItem* theWrappedObject, bool  expand);
   void setFirstColumnSpanned(QTreeWidgetItem* theWrappedObject, bool  span);
   void setFlags(QTreeWidgetItem* theWrappedObject, Qt::ItemFlags  flags);
   void setFont(QTreeWidgetItem* theWrappedObject, int  column, const QFont&  font);
   void setForeground(QTreeWidgetItem* theWrappedObject, int  column, const QBrush&  brush);
   void setHidden(QTreeWidgetItem* theWrappedObject, bool  hide);
   void setIcon(QTreeWidgetItem* theWrappedObject, int  column, const QIcon&  icon);
   void setSelected(QTreeWidgetItem* theWrappedObject, bool  select);
   void setSizeHint(QTreeWidgetItem* theWrappedObject, int  column, const QSize&  size);
   void setStatusTip(QTreeWidgetItem* theWrappedObject, int  column, const QString&  statusTip);
   void setText(QTreeWidgetItem* theWrappedObject, int  column, const QString&  text);
   void setTextAlignment(QTreeWidgetItem* theWrappedObject, int  column, int  alignment);
   void setToolTip(QTreeWidgetItem* theWrappedObject, int  column, const QString&  toolTip);
   void setWhatsThis(QTreeWidgetItem* theWrappedObject, int  column, const QString&  whatsThis);
   QSize  sizeHint(QTreeWidgetItem* theWrappedObject, int  column) const;
   void sortChildren(QTreeWidgetItem* theWrappedObject, int  column, Qt::SortOrder  order);
   QString  statusTip(QTreeWidgetItem* theWrappedObject, int  column) const;
   QTreeWidgetItem*  takeChild(QTreeWidgetItem* theWrappedObject, int  index);
   QList<QTreeWidgetItem* >  takeChildren(QTreeWidgetItem* theWrappedObject);
   QString  text(QTreeWidgetItem* theWrappedObject, int  column) const;
   int  textAlignment(QTreeWidgetItem* theWrappedObject, int  column) const;
   QString  toolTip(QTreeWidgetItem* theWrappedObject, int  column) const;
   QTreeWidget*  treeWidget(QTreeWidgetItem* theWrappedObject) const;
   int  type(QTreeWidgetItem* theWrappedObject) const;
   QString  whatsThis(QTreeWidgetItem* theWrappedObject, int  column) const;
};





class PythonQtShell_QUndoCommand : public QUndoCommand
{
public:
    PythonQtShell_QUndoCommand(QUndoCommand*  parent = 0):QUndoCommand(parent),_wrapper(NULL) {};
    PythonQtShell_QUndoCommand(const QString&  text, QUndoCommand*  parent = 0):QUndoCommand(text, parent),_wrapper(NULL) {};

virtual int  id() const;
virtual bool  mergeWith(const QUndoCommand*  other);
virtual void redo();
virtual void undo();

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QUndoCommand : public QUndoCommand
{ public:
inline int  promoted_id() const { return QUndoCommand::id(); }
inline bool  promoted_mergeWith(const QUndoCommand*  other) { return QUndoCommand::mergeWith(other); }
inline void promoted_redo() { QUndoCommand::redo(); }
inline void promoted_undo() { QUndoCommand::undo(); }
};

class PythonQtWrapper_QUndoCommand : public QObject
{ Q_OBJECT
public:
public slots:
QUndoCommand* new_QUndoCommand(QUndoCommand*  parent = 0);
QUndoCommand* new_QUndoCommand(const QString&  text, QUndoCommand*  parent = 0);
void delete_QUndoCommand(QUndoCommand* obj) { delete obj; } 
   const QUndoCommand*  child(QUndoCommand* theWrappedObject, int  index) const;
   int  childCount(QUndoCommand* theWrappedObject) const;
   int  id(QUndoCommand* theWrappedObject) const;
   bool  mergeWith(QUndoCommand* theWrappedObject, const QUndoCommand*  other);
   void redo(QUndoCommand* theWrappedObject);
   void setText(QUndoCommand* theWrappedObject, const QString&  text);
   QString  text(QUndoCommand* theWrappedObject) const;
   void undo(QUndoCommand* theWrappedObject);
};





class PythonQtShell_QUndoGroup : public QUndoGroup
{
public:
    PythonQtShell_QUndoGroup(QObject*  parent = 0):QUndoGroup(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QUndoGroup : public QObject
{ Q_OBJECT
public:
public slots:
QUndoGroup* new_QUndoGroup(QObject*  parent = 0);
void delete_QUndoGroup(QUndoGroup* obj) { delete obj; } 
   QUndoStack*  activeStack(QUndoGroup* theWrappedObject) const;
   void addStack(QUndoGroup* theWrappedObject, QUndoStack*  stack);
   bool  canRedo(QUndoGroup* theWrappedObject) const;
   bool  canUndo(QUndoGroup* theWrappedObject) const;
   QAction*  createRedoAction(QUndoGroup* theWrappedObject, QObject*  parent, const QString&  prefix = QString()) const;
   QAction*  createUndoAction(QUndoGroup* theWrappedObject, QObject*  parent, const QString&  prefix = QString()) const;
   bool  isClean(QUndoGroup* theWrappedObject) const;
   QString  redoText(QUndoGroup* theWrappedObject) const;
   void removeStack(QUndoGroup* theWrappedObject, QUndoStack*  stack);
   QList<QUndoStack* >  stacks(QUndoGroup* theWrappedObject) const;
   QString  undoText(QUndoGroup* theWrappedObject) const;
};





class PythonQtShell_QUndoStack : public QUndoStack
{
public:
    PythonQtShell_QUndoStack(QObject*  parent = 0):QUndoStack(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QUndoStack : public QObject
{ Q_OBJECT
public:
public slots:
QUndoStack* new_QUndoStack(QObject*  parent = 0);
void delete_QUndoStack(QUndoStack* obj) { delete obj; } 
   void beginMacro(QUndoStack* theWrappedObject, const QString&  text);
   bool  canRedo(QUndoStack* theWrappedObject) const;
   bool  canUndo(QUndoStack* theWrappedObject) const;
   int  cleanIndex(QUndoStack* theWrappedObject) const;
   void clear(QUndoStack* theWrappedObject);
   const QUndoCommand*  command(QUndoStack* theWrappedObject, int  index) const;
   int  count(QUndoStack* theWrappedObject) const;
   QAction*  createRedoAction(QUndoStack* theWrappedObject, QObject*  parent, const QString&  prefix = QString()) const;
   QAction*  createUndoAction(QUndoStack* theWrappedObject, QObject*  parent, const QString&  prefix = QString()) const;
   void endMacro(QUndoStack* theWrappedObject);
   int  index(QUndoStack* theWrappedObject) const;
   bool  isActive(QUndoStack* theWrappedObject) const;
   bool  isClean(QUndoStack* theWrappedObject) const;
   void push(QUndoStack* theWrappedObject, QUndoCommand*  cmd);
   QString  redoText(QUndoStack* theWrappedObject) const;
   void setUndoLimit(QUndoStack* theWrappedObject, int  limit);
   QString  text(QUndoStack* theWrappedObject, int  idx) const;
   int  undoLimit(QUndoStack* theWrappedObject) const;
   QString  undoText(QUndoStack* theWrappedObject) const;
};





class PythonQtShell_QUndoView : public QUndoView
{
public:
    PythonQtShell_QUndoView(QUndoGroup*  group, QWidget*  parent = 0):QUndoView(group, parent),_wrapper(NULL) {};
    PythonQtShell_QUndoView(QUndoStack*  stack, QWidget*  parent = 0):QUndoView(stack, parent),_wrapper(NULL) {};
    PythonQtShell_QUndoView(QWidget*  parent = 0):QUndoView(parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  arg__1);
virtual void childEvent(QChildEvent*  arg__1);
virtual void closeEditor(QWidget*  editor, QAbstractItemDelegate::EndEditHint  hint);
virtual void closeEvent(QCloseEvent*  arg__1);
virtual void commitData(QWidget*  editor);
virtual void contextMenuEvent(QContextMenuEvent*  arg__1);
virtual void currentChanged(const QModelIndex&  current, const QModelIndex&  previous);
virtual void customEvent(QEvent*  arg__1);
virtual void dataChanged(const QModelIndex&  topLeft, const QModelIndex&  bottomRight);
virtual int  devType() const;
virtual void doItemsLayout();
virtual void dragEnterEvent(QDragEnterEvent*  event);
virtual void dragLeaveEvent(QDragLeaveEvent*  e);
virtual void dragMoveEvent(QDragMoveEvent*  e);
virtual void dropEvent(QDropEvent*  e);
virtual bool  edit(const QModelIndex&  index, QAbstractItemView::EditTrigger  trigger, QEvent*  event);
virtual void editorDestroyed(QObject*  editor);
virtual void enterEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  e);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void focusInEvent(QFocusEvent*  event);
virtual bool  focusNextPrevChild(bool  next);
virtual void focusOutEvent(QFocusEvent*  event);
virtual int  heightForWidth(int  arg__1) const;
virtual void hideEvent(QHideEvent*  arg__1);
virtual int  horizontalOffset() const;
virtual void horizontalScrollbarAction(int  action);
virtual void horizontalScrollbarValueChanged(int  value);
virtual QModelIndex  indexAt(const QPoint&  p) const;
virtual void inputMethodEvent(QInputMethodEvent*  event);
virtual QVariant  inputMethodQuery(Qt::InputMethodQuery  query) const;
virtual bool  isIndexHidden(const QModelIndex&  index) const;
virtual void keyPressEvent(QKeyEvent*  event);
virtual void keyReleaseEvent(QKeyEvent*  arg__1);
virtual void keyboardSearch(const QString&  search);
virtual void languageChange();
virtual void leaveEvent(QEvent*  arg__1);
virtual int  metric(QPaintDevice::PaintDeviceMetric  arg__1) const;
virtual void mouseDoubleClickEvent(QMouseEvent*  event);
virtual void mouseMoveEvent(QMouseEvent*  e);
virtual void mousePressEvent(QMouseEvent*  event);
virtual void mouseReleaseEvent(QMouseEvent*  e);
virtual void moveEvent(QMoveEvent*  arg__1);
virtual QPaintEngine*  paintEngine() const;
virtual void paintEvent(QPaintEvent*  e);
virtual void reset();
virtual void resizeEvent(QResizeEvent*  e);
virtual void rowsAboutToBeRemoved(const QModelIndex&  parent, int  start, int  end);
virtual void rowsInserted(const QModelIndex&  parent, int  start, int  end);
virtual void scrollContentsBy(int  dx, int  dy);
virtual void scrollTo(const QModelIndex&  index, QAbstractItemView::ScrollHint  hint);
virtual void selectAll();
virtual QList<QModelIndex >  selectedIndexes() const;
virtual void selectionChanged(const QItemSelection&  selected, const QItemSelection&  deselected);
virtual QItemSelectionModel::SelectionFlags  selectionCommand(const QModelIndex&  index, const QEvent*  event) const;
virtual void setModel(QAbstractItemModel*  model);
virtual void setRootIndex(const QModelIndex&  index);
virtual void setSelection(const QRect&  rect, QItemSelectionModel::SelectionFlags  command);
virtual void setSelectionModel(QItemSelectionModel*  selectionModel);
virtual void showEvent(QShowEvent*  arg__1);
virtual int  sizeHintForColumn(int  column) const;
virtual int  sizeHintForRow(int  row) const;
virtual void startDrag(Qt::DropActions  supportedActions);
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  e);
virtual void updateEditorData();
virtual void updateEditorGeometries();
virtual void updateGeometries();
virtual int  verticalOffset() const;
virtual void verticalScrollbarAction(int  action);
virtual void verticalScrollbarValueChanged(int  value);
virtual QStyleOptionViewItem  viewOptions() const;
virtual bool  viewportEvent(QEvent*  event);
virtual QRect  visualRect(const QModelIndex&  index) const;
virtual QRegion  visualRegionForSelection(const QItemSelection&  selection) const;
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QUndoView : public QObject
{ Q_OBJECT
public:
public slots:
QUndoView* new_QUndoView(QUndoGroup*  group, QWidget*  parent = 0);
QUndoView* new_QUndoView(QUndoStack*  stack, QWidget*  parent = 0);
QUndoView* new_QUndoView(QWidget*  parent = 0);
void delete_QUndoView(QUndoView* obj) { delete obj; } 
   QIcon  cleanIcon(QUndoView* theWrappedObject) const;
   QString  emptyLabel(QUndoView* theWrappedObject) const;
   QUndoGroup*  group(QUndoView* theWrappedObject) const;
   void setCleanIcon(QUndoView* theWrappedObject, const QIcon&  icon);
   void setEmptyLabel(QUndoView* theWrappedObject, const QString&  label);
   QUndoStack*  stack(QUndoView* theWrappedObject) const;
};





class PythonQtShell_QVBoxLayout : public QVBoxLayout
{
public:
    PythonQtShell_QVBoxLayout():QVBoxLayout(),_wrapper(NULL) {};
    PythonQtShell_QVBoxLayout(QWidget*  parent):QVBoxLayout(parent),_wrapper(NULL) {};

virtual void addItem(QLayoutItem*  arg__1);
virtual void childEvent(QChildEvent*  e);
virtual int  count() const;
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual Qt::Orientations  expandingDirections() const;
virtual QRect  geometry() const;
virtual int  indexOf(QWidget*  arg__1) const;
virtual void invalidate();
virtual bool  isEmpty() const;
virtual QLayoutItem*  itemAt(int  arg__1) const;
virtual QLayout*  layout();
virtual QSize  maximumSize() const;
virtual QSize  minimumSize() const;
virtual void setGeometry(const QRect&  arg__1);
virtual QLayoutItem*  takeAt(int  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QVBoxLayout : public QObject
{ Q_OBJECT
public:
public slots:
QVBoxLayout* new_QVBoxLayout();
QVBoxLayout* new_QVBoxLayout(QWidget*  parent);
void delete_QVBoxLayout(QVBoxLayout* obj) { delete obj; } 
};





class PythonQtShell_QValidator : public QValidator
{
public:
    PythonQtShell_QValidator(QObject*  parent = 0):QValidator(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void fixup(QString&  arg__1) const;
virtual void timerEvent(QTimerEvent*  arg__1);
virtual QValidator::State  validate(QString&  arg__1, int&  arg__2) const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QValidator : public QValidator
{ public:
inline void promoted_fixup(QString&  arg__1) const { QValidator::fixup(arg__1); }
};

class PythonQtWrapper_QValidator : public QObject
{ Q_OBJECT
public:
Q_ENUMS(State )
enum State{
  Invalid = QValidator::Invalid,   Intermediate = QValidator::Intermediate,   Acceptable = QValidator::Acceptable};
public slots:
QValidator* new_QValidator(QObject*  parent = 0);
void delete_QValidator(QValidator* obj) { delete obj; } 
   void fixup(QValidator* theWrappedObject, QString&  arg__1) const;
   QLocale  locale(QValidator* theWrappedObject) const;
   void setLocale(QValidator* theWrappedObject, const QLocale&  locale);
};





class PythonQtWrapper_QVector2D : public QObject
{ Q_OBJECT
public:
public slots:
QVector2D* new_QVector2D();
QVector2D* new_QVector2D(const QPoint&  point);
QVector2D* new_QVector2D(const QPointF&  point);
QVector2D* new_QVector2D(const QVector3D&  vector);
QVector2D* new_QVector2D(const QVector4D&  vector);
QVector2D* new_QVector2D(qreal  xpos, qreal  ypos);
QVector2D* new_QVector2D(const QVector2D& other) {
QVector2D* a = new QVector2D();
*((QVector2D*)a) = other;
return a; }
void delete_QVector2D(QVector2D* obj) { delete obj; } 
   qreal  static_QVector2D_dotProduct(const QVector2D&  v1, const QVector2D&  v2);
   bool  isNull(QVector2D* theWrappedObject) const;
   qreal  length(QVector2D* theWrappedObject) const;
   qreal  lengthSquared(QVector2D* theWrappedObject) const;
   void normalize(QVector2D* theWrappedObject);
   QVector2D  normalized(QVector2D* theWrappedObject) const;
   const QVector2D  __mul__(QVector2D* theWrappedObject, const QVector2D&  v2);
   const QVector2D  __mul__(QVector2D* theWrappedObject, qreal  factor);
   QVector2D*  __imul__(QVector2D* theWrappedObject, const QVector2D&  vector);
   QVector2D*  __imul__(QVector2D* theWrappedObject, qreal  factor);
   const QVector2D  __add__(QVector2D* theWrappedObject, const QVector2D&  v2);
   QVector2D*  __iadd__(QVector2D* theWrappedObject, const QVector2D&  vector);
   const QVector2D  __sub__(QVector2D* theWrappedObject, const QVector2D&  v2);
   QVector2D*  __isub__(QVector2D* theWrappedObject, const QVector2D&  vector);
   const QVector2D  __div__(QVector2D* theWrappedObject, qreal  divisor);
   QVector2D*  __idiv__(QVector2D* theWrappedObject, qreal  divisor);
   void writeTo(QVector2D* theWrappedObject, QDataStream&  arg__1);
   bool  __eq__(QVector2D* theWrappedObject, const QVector2D&  v2);
   void readFrom(QVector2D* theWrappedObject, QDataStream&  arg__1);
   void setX(QVector2D* theWrappedObject, qreal  x);
   void setY(QVector2D* theWrappedObject, qreal  y);
   QPoint  toPoint(QVector2D* theWrappedObject) const;
   QPointF  toPointF(QVector2D* theWrappedObject) const;
   QVector3D  toVector3D(QVector2D* theWrappedObject) const;
   QVector4D  toVector4D(QVector2D* theWrappedObject) const;
   qreal  x(QVector2D* theWrappedObject) const;
   qreal  y(QVector2D* theWrappedObject) const;
    QString py_toString(QVector2D*);
    bool __nonzero__(QVector2D* obj) { return !obj->isNull(); }
};





class PythonQtWrapper_QVector3D : public QObject
{ Q_OBJECT
public:
public slots:
QVector3D* new_QVector3D();
QVector3D* new_QVector3D(const QPoint&  point);
QVector3D* new_QVector3D(const QPointF&  point);
QVector3D* new_QVector3D(const QVector2D&  vector);
QVector3D* new_QVector3D(const QVector2D&  vector, qreal  zpos);
QVector3D* new_QVector3D(const QVector4D&  vector);
QVector3D* new_QVector3D(qreal  xpos, qreal  ypos, qreal  zpos);
QVector3D* new_QVector3D(const QVector3D& other) {
QVector3D* a = new QVector3D();
*((QVector3D*)a) = other;
return a; }
void delete_QVector3D(QVector3D* obj) { delete obj; } 
   QVector3D  static_QVector3D_crossProduct(const QVector3D&  v1, const QVector3D&  v2);
   qreal  distanceToLine(QVector3D* theWrappedObject, const QVector3D&  point, const QVector3D&  direction) const;
   qreal  distanceToPlane(QVector3D* theWrappedObject, const QVector3D&  plane, const QVector3D&  normal) const;
   qreal  distanceToPlane(QVector3D* theWrappedObject, const QVector3D&  plane1, const QVector3D&  plane2, const QVector3D&  plane3) const;
   qreal  static_QVector3D_dotProduct(const QVector3D&  v1, const QVector3D&  v2);
   bool  isNull(QVector3D* theWrappedObject) const;
   qreal  length(QVector3D* theWrappedObject) const;
   qreal  lengthSquared(QVector3D* theWrappedObject) const;
   QVector3D  static_QVector3D_normal(const QVector3D&  v1, const QVector3D&  v2);
   QVector3D  static_QVector3D_normal(const QVector3D&  v1, const QVector3D&  v2, const QVector3D&  v3);
   void normalize(QVector3D* theWrappedObject);
   QVector3D  normalized(QVector3D* theWrappedObject) const;
   QVector3D  __mul__(QVector3D* theWrappedObject, const QMatrix4x4&  matrix);
   const QVector3D  __mul__(QVector3D* theWrappedObject, const QVector3D&  v2);
   const QVector3D  __mul__(QVector3D* theWrappedObject, qreal  factor);
   QVector3D*  __imul__(QVector3D* theWrappedObject, const QVector3D&  vector);
   QVector3D*  __imul__(QVector3D* theWrappedObject, qreal  factor);
   const QVector3D  __add__(QVector3D* theWrappedObject, const QVector3D&  v2);
   QVector3D*  __iadd__(QVector3D* theWrappedObject, const QVector3D&  vector);
   const QVector3D  __sub__(QVector3D* theWrappedObject, const QVector3D&  v2);
   QVector3D*  __isub__(QVector3D* theWrappedObject, const QVector3D&  vector);
   const QVector3D  __div__(QVector3D* theWrappedObject, qreal  divisor);
   QVector3D*  __idiv__(QVector3D* theWrappedObject, qreal  divisor);
   void writeTo(QVector3D* theWrappedObject, QDataStream&  arg__1);
   bool  __eq__(QVector3D* theWrappedObject, const QVector3D&  v2);
   void readFrom(QVector3D* theWrappedObject, QDataStream&  arg__1);
   void setX(QVector3D* theWrappedObject, qreal  x);
   void setY(QVector3D* theWrappedObject, qreal  y);
   void setZ(QVector3D* theWrappedObject, qreal  z);
   QPoint  toPoint(QVector3D* theWrappedObject) const;
   QPointF  toPointF(QVector3D* theWrappedObject) const;
   QVector2D  toVector2D(QVector3D* theWrappedObject) const;
   QVector4D  toVector4D(QVector3D* theWrappedObject) const;
   qreal  x(QVector3D* theWrappedObject) const;
   qreal  y(QVector3D* theWrappedObject) const;
   qreal  z(QVector3D* theWrappedObject) const;
    QString py_toString(QVector3D*);
    bool __nonzero__(QVector3D* obj) { return !obj->isNull(); }
};





class PythonQtWrapper_QVector4D : public QObject
{ Q_OBJECT
public:
public slots:
QVector4D* new_QVector4D();
QVector4D* new_QVector4D(const QPoint&  point);
QVector4D* new_QVector4D(const QPointF&  point);
QVector4D* new_QVector4D(const QVector2D&  vector);
QVector4D* new_QVector4D(const QVector2D&  vector, qreal  zpos, qreal  wpos);
QVector4D* new_QVector4D(const QVector3D&  vector);
QVector4D* new_QVector4D(const QVector3D&  vector, qreal  wpos);
QVector4D* new_QVector4D(qreal  xpos, qreal  ypos, qreal  zpos, qreal  wpos);
QVector4D* new_QVector4D(const QVector4D& other) {
QVector4D* a = new QVector4D();
*((QVector4D*)a) = other;
return a; }
void delete_QVector4D(QVector4D* obj) { delete obj; } 
   qreal  static_QVector4D_dotProduct(const QVector4D&  v1, const QVector4D&  v2);
   bool  isNull(QVector4D* theWrappedObject) const;
   qreal  length(QVector4D* theWrappedObject) const;
   qreal  lengthSquared(QVector4D* theWrappedObject) const;
   void normalize(QVector4D* theWrappedObject);
   QVector4D  normalized(QVector4D* theWrappedObject) const;
   QVector4D  __mul__(QVector4D* theWrappedObject, const QMatrix4x4&  matrix);
   const QVector4D  __mul__(QVector4D* theWrappedObject, const QVector4D&  v2);
   const QVector4D  __mul__(QVector4D* theWrappedObject, qreal  factor);
   QVector4D*  __imul__(QVector4D* theWrappedObject, const QVector4D&  vector);
   QVector4D*  __imul__(QVector4D* theWrappedObject, qreal  factor);
   const QVector4D  __add__(QVector4D* theWrappedObject, const QVector4D&  v2);
   QVector4D*  __iadd__(QVector4D* theWrappedObject, const QVector4D&  vector);
   const QVector4D  __sub__(QVector4D* theWrappedObject, const QVector4D&  v2);
   QVector4D*  __isub__(QVector4D* theWrappedObject, const QVector4D&  vector);
   const QVector4D  __div__(QVector4D* theWrappedObject, qreal  divisor);
   QVector4D*  __idiv__(QVector4D* theWrappedObject, qreal  divisor);
   void writeTo(QVector4D* theWrappedObject, QDataStream&  arg__1);
   bool  __eq__(QVector4D* theWrappedObject, const QVector4D&  v2);
   void readFrom(QVector4D* theWrappedObject, QDataStream&  arg__1);
   void setW(QVector4D* theWrappedObject, qreal  w);
   void setX(QVector4D* theWrappedObject, qreal  x);
   void setY(QVector4D* theWrappedObject, qreal  y);
   void setZ(QVector4D* theWrappedObject, qreal  z);
   QPoint  toPoint(QVector4D* theWrappedObject) const;
   QPointF  toPointF(QVector4D* theWrappedObject) const;
   QVector2D  toVector2D(QVector4D* theWrappedObject) const;
   QVector2D  toVector2DAffine(QVector4D* theWrappedObject) const;
   QVector3D  toVector3D(QVector4D* theWrappedObject) const;
   QVector3D  toVector3DAffine(QVector4D* theWrappedObject) const;
   qreal  w(QVector4D* theWrappedObject) const;
   qreal  x(QVector4D* theWrappedObject) const;
   qreal  y(QVector4D* theWrappedObject) const;
   qreal  z(QVector4D* theWrappedObject) const;
    QString py_toString(QVector4D*);
    bool __nonzero__(QVector4D* obj) { return !obj->isNull(); }
};





class PythonQtWrapper_QWhatsThis : public QObject
{ Q_OBJECT
public:
public slots:
void delete_QWhatsThis(QWhatsThis* obj) { delete obj; } 
   QAction*  static_QWhatsThis_createAction(QObject*  parent = 0);
   void static_QWhatsThis_enterWhatsThisMode();
   void static_QWhatsThis_hideText();
   bool  static_QWhatsThis_inWhatsThisMode();
   void static_QWhatsThis_leaveWhatsThisMode();
   void static_QWhatsThis_showText(const QPoint&  pos, const QString&  text, QWidget*  w = 0);
};





class PythonQtWrapper_QWhatsThisClickedEvent : public QObject
{ Q_OBJECT
public:
public slots:
QWhatsThisClickedEvent* new_QWhatsThisClickedEvent(const QString&  href);
void delete_QWhatsThisClickedEvent(QWhatsThisClickedEvent* obj) { delete obj; } 
   QString  href(QWhatsThisClickedEvent* theWrappedObject) const;
};





class PythonQtShell_QWheelEvent : public QWheelEvent
{
public:
    PythonQtShell_QWheelEvent(const QPoint&  pos, const QPoint&  globalPos, int  delta, Qt::MouseButtons  buttons, Qt::KeyboardModifiers  modifiers, Qt::Orientation  orient = Qt::Vertical):QWheelEvent(pos, globalPos, delta, buttons, modifiers, orient),_wrapper(NULL) {};
    PythonQtShell_QWheelEvent(const QPoint&  pos, int  delta, Qt::MouseButtons  buttons, Qt::KeyboardModifiers  modifiers, Qt::Orientation  orient = Qt::Vertical):QWheelEvent(pos, delta, buttons, modifiers, orient),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QWheelEvent : public QObject
{ Q_OBJECT
public:
public slots:
QWheelEvent* new_QWheelEvent(const QPoint&  pos, const QPoint&  globalPos, int  delta, Qt::MouseButtons  buttons, Qt::KeyboardModifiers  modifiers, Qt::Orientation  orient = Qt::Vertical);
QWheelEvent* new_QWheelEvent(const QPoint&  pos, int  delta, Qt::MouseButtons  buttons, Qt::KeyboardModifiers  modifiers, Qt::Orientation  orient = Qt::Vertical);
void delete_QWheelEvent(QWheelEvent* obj) { delete obj; } 
   Qt::MouseButtons  buttons(QWheelEvent* theWrappedObject) const;
   int  delta(QWheelEvent* theWrappedObject) const;
   const QPoint*  globalPos(QWheelEvent* theWrappedObject) const;
   int  globalX(QWheelEvent* theWrappedObject) const;
   int  globalY(QWheelEvent* theWrappedObject) const;
   Qt::Orientation  orientation(QWheelEvent* theWrappedObject) const;
   const QPoint*  pos(QWheelEvent* theWrappedObject) const;
   int  x(QWheelEvent* theWrappedObject) const;
   int  y(QWheelEvent* theWrappedObject) const;
};





class PythonQtShell_QWidget : public QWidget
{
public:
    PythonQtShell_QWidget(QWidget*  parent = 0, Qt::WindowFlags  f = 0):QWidget(parent, f),_wrapper(NULL) {};

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
virtual void enabledChange(bool  arg__1);
virtual void enterEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void focusInEvent(QFocusEvent*  arg__1);
virtual bool  focusNextPrevChild(bool  next);
virtual void focusOutEvent(QFocusEvent*  arg__1);
virtual void fontChange(const QFont&  arg__1);
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
virtual void paletteChange(const QPalette&  arg__1);
virtual void resizeEvent(QResizeEvent*  arg__1);
virtual void setVisible(bool  visible);
virtual void showEvent(QShowEvent*  arg__1);
virtual QSize  sizeHint() const;
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void wheelEvent(QWheelEvent*  arg__1);
virtual void windowActivationChange(bool  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QWidget : public QWidget
{ public:
inline void promoted_actionEvent(QActionEvent*  arg__1) { QWidget::actionEvent(arg__1); }
inline void promoted_changeEvent(QEvent*  arg__1) { QWidget::changeEvent(arg__1); }
inline void promoted_closeEvent(QCloseEvent*  arg__1) { QWidget::closeEvent(arg__1); }
inline void promoted_contextMenuEvent(QContextMenuEvent*  arg__1) { QWidget::contextMenuEvent(arg__1); }
inline int  promoted_devType() const { return QWidget::devType(); }
inline void promoted_dragEnterEvent(QDragEnterEvent*  arg__1) { QWidget::dragEnterEvent(arg__1); }
inline void promoted_dragLeaveEvent(QDragLeaveEvent*  arg__1) { QWidget::dragLeaveEvent(arg__1); }
inline void promoted_dragMoveEvent(QDragMoveEvent*  arg__1) { QWidget::dragMoveEvent(arg__1); }
inline void promoted_dropEvent(QDropEvent*  arg__1) { QWidget::dropEvent(arg__1); }
inline void promoted_enterEvent(QEvent*  arg__1) { QWidget::enterEvent(arg__1); }
inline bool  promoted_event(QEvent*  arg__1) { return QWidget::event(arg__1); }
inline void promoted_focusInEvent(QFocusEvent*  arg__1) { QWidget::focusInEvent(arg__1); }
inline bool  promoted_focusNextPrevChild(bool  next) { return QWidget::focusNextPrevChild(next); }
inline void promoted_focusOutEvent(QFocusEvent*  arg__1) { QWidget::focusOutEvent(arg__1); }
inline int  promoted_heightForWidth(int  arg__1) const { return QWidget::heightForWidth(arg__1); }
inline void promoted_hideEvent(QHideEvent*  arg__1) { QWidget::hideEvent(arg__1); }
inline void promoted_inputMethodEvent(QInputMethodEvent*  arg__1) { QWidget::inputMethodEvent(arg__1); }
inline QVariant  promoted_inputMethodQuery(Qt::InputMethodQuery  arg__1) const { return QWidget::inputMethodQuery(arg__1); }
inline void promoted_keyPressEvent(QKeyEvent*  arg__1) { QWidget::keyPressEvent(arg__1); }
inline void promoted_keyReleaseEvent(QKeyEvent*  arg__1) { QWidget::keyReleaseEvent(arg__1); }
inline void promoted_languageChange() { QWidget::languageChange(); }
inline void promoted_leaveEvent(QEvent*  arg__1) { QWidget::leaveEvent(arg__1); }
inline int  promoted_metric(QPaintDevice::PaintDeviceMetric  arg__1) const { return QWidget::metric(arg__1); }
inline QSize  promoted_minimumSizeHint() const { return QWidget::minimumSizeHint(); }
inline void promoted_mouseDoubleClickEvent(QMouseEvent*  arg__1) { QWidget::mouseDoubleClickEvent(arg__1); }
inline void promoted_mouseMoveEvent(QMouseEvent*  arg__1) { QWidget::mouseMoveEvent(arg__1); }
inline void promoted_mousePressEvent(QMouseEvent*  arg__1) { QWidget::mousePressEvent(arg__1); }
inline void promoted_mouseReleaseEvent(QMouseEvent*  arg__1) { QWidget::mouseReleaseEvent(arg__1); }
inline void promoted_moveEvent(QMoveEvent*  arg__1) { QWidget::moveEvent(arg__1); }
inline QPaintEngine*  promoted_paintEngine() const { return QWidget::paintEngine(); }
inline void promoted_paintEvent(QPaintEvent*  arg__1) { QWidget::paintEvent(arg__1); }
inline void promoted_resizeEvent(QResizeEvent*  arg__1) { QWidget::resizeEvent(arg__1); }
inline void promoted_showEvent(QShowEvent*  arg__1) { QWidget::showEvent(arg__1); }
inline QSize  promoted_sizeHint() const { return QWidget::sizeHint(); }
inline void promoted_tabletEvent(QTabletEvent*  arg__1) { QWidget::tabletEvent(arg__1); }
inline void promoted_wheelEvent(QWheelEvent*  arg__1) { QWidget::wheelEvent(arg__1); }
};

class PythonQtWrapper_QWidget : public QObject
{ Q_OBJECT
public:
Q_ENUMS(RenderFlag )
Q_FLAGS(RenderFlags )
enum RenderFlag{
  DrawWindowBackground = QWidget::DrawWindowBackground,   DrawChildren = QWidget::DrawChildren,   IgnoreMask = QWidget::IgnoreMask};
Q_DECLARE_FLAGS(RenderFlags, RenderFlag)
public slots:
QWidget* new_QWidget(QWidget*  parent = 0, Qt::WindowFlags  f = 0);
void delete_QWidget(QWidget* obj) { delete obj; } 
   bool  acceptDrops(QWidget* theWrappedObject) const;
   QString  accessibleDescription(QWidget* theWrappedObject) const;
   QString  accessibleName(QWidget* theWrappedObject) const;
   void actionEvent(QWidget* theWrappedObject, QActionEvent*  arg__1);
   QList<QAction* >  actions(QWidget* theWrappedObject) const;
   void activateWindow(QWidget* theWrappedObject);
   void addAction(QWidget* theWrappedObject, QAction*  action);
   void addActions(QWidget* theWrappedObject, QList<QAction* >  actions);
   void adjustSize(QWidget* theWrappedObject);
   bool  autoFillBackground(QWidget* theWrappedObject) const;
   QPalette::ColorRole  backgroundRole(QWidget* theWrappedObject) const;
   QSize  baseSize(QWidget* theWrappedObject) const;
   void changeEvent(QWidget* theWrappedObject, QEvent*  arg__1);
   QWidget*  childAt(QWidget* theWrappedObject, const QPoint&  p) const;
   QWidget*  childAt(QWidget* theWrappedObject, int  x, int  y) const;
   QRect  childrenRect(QWidget* theWrappedObject) const;
   QRegion  childrenRegion(QWidget* theWrappedObject) const;
   void clearFocus(QWidget* theWrappedObject);
   void clearMask(QWidget* theWrappedObject);
   void closeEvent(QWidget* theWrappedObject, QCloseEvent*  arg__1);
   QMargins  contentsMargins(QWidget* theWrappedObject) const;
   QRect  contentsRect(QWidget* theWrappedObject) const;
   void contextMenuEvent(QWidget* theWrappedObject, QContextMenuEvent*  arg__1);
   Qt::ContextMenuPolicy  contextMenuPolicy(QWidget* theWrappedObject) const;
   void createWinId(QWidget* theWrappedObject);
   QCursor  cursor(QWidget* theWrappedObject) const;
   int  devType(QWidget* theWrappedObject) const;
   void dragEnterEvent(QWidget* theWrappedObject, QDragEnterEvent*  arg__1);
   void dragLeaveEvent(QWidget* theWrappedObject, QDragLeaveEvent*  arg__1);
   void dragMoveEvent(QWidget* theWrappedObject, QDragMoveEvent*  arg__1);
   void dropEvent(QWidget* theWrappedObject, QDropEvent*  arg__1);
   WId  effectiveWinId(QWidget* theWrappedObject) const;
   void ensurePolished(QWidget* theWrappedObject) const;
   void enterEvent(QWidget* theWrappedObject, QEvent*  arg__1);
   bool  event(QWidget* theWrappedObject, QEvent*  arg__1);
   void focusInEvent(QWidget* theWrappedObject, QFocusEvent*  arg__1);
   bool  focusNextPrevChild(QWidget* theWrappedObject, bool  next);
   void focusOutEvent(QWidget* theWrappedObject, QFocusEvent*  arg__1);
   Qt::FocusPolicy  focusPolicy(QWidget* theWrappedObject) const;
   QWidget*  focusProxy(QWidget* theWrappedObject) const;
   QWidget*  focusWidget(QWidget* theWrappedObject) const;
   const QFont*  font(QWidget* theWrappedObject) const;
   QPalette::ColorRole  foregroundRole(QWidget* theWrappedObject) const;
   QRect  frameGeometry(QWidget* theWrappedObject) const;
   QSize  frameSize(QWidget* theWrappedObject) const;
   const QRect*  geometry(QWidget* theWrappedObject) const;
   void getContentsMargins(QWidget* theWrappedObject, int*  left, int*  top, int*  right, int*  bottom) const;
   void grabKeyboard(QWidget* theWrappedObject);
   void grabMouse(QWidget* theWrappedObject);
   void grabMouse(QWidget* theWrappedObject, const QCursor&  arg__1);
   int  grabShortcut(QWidget* theWrappedObject, const QKeySequence&  key, Qt::ShortcutContext  context = Qt::WindowShortcut);
   QGraphicsEffect*  graphicsEffect(QWidget* theWrappedObject) const;
   QGraphicsProxyWidget*  graphicsProxyWidget(QWidget* theWrappedObject) const;
   bool  hasFocus(QWidget* theWrappedObject) const;
   bool  hasMouseTracking(QWidget* theWrappedObject) const;
   int  height(QWidget* theWrappedObject) const;
   int  heightForWidth(QWidget* theWrappedObject, int  arg__1) const;
   void hideEvent(QWidget* theWrappedObject, QHideEvent*  arg__1);
   QInputContext*  inputContext(QWidget* theWrappedObject);
   void inputMethodEvent(QWidget* theWrappedObject, QInputMethodEvent*  arg__1);
   Qt::InputMethodHints  inputMethodHints(QWidget* theWrappedObject) const;
   QVariant  inputMethodQuery(QWidget* theWrappedObject, Qt::InputMethodQuery  arg__1) const;
   void insertAction(QWidget* theWrappedObject, QAction*  before, QAction*  action);
   void insertActions(QWidget* theWrappedObject, QAction*  before, QList<QAction* >  actions);
   bool  isActiveWindow(QWidget* theWrappedObject) const;
   bool  isAncestorOf(QWidget* theWrappedObject, const QWidget*  child) const;
   bool  isEnabled(QWidget* theWrappedObject) const;
   bool  isEnabledTo(QWidget* theWrappedObject, QWidget*  arg__1) const;
   bool  isFullScreen(QWidget* theWrappedObject) const;
   bool  isHidden(QWidget* theWrappedObject) const;
   bool  isLeftToRight(QWidget* theWrappedObject) const;
   bool  isMaximized(QWidget* theWrappedObject) const;
   bool  isMinimized(QWidget* theWrappedObject) const;
   bool  isModal(QWidget* theWrappedObject) const;
   bool  isRightToLeft(QWidget* theWrappedObject) const;
   bool  isVisible(QWidget* theWrappedObject) const;
   bool  isVisibleTo(QWidget* theWrappedObject, QWidget*  arg__1) const;
   bool  isWindow(QWidget* theWrappedObject) const;
   bool  isWindowModified(QWidget* theWrappedObject) const;
   void keyPressEvent(QWidget* theWrappedObject, QKeyEvent*  arg__1);
   void keyReleaseEvent(QWidget* theWrappedObject, QKeyEvent*  arg__1);
   QWidget*  static_QWidget_keyboardGrabber();
   void languageChange(QWidget* theWrappedObject);
   QLayout*  layout(QWidget* theWrappedObject) const;
   Qt::LayoutDirection  layoutDirection(QWidget* theWrappedObject) const;
   void leaveEvent(QWidget* theWrappedObject, QEvent*  arg__1);
   QLocale  locale(QWidget* theWrappedObject) const;
   QPoint  mapFrom(QWidget* theWrappedObject, QWidget*  arg__1, const QPoint&  arg__2) const;
   QPoint  mapFromGlobal(QWidget* theWrappedObject, const QPoint&  arg__1) const;
   QPoint  mapFromParent(QWidget* theWrappedObject, const QPoint&  arg__1) const;
   QPoint  mapTo(QWidget* theWrappedObject, QWidget*  arg__1, const QPoint&  arg__2) const;
   QPoint  mapToGlobal(QWidget* theWrappedObject, const QPoint&  arg__1) const;
   QPoint  mapToParent(QWidget* theWrappedObject, const QPoint&  arg__1) const;
   QRegion  mask(QWidget* theWrappedObject) const;
   int  maximumHeight(QWidget* theWrappedObject) const;
   QSize  maximumSize(QWidget* theWrappedObject) const;
   int  maximumWidth(QWidget* theWrappedObject) const;
   int  metric(QWidget* theWrappedObject, QPaintDevice::PaintDeviceMetric  arg__1) const;
   int  minimumHeight(QWidget* theWrappedObject) const;
   QSize  minimumSize(QWidget* theWrappedObject) const;
   QSize  minimumSizeHint(QWidget* theWrappedObject) const;
   int  minimumWidth(QWidget* theWrappedObject) const;
   void mouseDoubleClickEvent(QWidget* theWrappedObject, QMouseEvent*  arg__1);
   QWidget*  static_QWidget_mouseGrabber();
   void mouseMoveEvent(QWidget* theWrappedObject, QMouseEvent*  arg__1);
   void mousePressEvent(QWidget* theWrappedObject, QMouseEvent*  arg__1);
   void mouseReleaseEvent(QWidget* theWrappedObject, QMouseEvent*  arg__1);
   void move(QWidget* theWrappedObject, const QPoint&  arg__1);
   void move(QWidget* theWrappedObject, int  x, int  y);
   void moveEvent(QWidget* theWrappedObject, QMoveEvent*  arg__1);
   QWidget*  nativeParentWidget(QWidget* theWrappedObject) const;
   QWidget*  nextInFocusChain(QWidget* theWrappedObject) const;
   QRect  normalGeometry(QWidget* theWrappedObject) const;
   void overrideWindowFlags(QWidget* theWrappedObject, Qt::WindowFlags  type);
   void overrideWindowState(QWidget* theWrappedObject, Qt::WindowStates  state);
   QPaintEngine*  paintEngine(QWidget* theWrappedObject) const;
   void paintEvent(QWidget* theWrappedObject, QPaintEvent*  arg__1);
   const QPalette*  palette(QWidget* theWrappedObject) const;
   QWidget*  parentWidget(QWidget* theWrappedObject) const;
   QPoint  pos(QWidget* theWrappedObject) const;
   QWidget*  previousInFocusChain(QWidget* theWrappedObject) const;
   QRect  rect(QWidget* theWrappedObject) const;
   void releaseKeyboard(QWidget* theWrappedObject);
   void releaseMouse(QWidget* theWrappedObject);
   void releaseShortcut(QWidget* theWrappedObject, int  id);
   void removeAction(QWidget* theWrappedObject, QAction*  action);
   void render(QWidget* theWrappedObject, QPaintDevice*  target, const QPoint&  targetOffset = QPoint(), const QRegion&  sourceRegion = QRegion(), QWidget::RenderFlags  renderFlags = QWidget::RenderFlags(DrawWindowBackground | DrawChildren));
   void render(QWidget* theWrappedObject, QPainter*  painter, const QPoint&  targetOffset = QPoint(), const QRegion&  sourceRegion = QRegion(), QWidget::RenderFlags  renderFlags = QWidget::RenderFlags(DrawWindowBackground | DrawChildren));
   void repaint(QWidget* theWrappedObject, const QRect&  arg__1);
   void repaint(QWidget* theWrappedObject, const QRegion&  arg__1);
   void repaint(QWidget* theWrappedObject, int  x, int  y, int  w, int  h);
   void resize(QWidget* theWrappedObject, const QSize&  arg__1);
   void resize(QWidget* theWrappedObject, int  w, int  h);
   void resizeEvent(QWidget* theWrappedObject, QResizeEvent*  arg__1);
   bool  restoreGeometry(QWidget* theWrappedObject, const QByteArray&  geometry);
   QByteArray  saveGeometry(QWidget* theWrappedObject) const;
   void scroll(QWidget* theWrappedObject, int  dx, int  dy);
   void scroll(QWidget* theWrappedObject, int  dx, int  dy, const QRect&  arg__3);
   void setAcceptDrops(QWidget* theWrappedObject, bool  on);
   void setAccessibleDescription(QWidget* theWrappedObject, const QString&  description);
   void setAccessibleName(QWidget* theWrappedObject, const QString&  name);
   void setAttribute(QWidget* theWrappedObject, Qt::WidgetAttribute  arg__1, bool  on = true);
   void setAutoFillBackground(QWidget* theWrappedObject, bool  enabled);
   void setBackgroundRole(QWidget* theWrappedObject, QPalette::ColorRole  arg__1);
   void setBaseSize(QWidget* theWrappedObject, const QSize&  arg__1);
   void setBaseSize(QWidget* theWrappedObject, int  basew, int  baseh);
   void setContentsMargins(QWidget* theWrappedObject, const QMargins&  margins);
   void setContentsMargins(QWidget* theWrappedObject, int  left, int  top, int  right, int  bottom);
   void setContextMenuPolicy(QWidget* theWrappedObject, Qt::ContextMenuPolicy  policy);
   void setCursor(QWidget* theWrappedObject, const QCursor&  arg__1);
   void setFixedHeight(QWidget* theWrappedObject, int  h);
   void setFixedSize(QWidget* theWrappedObject, const QSize&  arg__1);
   void setFixedSize(QWidget* theWrappedObject, int  w, int  h);
   void setFixedWidth(QWidget* theWrappedObject, int  w);
   void setFocus(QWidget* theWrappedObject, Qt::FocusReason  reason);
   void setFocusPolicy(QWidget* theWrappedObject, Qt::FocusPolicy  policy);
   void setFocusProxy(QWidget* theWrappedObject, QWidget*  arg__1);
   void setFont(QWidget* theWrappedObject, const QFont&  arg__1);
   void setForegroundRole(QWidget* theWrappedObject, QPalette::ColorRole  arg__1);
   void setGeometry(QWidget* theWrappedObject, const QRect&  arg__1);
   void setGeometry(QWidget* theWrappedObject, int  x, int  y, int  w, int  h);
   void setGraphicsEffect(QWidget* theWrappedObject, QGraphicsEffect*  effect);
   void setInputContext(QWidget* theWrappedObject, QInputContext*  arg__1);
   void setInputMethodHints(QWidget* theWrappedObject, Qt::InputMethodHints  hints);
   void setLayout(QWidget* theWrappedObject, QLayout*  arg__1);
   void setLayoutDirection(QWidget* theWrappedObject, Qt::LayoutDirection  direction);
   void setLocale(QWidget* theWrappedObject, const QLocale&  locale);
   void setMask(QWidget* theWrappedObject, const QBitmap&  arg__1);
   void setMask(QWidget* theWrappedObject, const QRegion&  arg__1);
   void setMaximumHeight(QWidget* theWrappedObject, int  maxh);
   void setMaximumSize(QWidget* theWrappedObject, const QSize&  arg__1);
   void setMaximumSize(QWidget* theWrappedObject, int  maxw, int  maxh);
   void setMaximumWidth(QWidget* theWrappedObject, int  maxw);
   void setMinimumHeight(QWidget* theWrappedObject, int  minh);
   void setMinimumSize(QWidget* theWrappedObject, const QSize&  arg__1);
   void setMinimumSize(QWidget* theWrappedObject, int  minw, int  minh);
   void setMinimumWidth(QWidget* theWrappedObject, int  minw);
   void setMouseTracking(QWidget* theWrappedObject, bool  enable);
   void setPalette(QWidget* theWrappedObject, const QPalette&  arg__1);
   void setParent(QWidget* theWrappedObject, QWidget*  parent);
   void setParent(QWidget* theWrappedObject, QWidget*  parent, Qt::WindowFlags  f);
   void setShortcutAutoRepeat(QWidget* theWrappedObject, int  id, bool  enable = true);
   void setShortcutEnabled(QWidget* theWrappedObject, int  id, bool  enable = true);
   void setSizeIncrement(QWidget* theWrappedObject, const QSize&  arg__1);
   void setSizeIncrement(QWidget* theWrappedObject, int  w, int  h);
   void setSizePolicy(QWidget* theWrappedObject, QSizePolicy  arg__1);
   void setSizePolicy(QWidget* theWrappedObject, QSizePolicy::Policy  horizontal, QSizePolicy::Policy  vertical);
   void setStatusTip(QWidget* theWrappedObject, const QString&  arg__1);
   void setStyle(QWidget* theWrappedObject, QStyle*  arg__1);
   void static_QWidget_setTabOrder(QWidget*  arg__1, QWidget*  arg__2);
   void setToolTip(QWidget* theWrappedObject, const QString&  arg__1);
   void setUpdatesEnabled(QWidget* theWrappedObject, bool  enable);
   void setWhatsThis(QWidget* theWrappedObject, const QString&  arg__1);
   void setWindowFilePath(QWidget* theWrappedObject, const QString&  filePath);
   void setWindowFlags(QWidget* theWrappedObject, Qt::WindowFlags  type);
   void setWindowIcon(QWidget* theWrappedObject, const QIcon&  icon);
   void setWindowIconText(QWidget* theWrappedObject, const QString&  arg__1);
   void setWindowModality(QWidget* theWrappedObject, Qt::WindowModality  windowModality);
   void setWindowOpacity(QWidget* theWrappedObject, qreal  level);
   void setWindowRole(QWidget* theWrappedObject, const QString&  arg__1);
   void setWindowState(QWidget* theWrappedObject, Qt::WindowStates  state);
   void showEvent(QWidget* theWrappedObject, QShowEvent*  arg__1);
   QSize  size(QWidget* theWrappedObject) const;
   QSize  sizeHint(QWidget* theWrappedObject) const;
   QSize  sizeIncrement(QWidget* theWrappedObject) const;
   QSizePolicy  sizePolicy(QWidget* theWrappedObject) const;
   void stackUnder(QWidget* theWrappedObject, QWidget*  arg__1);
   QString  statusTip(QWidget* theWrappedObject) const;
   QStyle*  style(QWidget* theWrappedObject) const;
   QString  styleSheet(QWidget* theWrappedObject) const;
   void tabletEvent(QWidget* theWrappedObject, QTabletEvent*  arg__1);
   bool  testAttribute(QWidget* theWrappedObject, Qt::WidgetAttribute  arg__1) const;
   QString  toolTip(QWidget* theWrappedObject) const;
   bool  underMouse(QWidget* theWrappedObject) const;
   void unsetCursor(QWidget* theWrappedObject);
   void unsetLayoutDirection(QWidget* theWrappedObject);
   void unsetLocale(QWidget* theWrappedObject);
   void update(QWidget* theWrappedObject, const QRect&  arg__1);
   void update(QWidget* theWrappedObject, const QRegion&  arg__1);
   void update(QWidget* theWrappedObject, int  x, int  y, int  w, int  h);
   void updateGeometry(QWidget* theWrappedObject);
   bool  updatesEnabled(QWidget* theWrappedObject) const;
   QRegion  visibleRegion(QWidget* theWrappedObject) const;
   QString  whatsThis(QWidget* theWrappedObject) const;
   void wheelEvent(QWidget* theWrappedObject, QWheelEvent*  arg__1);
   int  width(QWidget* theWrappedObject) const;
   WId  winId(QWidget* theWrappedObject) const;
   QWidget*  window(QWidget* theWrappedObject) const;
   QString  windowFilePath(QWidget* theWrappedObject) const;
   Qt::WindowFlags  windowFlags(QWidget* theWrappedObject) const;
   QIcon  windowIcon(QWidget* theWrappedObject) const;
   QString  windowIconText(QWidget* theWrappedObject) const;
   Qt::WindowModality  windowModality(QWidget* theWrappedObject) const;
   qreal  windowOpacity(QWidget* theWrappedObject) const;
   QString  windowRole(QWidget* theWrappedObject) const;
   Qt::WindowStates  windowState(QWidget* theWrappedObject) const;
   QString  windowTitle(QWidget* theWrappedObject) const;
   Qt::WindowType  windowType(QWidget* theWrappedObject) const;
   int  x(QWidget* theWrappedObject) const;
   int  y(QWidget* theWrappedObject) const;
};





class PythonQtShell_QWidgetAction : public QWidgetAction
{
public:
    PythonQtShell_QWidgetAction(QObject*  parent):QWidgetAction(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual QWidget*  createWidget(QWidget*  parent);
virtual void customEvent(QEvent*  arg__1);
virtual void deleteWidget(QWidget*  widget);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QWidgetAction : public QWidgetAction
{ public:
inline QWidget*  promoted_createWidget(QWidget*  parent) { return QWidgetAction::createWidget(parent); }
inline void promoted_deleteWidget(QWidget*  widget) { QWidgetAction::deleteWidget(widget); }
inline bool  promoted_event(QEvent*  arg__1) { return QWidgetAction::event(arg__1); }
inline bool  promoted_eventFilter(QObject*  arg__1, QEvent*  arg__2) { return QWidgetAction::eventFilter(arg__1, arg__2); }
};

class PythonQtWrapper_QWidgetAction : public QObject
{ Q_OBJECT
public:
public slots:
QWidgetAction* new_QWidgetAction(QObject*  parent);
void delete_QWidgetAction(QWidgetAction* obj) { delete obj; } 
   QWidget*  createWidget(QWidgetAction* theWrappedObject, QWidget*  parent);
   QWidget*  defaultWidget(QWidgetAction* theWrappedObject) const;
   void deleteWidget(QWidgetAction* theWrappedObject, QWidget*  widget);
   bool  event(QWidgetAction* theWrappedObject, QEvent*  arg__1);
   bool  eventFilter(QWidgetAction* theWrappedObject, QObject*  arg__1, QEvent*  arg__2);
   void releaseWidget(QWidgetAction* theWrappedObject, QWidget*  widget);
   QWidget*  requestWidget(QWidgetAction* theWrappedObject, QWidget*  parent);
   void setDefaultWidget(QWidgetAction* theWrappedObject, QWidget*  w);
};





class PythonQtShell_QWidgetItem : public QWidgetItem
{
public:
    PythonQtShell_QWidgetItem(QWidget*  w):QWidgetItem(w),_wrapper(NULL) {};

virtual Qt::Orientations  expandingDirections() const;
virtual QRect  geometry() const;
virtual bool  hasHeightForWidth() const;
virtual int  heightForWidth(int  arg__1) const;
virtual void invalidate();
virtual bool  isEmpty() const;
virtual QLayout*  layout();
virtual QSize  maximumSize() const;
virtual int  minimumHeightForWidth(int  arg__1) const;
virtual QSize  minimumSize() const;
virtual void setGeometry(const QRect&  arg__1);
virtual QSize  sizeHint() const;
virtual QSpacerItem*  spacerItem();
virtual QWidget*  widget();

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QWidgetItem : public QWidgetItem
{ public:
inline Qt::Orientations  promoted_expandingDirections() const { return QWidgetItem::expandingDirections(); }
inline QRect  promoted_geometry() const { return QWidgetItem::geometry(); }
inline bool  promoted_hasHeightForWidth() const { return QWidgetItem::hasHeightForWidth(); }
inline int  promoted_heightForWidth(int  arg__1) const { return QWidgetItem::heightForWidth(arg__1); }
inline bool  promoted_isEmpty() const { return QWidgetItem::isEmpty(); }
inline QSize  promoted_maximumSize() const { return QWidgetItem::maximumSize(); }
inline QSize  promoted_minimumSize() const { return QWidgetItem::minimumSize(); }
inline void promoted_setGeometry(const QRect&  arg__1) { QWidgetItem::setGeometry(arg__1); }
inline QSize  promoted_sizeHint() const { return QWidgetItem::sizeHint(); }
inline QWidget*  promoted_widget() { return QWidgetItem::widget(); }
};

class PythonQtWrapper_QWidgetItem : public QObject
{ Q_OBJECT
public:
public slots:
QWidgetItem* new_QWidgetItem(QWidget*  w);
void delete_QWidgetItem(QWidgetItem* obj) { delete obj; } 
   Qt::Orientations  expandingDirections(QWidgetItem* theWrappedObject) const;
   QRect  geometry(QWidgetItem* theWrappedObject) const;
   bool  hasHeightForWidth(QWidgetItem* theWrappedObject) const;
   int  heightForWidth(QWidgetItem* theWrappedObject, int  arg__1) const;
   bool  isEmpty(QWidgetItem* theWrappedObject) const;
   QSize  maximumSize(QWidgetItem* theWrappedObject) const;
   QSize  minimumSize(QWidgetItem* theWrappedObject) const;
   void setGeometry(QWidgetItem* theWrappedObject, const QRect&  arg__1);
   QSize  sizeHint(QWidgetItem* theWrappedObject) const;
   QWidget*  widget(QWidgetItem* theWrappedObject);
};





class PythonQtWrapper_QWindowStateChangeEvent : public QObject
{ Q_OBJECT
public:
public slots:
QWindowStateChangeEvent* new_QWindowStateChangeEvent(Qt::WindowStates  aOldState);
QWindowStateChangeEvent* new_QWindowStateChangeEvent(Qt::WindowStates  aOldState, bool  isOverride);
void delete_QWindowStateChangeEvent(QWindowStateChangeEvent* obj) { delete obj; } 
   bool  isOverride(QWindowStateChangeEvent* theWrappedObject) const;
   Qt::WindowStates  oldState(QWindowStateChangeEvent* theWrappedObject) const;
};





class PythonQtShell_QWindowsStyle : public QWindowsStyle
{
public:
    PythonQtShell_QWindowsStyle():QWindowsStyle(),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual void drawComplexControl(QStyle::ComplexControl  cc, const QStyleOptionComplex*  opt, QPainter*  p, const QWidget*  w = 0) const;
virtual void drawControl(QStyle::ControlElement  element, const QStyleOption*  opt, QPainter*  p, const QWidget*  w = 0) const;
virtual void drawItemPixmap(QPainter*  painter, const QRect&  rect, int  alignment, const QPixmap&  pixmap) const;
virtual void drawItemText(QPainter*  painter, const QRect&  rect, int  flags, const QPalette&  pal, bool  enabled, const QString&  text, QPalette::ColorRole  textRole) const;
virtual void drawPrimitive(QStyle::PrimitiveElement  pe, const QStyleOption*  opt, QPainter*  p, const QWidget*  w = 0) const;
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  o, QEvent*  e);
virtual QPixmap  generatedIconPixmap(QIcon::Mode  iconMode, const QPixmap&  pixmap, const QStyleOption*  opt) const;
virtual QStyle::SubControl  hitTestComplexControl(QStyle::ComplexControl  cc, const QStyleOptionComplex*  opt, const QPoint&  pt, const QWidget*  w) const;
virtual QRect  itemPixmapRect(const QRect&  r, int  flags, const QPixmap&  pixmap) const;
virtual int  pixelMetric(QStyle::PixelMetric  pm, const QStyleOption*  option = 0, const QWidget*  widget = 0) const;
virtual void polish(QApplication*  arg__1);
virtual void polish(QPalette&  arg__1);
virtual void polish(QWidget*  arg__1);
virtual QSize  sizeFromContents(QStyle::ContentsType  ct, const QStyleOption*  opt, const QSize&  contentsSize, const QWidget*  widget = 0) const;
virtual QPalette  standardPalette() const;
virtual QPixmap  standardPixmap(QStyle::StandardPixmap  standardPixmap, const QStyleOption*  opt, const QWidget*  widget) const;
virtual int  styleHint(QStyle::StyleHint  hint, const QStyleOption*  opt = 0, const QWidget*  widget = 0, QStyleHintReturn*  returnData = 0) const;
virtual QRect  subControlRect(QStyle::ComplexControl  cc, const QStyleOptionComplex*  opt, QStyle::SubControl  sc, const QWidget*  w) const;
virtual QRect  subElementRect(QStyle::SubElement  r, const QStyleOption*  opt, const QWidget*  widget = 0) const;
virtual void timerEvent(QTimerEvent*  event);
virtual void unpolish(QApplication*  arg__1);
virtual void unpolish(QWidget*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QWindowsStyle : public QWindowsStyle
{ public:
inline void promoted_drawComplexControl(QStyle::ComplexControl  cc, const QStyleOptionComplex*  opt, QPainter*  p, const QWidget*  w = 0) const { QWindowsStyle::drawComplexControl(cc, opt, p, w); }
inline void promoted_drawControl(QStyle::ControlElement  element, const QStyleOption*  opt, QPainter*  p, const QWidget*  w = 0) const { QWindowsStyle::drawControl(element, opt, p, w); }
inline void promoted_drawPrimitive(QStyle::PrimitiveElement  pe, const QStyleOption*  opt, QPainter*  p, const QWidget*  w = 0) const { QWindowsStyle::drawPrimitive(pe, opt, p, w); }
inline bool  promoted_eventFilter(QObject*  o, QEvent*  e) { return QWindowsStyle::eventFilter(o, e); }
inline int  promoted_pixelMetric(QStyle::PixelMetric  pm, const QStyleOption*  option = 0, const QWidget*  widget = 0) const { return QWindowsStyle::pixelMetric(pm, option, widget); }
inline void promoted_polish(QApplication*  arg__1) { QWindowsStyle::polish(arg__1); }
inline void promoted_polish(QPalette&  arg__1) { QWindowsStyle::polish(arg__1); }
inline void promoted_polish(QWidget*  arg__1) { QWindowsStyle::polish(arg__1); }
inline QSize  promoted_sizeFromContents(QStyle::ContentsType  ct, const QStyleOption*  opt, const QSize&  contentsSize, const QWidget*  widget = 0) const { return QWindowsStyle::sizeFromContents(ct, opt, contentsSize, widget); }
inline int  promoted_styleHint(QStyle::StyleHint  hint, const QStyleOption*  opt = 0, const QWidget*  widget = 0, QStyleHintReturn*  returnData = 0) const { return QWindowsStyle::styleHint(hint, opt, widget, returnData); }
inline QRect  promoted_subElementRect(QStyle::SubElement  r, const QStyleOption*  opt, const QWidget*  widget = 0) const { return QWindowsStyle::subElementRect(r, opt, widget); }
inline void promoted_timerEvent(QTimerEvent*  event) { QWindowsStyle::timerEvent(event); }
inline void promoted_unpolish(QApplication*  arg__1) { QWindowsStyle::unpolish(arg__1); }
inline void promoted_unpolish(QWidget*  arg__1) { QWindowsStyle::unpolish(arg__1); }
};

class PythonQtWrapper_QWindowsStyle : public QObject
{ Q_OBJECT
public:
public slots:
QWindowsStyle* new_QWindowsStyle();
void delete_QWindowsStyle(QWindowsStyle* obj) { delete obj; } 
   void drawComplexControl(QWindowsStyle* theWrappedObject, QStyle::ComplexControl  cc, const QStyleOptionComplex*  opt, QPainter*  p, const QWidget*  w = 0) const;
   void drawControl(QWindowsStyle* theWrappedObject, QStyle::ControlElement  element, const QStyleOption*  opt, QPainter*  p, const QWidget*  w = 0) const;
   void drawPrimitive(QWindowsStyle* theWrappedObject, QStyle::PrimitiveElement  pe, const QStyleOption*  opt, QPainter*  p, const QWidget*  w = 0) const;
   bool  eventFilter(QWindowsStyle* theWrappedObject, QObject*  o, QEvent*  e);
   int  pixelMetric(QWindowsStyle* theWrappedObject, QStyle::PixelMetric  pm, const QStyleOption*  option = 0, const QWidget*  widget = 0) const;
   void polish(QWindowsStyle* theWrappedObject, QApplication*  arg__1);
   void polish(QWindowsStyle* theWrappedObject, QPalette&  arg__1);
   void polish(QWindowsStyle* theWrappedObject, QWidget*  arg__1);
   QSize  sizeFromContents(QWindowsStyle* theWrappedObject, QStyle::ContentsType  ct, const QStyleOption*  opt, const QSize&  contentsSize, const QWidget*  widget = 0) const;
   int  styleHint(QWindowsStyle* theWrappedObject, QStyle::StyleHint  hint, const QStyleOption*  opt = 0, const QWidget*  widget = 0, QStyleHintReturn*  returnData = 0) const;
   QRect  subElementRect(QWindowsStyle* theWrappedObject, QStyle::SubElement  r, const QStyleOption*  opt, const QWidget*  widget = 0) const;
   void timerEvent(QWindowsStyle* theWrappedObject, QTimerEvent*  event);
   void unpolish(QWindowsStyle* theWrappedObject, QApplication*  arg__1);
   void unpolish(QWindowsStyle* theWrappedObject, QWidget*  arg__1);
};





class PythonQtShell_QWizard : public QWizard
{
public:
    PythonQtShell_QWizard(QWidget*  parent = 0, Qt::WindowFlags  flags = 0):QWizard(parent, flags),_wrapper(NULL) {};

virtual void accept();
virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  arg__1);
virtual void childEvent(QChildEvent*  arg__1);
virtual void cleanupPage(int  id);
virtual void closeEvent(QCloseEvent*  arg__1);
virtual void contextMenuEvent(QContextMenuEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual int  devType() const;
virtual void done(int  result);
virtual void dragEnterEvent(QDragEnterEvent*  arg__1);
virtual void dragLeaveEvent(QDragLeaveEvent*  arg__1);
virtual void dragMoveEvent(QDragMoveEvent*  arg__1);
virtual void dropEvent(QDropEvent*  arg__1);
virtual void enterEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  event);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void focusInEvent(QFocusEvent*  arg__1);
virtual bool  focusNextPrevChild(bool  next);
virtual void focusOutEvent(QFocusEvent*  arg__1);
virtual int  heightForWidth(int  arg__1) const;
virtual void hideEvent(QHideEvent*  arg__1);
virtual void initializePage(int  id);
virtual void inputMethodEvent(QInputMethodEvent*  arg__1);
virtual QVariant  inputMethodQuery(Qt::InputMethodQuery  arg__1) const;
virtual void keyPressEvent(QKeyEvent*  arg__1);
virtual void keyReleaseEvent(QKeyEvent*  arg__1);
virtual void languageChange();
virtual void leaveEvent(QEvent*  arg__1);
virtual int  metric(QPaintDevice::PaintDeviceMetric  arg__1) const;
virtual void mouseDoubleClickEvent(QMouseEvent*  arg__1);
virtual void mouseMoveEvent(QMouseEvent*  arg__1);
virtual void mousePressEvent(QMouseEvent*  arg__1);
virtual void mouseReleaseEvent(QMouseEvent*  arg__1);
virtual void moveEvent(QMoveEvent*  arg__1);
virtual int  nextId() const;
virtual QPaintEngine*  paintEngine() const;
virtual void paintEvent(QPaintEvent*  event);
virtual void reject();
virtual void resizeEvent(QResizeEvent*  event);
virtual void showEvent(QShowEvent*  arg__1);
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual bool  validateCurrentPage();
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QWizard : public QWizard
{ public:
inline void promoted_cleanupPage(int  id) { QWizard::cleanupPage(id); }
inline void promoted_done(int  result) { QWizard::done(result); }
inline bool  promoted_event(QEvent*  event) { return QWizard::event(event); }
inline void promoted_initializePage(int  id) { QWizard::initializePage(id); }
inline int  promoted_nextId() const { return QWizard::nextId(); }
inline void promoted_paintEvent(QPaintEvent*  event) { QWizard::paintEvent(event); }
inline void promoted_resizeEvent(QResizeEvent*  event) { QWizard::resizeEvent(event); }
inline bool  promoted_validateCurrentPage() { return QWizard::validateCurrentPage(); }
};

class PythonQtWrapper_QWizard : public QObject
{ Q_OBJECT
public:
Q_ENUMS(WizardButton WizardPixmap )
enum WizardButton{
  BackButton = QWizard::BackButton,   NextButton = QWizard::NextButton,   CommitButton = QWizard::CommitButton,   FinishButton = QWizard::FinishButton,   CancelButton = QWizard::CancelButton,   HelpButton = QWizard::HelpButton,   CustomButton1 = QWizard::CustomButton1,   CustomButton2 = QWizard::CustomButton2,   CustomButton3 = QWizard::CustomButton3,   Stretch = QWizard::Stretch,   NoButton = QWizard::NoButton,   NStandardButtons = QWizard::NStandardButtons,   NButtons = QWizard::NButtons};
enum WizardPixmap{
  WatermarkPixmap = QWizard::WatermarkPixmap,   LogoPixmap = QWizard::LogoPixmap,   BannerPixmap = QWizard::BannerPixmap,   BackgroundPixmap = QWizard::BackgroundPixmap,   NPixmaps = QWizard::NPixmaps};
public slots:
QWizard* new_QWizard(QWidget*  parent = 0, Qt::WindowFlags  flags = 0);
void delete_QWizard(QWizard* obj) { delete obj; } 
   int  addPage(QWizard* theWrappedObject, QWizardPage*  page);
   QAbstractButton*  button(QWizard* theWrappedObject, QWizard::WizardButton  which) const;
   QString  buttonText(QWizard* theWrappedObject, QWizard::WizardButton  which) const;
   void cleanupPage(QWizard* theWrappedObject, int  id);
   int  currentId(QWizard* theWrappedObject) const;
   QWizardPage*  currentPage(QWizard* theWrappedObject) const;
   void done(QWizard* theWrappedObject, int  result);
   bool  event(QWizard* theWrappedObject, QEvent*  event);
   QVariant  field(QWizard* theWrappedObject, const QString&  name) const;
   bool  hasVisitedPage(QWizard* theWrappedObject, int  id) const;
   void initializePage(QWizard* theWrappedObject, int  id);
   int  nextId(QWizard* theWrappedObject) const;
   QWizard::WizardOptions  options(QWizard* theWrappedObject) const;
   QWizardPage*  page(QWizard* theWrappedObject, int  id) const;
   QList<int >  pageIds(QWizard* theWrappedObject) const;
   void paintEvent(QWizard* theWrappedObject, QPaintEvent*  event);
   QPixmap  pixmap(QWizard* theWrappedObject, QWizard::WizardPixmap  which) const;
   void removePage(QWizard* theWrappedObject, int  id);
   void resizeEvent(QWizard* theWrappedObject, QResizeEvent*  event);
   void setButton(QWizard* theWrappedObject, QWizard::WizardButton  which, QAbstractButton*  button);
   void setButtonLayout(QWizard* theWrappedObject, const QList<QWizard::WizardButton >&  layout);
   void setButtonText(QWizard* theWrappedObject, QWizard::WizardButton  which, const QString&  text);
   void setField(QWizard* theWrappedObject, const QString&  name, const QVariant&  value);
   void setOption(QWizard* theWrappedObject, QWizard::WizardOption  option, bool  on = true);
   void setOptions(QWizard* theWrappedObject, QWizard::WizardOptions  options);
   void setPage(QWizard* theWrappedObject, int  id, QWizardPage*  page);
   void setPixmap(QWizard* theWrappedObject, QWizard::WizardPixmap  which, const QPixmap&  pixmap);
   void setStartId(QWizard* theWrappedObject, int  id);
   void setSubTitleFormat(QWizard* theWrappedObject, Qt::TextFormat  format);
   void setTitleFormat(QWizard* theWrappedObject, Qt::TextFormat  format);
   void setVisible(QWizard* theWrappedObject, bool  visible);
   void setWizardStyle(QWizard* theWrappedObject, QWizard::WizardStyle  style);
   QSize  sizeHint(QWizard* theWrappedObject) const;
   int  startId(QWizard* theWrappedObject) const;
   Qt::TextFormat  subTitleFormat(QWizard* theWrappedObject) const;
   bool  testOption(QWizard* theWrappedObject, QWizard::WizardOption  option) const;
   Qt::TextFormat  titleFormat(QWizard* theWrappedObject) const;
   bool  validateCurrentPage(QWizard* theWrappedObject);
   QList<int >  visitedPages(QWizard* theWrappedObject) const;
   QWizard::WizardStyle  wizardStyle(QWizard* theWrappedObject) const;
};





class PythonQtShell_QWizardPage : public QWizardPage
{
public:
    PythonQtShell_QWizardPage(QWidget*  parent = 0):QWizardPage(parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  arg__1);
virtual void childEvent(QChildEvent*  arg__1);
virtual void cleanupPage();
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
virtual void initializePage();
virtual void inputMethodEvent(QInputMethodEvent*  arg__1);
virtual QVariant  inputMethodQuery(Qt::InputMethodQuery  arg__1) const;
virtual bool  isComplete() const;
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
virtual int  nextId() const;
virtual QPaintEngine*  paintEngine() const;
virtual void paintEvent(QPaintEvent*  arg__1);
virtual void resizeEvent(QResizeEvent*  arg__1);
virtual void showEvent(QShowEvent*  arg__1);
virtual QSize  sizeHint() const;
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual bool  validatePage();
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QWizardPage : public QWizardPage
{ public:
inline void promoted_cleanupPage() { QWizardPage::cleanupPage(); }
inline void promoted_initializePage() { QWizardPage::initializePage(); }
inline bool  promoted_isComplete() const { return QWizardPage::isComplete(); }
inline int  promoted_nextId() const { return QWizardPage::nextId(); }
inline bool  promoted_validatePage() { return QWizardPage::validatePage(); }
};

class PythonQtWrapper_QWizardPage : public QObject
{ Q_OBJECT
public:
public slots:
QWizardPage* new_QWizardPage(QWidget*  parent = 0);
void delete_QWizardPage(QWizardPage* obj) { delete obj; } 
   QString  buttonText(QWizardPage* theWrappedObject, QWizard::WizardButton  which) const;
   void cleanupPage(QWizardPage* theWrappedObject);
   void initializePage(QWizardPage* theWrappedObject);
   bool  isCommitPage(QWizardPage* theWrappedObject) const;
   bool  isComplete(QWizardPage* theWrappedObject) const;
   bool  isFinalPage(QWizardPage* theWrappedObject) const;
   int  nextId(QWizardPage* theWrappedObject) const;
   QPixmap  pixmap(QWizardPage* theWrappedObject, QWizard::WizardPixmap  which) const;
   void setButtonText(QWizardPage* theWrappedObject, QWizard::WizardButton  which, const QString&  text);
   void setCommitPage(QWizardPage* theWrappedObject, bool  commitPage);
   void setFinalPage(QWizardPage* theWrappedObject, bool  finalPage);
   void setPixmap(QWizardPage* theWrappedObject, QWizard::WizardPixmap  which, const QPixmap&  pixmap);
   void setSubTitle(QWizardPage* theWrappedObject, const QString&  subTitle);
   void setTitle(QWizardPage* theWrappedObject, const QString&  title);
   QString  subTitle(QWizardPage* theWrappedObject) const;
   QString  title(QWizardPage* theWrappedObject) const;
   bool  validatePage(QWizardPage* theWrappedObject);
};





class PythonQtShell_QWorkspace : public QWorkspace
{
public:
    PythonQtShell_QWorkspace(QWidget*  parent = 0):QWorkspace(parent),_wrapper(NULL) {};

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
virtual void hideEvent(QHideEvent*  e);
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
virtual void paintEvent(QPaintEvent*  e);
virtual void resizeEvent(QResizeEvent*  arg__1);
virtual void showEvent(QShowEvent*  e);
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void wheelEvent(QWheelEvent*  e);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QWorkspace : public QWorkspace
{ public:
inline void promoted_changeEvent(QEvent*  arg__1) { QWorkspace::changeEvent(arg__1); }
inline void promoted_childEvent(QChildEvent*  arg__1) { QWorkspace::childEvent(arg__1); }
inline bool  promoted_event(QEvent*  e) { return QWorkspace::event(e); }
inline bool  promoted_eventFilter(QObject*  arg__1, QEvent*  arg__2) { return QWorkspace::eventFilter(arg__1, arg__2); }
inline void promoted_hideEvent(QHideEvent*  e) { QWorkspace::hideEvent(e); }
inline void promoted_paintEvent(QPaintEvent*  e) { QWorkspace::paintEvent(e); }
inline void promoted_resizeEvent(QResizeEvent*  arg__1) { QWorkspace::resizeEvent(arg__1); }
inline void promoted_showEvent(QShowEvent*  e) { QWorkspace::showEvent(e); }
inline void promoted_wheelEvent(QWheelEvent*  e) { QWorkspace::wheelEvent(e); }
};

class PythonQtWrapper_QWorkspace : public QObject
{ Q_OBJECT
public:
Q_ENUMS(WindowOrder )
enum WindowOrder{
  CreationOrder = QWorkspace::CreationOrder,   StackingOrder = QWorkspace::StackingOrder};
public slots:
QWorkspace* new_QWorkspace(QWidget*  parent = 0);
void delete_QWorkspace(QWorkspace* obj) { delete obj; } 
   QWidget*  activeWindow(QWorkspace* theWrappedObject) const;
   QWidget*  addWindow(QWorkspace* theWrappedObject, QWidget*  w, Qt::WindowFlags  flags = 0);
   QBrush  background(QWorkspace* theWrappedObject) const;
   void changeEvent(QWorkspace* theWrappedObject, QEvent*  arg__1);
   void childEvent(QWorkspace* theWrappedObject, QChildEvent*  arg__1);
   bool  event(QWorkspace* theWrappedObject, QEvent*  e);
   bool  eventFilter(QWorkspace* theWrappedObject, QObject*  arg__1, QEvent*  arg__2);
   void hideEvent(QWorkspace* theWrappedObject, QHideEvent*  e);
   void paintEvent(QWorkspace* theWrappedObject, QPaintEvent*  e);
   void resizeEvent(QWorkspace* theWrappedObject, QResizeEvent*  arg__1);
   bool  scrollBarsEnabled(QWorkspace* theWrappedObject) const;
   void setBackground(QWorkspace* theWrappedObject, const QBrush&  background);
   void setScrollBarsEnabled(QWorkspace* theWrappedObject, bool  enable);
   void showEvent(QWorkspace* theWrappedObject, QShowEvent*  e);
   QSize  sizeHint(QWorkspace* theWrappedObject) const;
   void wheelEvent(QWorkspace* theWrappedObject, QWheelEvent*  e);
   QList<QWidget* >  windowList(QWorkspace* theWrappedObject, QWorkspace::WindowOrder  order = QWorkspace::CreationOrder) const;
};


