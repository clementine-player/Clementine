#include <PythonQt.h>
#include <QObject>
#include <QPainterPath>
#include <QTextCursor>
#include <QVarLengthArray>
#include <QVariant>
#include <qabstractitemdelegate.h>
#include <qabstractitemmodel.h>
#include <qaction.h>
#include <qbitmap.h>
#include <qboxlayout.h>
#include <qbrush.h>
#include <qbytearray.h>
#include <qcolor.h>
#include <qcoreevent.h>
#include <qcursor.h>
#include <qevent.h>
#include <qfont.h>
#include <qgraphicseffect.h>
#include <qgraphicsitem.h>
#include <qgraphicslayout.h>
#include <qgraphicslayoutitem.h>
#include <qgraphicsproxywidget.h>
#include <qgraphicsscene.h>
#include <qgraphicssceneevent.h>
#include <qgraphicstransform.h>
#include <qgraphicsview.h>
#include <qgraphicswidget.h>
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qheaderview.h>
#include <qicon.h>
#include <qiconengine.h>
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
#include <qmimedata.h>
#include <qobject.h>
#include <qpaintdevice.h>
#include <qpaintengine.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qpalette.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpolygon.h>
#include <qrect.h>
#include <qregion.h>
#include <qscrollbar.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtextcursor.h>
#include <qtextdocument.h>
#include <qtransform.h>
#include <qvector3d.h>
#include <qwidget.h>



class PythonQtShell_QGraphicsPixmapItem : public QGraphicsPixmapItem
{
public:
    PythonQtShell_QGraphicsPixmapItem(QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0):QGraphicsPixmapItem(parent, scene),_wrapper(NULL) {};
    PythonQtShell_QGraphicsPixmapItem(const QPixmap&  pixmap, QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0):QGraphicsPixmapItem(pixmap, parent, scene),_wrapper(NULL) {};

virtual void advance(int  phase);
virtual QRectF  boundingRect() const;
virtual bool  collidesWithItem(const QGraphicsItem*  other, Qt::ItemSelectionMode  mode) const;
virtual bool  collidesWithPath(const QPainterPath&  path, Qt::ItemSelectionMode  mode) const;
virtual bool  contains(const QPointF&  point) const;
virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent*  event);
virtual void dragEnterEvent(QGraphicsSceneDragDropEvent*  event);
virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent*  event);
virtual void dragMoveEvent(QGraphicsSceneDragDropEvent*  event);
virtual void dropEvent(QGraphicsSceneDragDropEvent*  event);
virtual QVariant  extension(const QVariant&  variant) const;
virtual void focusInEvent(QFocusEvent*  event);
virtual void focusOutEvent(QFocusEvent*  event);
virtual void hoverEnterEvent(QGraphicsSceneHoverEvent*  event);
virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent*  event);
virtual void hoverMoveEvent(QGraphicsSceneHoverEvent*  event);
virtual void inputMethodEvent(QInputMethodEvent*  event);
virtual QVariant  inputMethodQuery(Qt::InputMethodQuery  query) const;
virtual bool  isObscuredBy(const QGraphicsItem*  item) const;
virtual QVariant  itemChange(QGraphicsItem::GraphicsItemChange  change, const QVariant&  value);
virtual void keyPressEvent(QKeyEvent*  event);
virtual void keyReleaseEvent(QKeyEvent*  event);
virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*  event);
virtual void mouseMoveEvent(QGraphicsSceneMouseEvent*  event);
virtual void mousePressEvent(QGraphicsSceneMouseEvent*  event);
virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*  event);
virtual QPainterPath  opaqueArea() const;
virtual void paint(QPainter*  painter, const QStyleOptionGraphicsItem*  option, QWidget*  widget);
virtual bool  sceneEvent(QEvent*  event);
virtual bool  sceneEventFilter(QGraphicsItem*  watched, QEvent*  event);
virtual QPainterPath  shape() const;
virtual int  type() const;
virtual void wheelEvent(QGraphicsSceneWheelEvent*  event);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QGraphicsPixmapItem : public QGraphicsPixmapItem
{ public:
inline QRectF  promoted_boundingRect() const { return QGraphicsPixmapItem::boundingRect(); }
inline bool  promoted_contains(const QPointF&  point) const { return QGraphicsPixmapItem::contains(point); }
inline QVariant  promoted_extension(const QVariant&  variant) const { return QGraphicsPixmapItem::extension(variant); }
inline bool  promoted_isObscuredBy(const QGraphicsItem*  item) const { return QGraphicsPixmapItem::isObscuredBy(item); }
inline QPainterPath  promoted_opaqueArea() const { return QGraphicsPixmapItem::opaqueArea(); }
inline void promoted_paint(QPainter*  painter, const QStyleOptionGraphicsItem*  option, QWidget*  widget) { QGraphicsPixmapItem::paint(painter, option, widget); }
inline QPainterPath  promoted_shape() const { return QGraphicsPixmapItem::shape(); }
inline int  promoted_type() const { return QGraphicsPixmapItem::type(); }
};

class PythonQtWrapper_QGraphicsPixmapItem : public QObject
{ Q_OBJECT
public:
Q_ENUMS(ShapeMode enum_1 )
enum ShapeMode{
  MaskShape = QGraphicsPixmapItem::MaskShape,   BoundingRectShape = QGraphicsPixmapItem::BoundingRectShape,   HeuristicMaskShape = QGraphicsPixmapItem::HeuristicMaskShape};
enum enum_1{
  Type = QGraphicsPixmapItem::Type};
public slots:
QGraphicsPixmapItem* new_QGraphicsPixmapItem(QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0);
QGraphicsPixmapItem* new_QGraphicsPixmapItem(const QPixmap&  pixmap, QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0);
void delete_QGraphicsPixmapItem(QGraphicsPixmapItem* obj) { delete obj; } 
   QRectF  boundingRect(QGraphicsPixmapItem* theWrappedObject) const;
   bool  contains(QGraphicsPixmapItem* theWrappedObject, const QPointF&  point) const;
   QVariant  extension(QGraphicsPixmapItem* theWrappedObject, const QVariant&  variant) const;
   bool  isObscuredBy(QGraphicsPixmapItem* theWrappedObject, const QGraphicsItem*  item) const;
   QPointF  offset(QGraphicsPixmapItem* theWrappedObject) const;
   QPainterPath  opaqueArea(QGraphicsPixmapItem* theWrappedObject) const;
   void paint(QGraphicsPixmapItem* theWrappedObject, QPainter*  painter, const QStyleOptionGraphicsItem*  option, QWidget*  widget);
   QPixmap  pixmap(QGraphicsPixmapItem* theWrappedObject) const;
   void setOffset(QGraphicsPixmapItem* theWrappedObject, const QPointF&  offset);
   void setOffset(QGraphicsPixmapItem* theWrappedObject, qreal  x, qreal  y);
   void setPixmap(QGraphicsPixmapItem* theWrappedObject, const QPixmap&  pixmap);
   void setShapeMode(QGraphicsPixmapItem* theWrappedObject, QGraphicsPixmapItem::ShapeMode  mode);
   void setTransformationMode(QGraphicsPixmapItem* theWrappedObject, Qt::TransformationMode  mode);
   QPainterPath  shape(QGraphicsPixmapItem* theWrappedObject) const;
   QGraphicsPixmapItem::ShapeMode  shapeMode(QGraphicsPixmapItem* theWrappedObject) const;
   Qt::TransformationMode  transformationMode(QGraphicsPixmapItem* theWrappedObject) const;
   int  type(QGraphicsPixmapItem* theWrappedObject) const;
};





class PythonQtShell_QGraphicsPolygonItem : public QGraphicsPolygonItem
{
public:
    PythonQtShell_QGraphicsPolygonItem(QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0):QGraphicsPolygonItem(parent, scene),_wrapper(NULL) {};
    PythonQtShell_QGraphicsPolygonItem(const QPolygonF&  polygon, QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0):QGraphicsPolygonItem(polygon, parent, scene),_wrapper(NULL) {};

virtual bool  isObscuredBy(const QGraphicsItem*  item) const;
virtual QPainterPath  opaqueArea() const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QGraphicsPolygonItem : public QGraphicsPolygonItem
{ public:
inline bool  promoted_isObscuredBy(const QGraphicsItem*  item) const { return QGraphicsPolygonItem::isObscuredBy(item); }
inline QPainterPath  promoted_opaqueArea() const { return QGraphicsPolygonItem::opaqueArea(); }
};

class PythonQtWrapper_QGraphicsPolygonItem : public QObject
{ Q_OBJECT
public:
Q_ENUMS(enum_1 )
enum enum_1{
  Type = QGraphicsPolygonItem::Type};
public slots:
QGraphicsPolygonItem* new_QGraphicsPolygonItem(QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0);
QGraphicsPolygonItem* new_QGraphicsPolygonItem(const QPolygonF&  polygon, QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0);
void delete_QGraphicsPolygonItem(QGraphicsPolygonItem* obj) { delete obj; } 
   QRectF  boundingRect(QGraphicsPolygonItem* theWrappedObject) const;
   bool  contains(QGraphicsPolygonItem* theWrappedObject, const QPointF&  point) const;
   Qt::FillRule  fillRule(QGraphicsPolygonItem* theWrappedObject) const;
   bool  isObscuredBy(QGraphicsPolygonItem* theWrappedObject, const QGraphicsItem*  item) const;
   QPainterPath  opaqueArea(QGraphicsPolygonItem* theWrappedObject) const;
   void paint(QGraphicsPolygonItem* theWrappedObject, QPainter*  painter, const QStyleOptionGraphicsItem*  option, QWidget*  widget = 0);
   QPolygonF  polygon(QGraphicsPolygonItem* theWrappedObject) const;
   void setFillRule(QGraphicsPolygonItem* theWrappedObject, Qt::FillRule  rule);
   void setPolygon(QGraphicsPolygonItem* theWrappedObject, const QPolygonF&  polygon);
   QPainterPath  shape(QGraphicsPolygonItem* theWrappedObject) const;
   int  type(QGraphicsPolygonItem* theWrappedObject) const;
};





class PythonQtShell_QGraphicsProxyWidget : public QGraphicsProxyWidget
{
public:
    PythonQtShell_QGraphicsProxyWidget(QGraphicsItem*  parent = 0, Qt::WindowFlags  wFlags = 0):QGraphicsProxyWidget(parent, wFlags),_wrapper(NULL) {};

virtual void changeEvent(QEvent*  event);
virtual void childEvent(QChildEvent*  arg__1);
virtual void closeEvent(QCloseEvent*  event);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  event);
virtual bool  eventFilter(QObject*  object, QEvent*  event);
virtual bool  focusNextPrevChild(bool  next);
virtual void getContentsMargins(qreal*  left, qreal*  top, qreal*  right, qreal*  bottom) const;
virtual void grabKeyboardEvent(QEvent*  event);
virtual void grabMouseEvent(QEvent*  event);
virtual void hideEvent(QHideEvent*  event);
virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent*  event);
virtual void hoverMoveEvent(QGraphicsSceneHoverEvent*  event);
virtual void initStyleOption(QStyleOption*  option) const;
virtual void moveEvent(QGraphicsSceneMoveEvent*  event);
virtual void paintWindowFrame(QPainter*  painter, const QStyleOptionGraphicsItem*  option, QWidget*  widget);
virtual void polishEvent();
virtual QVariant  propertyChange(const QString&  propertyName, const QVariant&  value);
virtual void resizeEvent(QGraphicsSceneResizeEvent*  event);
virtual void setGeometry(const QRectF&  rect);
virtual void showEvent(QShowEvent*  event);
virtual QSizeF  sizeHint(Qt::SizeHint  which, const QSizeF&  constraint = QSizeF()) const;
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void ungrabKeyboardEvent(QEvent*  event);
virtual void ungrabMouseEvent(QEvent*  event);
virtual void updateGeometry();
virtual bool  windowFrameEvent(QEvent*  e);
virtual Qt::WindowFrameSection  windowFrameSectionAt(const QPointF&  pos) const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QGraphicsProxyWidget : public QGraphicsProxyWidget
{ public:
inline bool  promoted_event(QEvent*  event) { return QGraphicsProxyWidget::event(event); }
inline bool  promoted_eventFilter(QObject*  object, QEvent*  event) { return QGraphicsProxyWidget::eventFilter(object, event); }
inline bool  promoted_focusNextPrevChild(bool  next) { return QGraphicsProxyWidget::focusNextPrevChild(next); }
inline void promoted_grabMouseEvent(QEvent*  event) { QGraphicsProxyWidget::grabMouseEvent(event); }
inline void promoted_hideEvent(QHideEvent*  event) { QGraphicsProxyWidget::hideEvent(event); }
inline void promoted_hoverLeaveEvent(QGraphicsSceneHoverEvent*  event) { QGraphicsProxyWidget::hoverLeaveEvent(event); }
inline void promoted_hoverMoveEvent(QGraphicsSceneHoverEvent*  event) { QGraphicsProxyWidget::hoverMoveEvent(event); }
inline void promoted_resizeEvent(QGraphicsSceneResizeEvent*  event) { QGraphicsProxyWidget::resizeEvent(event); }
inline void promoted_setGeometry(const QRectF&  rect) { QGraphicsProxyWidget::setGeometry(rect); }
inline void promoted_showEvent(QShowEvent*  event) { QGraphicsProxyWidget::showEvent(event); }
inline QSizeF  promoted_sizeHint(Qt::SizeHint  which, const QSizeF&  constraint = QSizeF()) const { return QGraphicsProxyWidget::sizeHint(which, constraint); }
inline void promoted_ungrabMouseEvent(QEvent*  event) { QGraphicsProxyWidget::ungrabMouseEvent(event); }
};

class PythonQtWrapper_QGraphicsProxyWidget : public QObject
{ Q_OBJECT
public:
Q_ENUMS(enum_1 )
enum enum_1{
  Type = QGraphicsProxyWidget::Type};
public slots:
QGraphicsProxyWidget* new_QGraphicsProxyWidget(QGraphicsItem*  parent = 0, Qt::WindowFlags  wFlags = 0);
void delete_QGraphicsProxyWidget(QGraphicsProxyWidget* obj) { delete obj; } 
   QGraphicsProxyWidget*  createProxyForChildWidget(QGraphicsProxyWidget* theWrappedObject, QWidget*  child);
   bool  event(QGraphicsProxyWidget* theWrappedObject, QEvent*  event);
   bool  eventFilter(QGraphicsProxyWidget* theWrappedObject, QObject*  object, QEvent*  event);
   bool  focusNextPrevChild(QGraphicsProxyWidget* theWrappedObject, bool  next);
   void grabMouseEvent(QGraphicsProxyWidget* theWrappedObject, QEvent*  event);
   void hideEvent(QGraphicsProxyWidget* theWrappedObject, QHideEvent*  event);
   void hoverLeaveEvent(QGraphicsProxyWidget* theWrappedObject, QGraphicsSceneHoverEvent*  event);
   void hoverMoveEvent(QGraphicsProxyWidget* theWrappedObject, QGraphicsSceneHoverEvent*  event);
   void paint(QGraphicsProxyWidget* theWrappedObject, QPainter*  painter, const QStyleOptionGraphicsItem*  option, QWidget*  widget);
   void resizeEvent(QGraphicsProxyWidget* theWrappedObject, QGraphicsSceneResizeEvent*  event);
   void setGeometry(QGraphicsProxyWidget* theWrappedObject, const QRectF&  rect);
   void setWidget(QGraphicsProxyWidget* theWrappedObject, QWidget*  widget);
   void showEvent(QGraphicsProxyWidget* theWrappedObject, QShowEvent*  event);
   QSizeF  sizeHint(QGraphicsProxyWidget* theWrappedObject, Qt::SizeHint  which, const QSizeF&  constraint = QSizeF()) const;
   QRectF  subWidgetRect(QGraphicsProxyWidget* theWrappedObject, const QWidget*  widget) const;
   int  type(QGraphicsProxyWidget* theWrappedObject) const;
   void ungrabMouseEvent(QGraphicsProxyWidget* theWrappedObject, QEvent*  event);
   QWidget*  widget(QGraphicsProxyWidget* theWrappedObject) const;
};





class PythonQtShell_QGraphicsRectItem : public QGraphicsRectItem
{
public:
    PythonQtShell_QGraphicsRectItem(QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0):QGraphicsRectItem(parent, scene),_wrapper(NULL) {};
    PythonQtShell_QGraphicsRectItem(const QRectF&  rect, QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0):QGraphicsRectItem(rect, parent, scene),_wrapper(NULL) {};
    PythonQtShell_QGraphicsRectItem(qreal  x, qreal  y, qreal  w, qreal  h, QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0):QGraphicsRectItem(x, y, w, h, parent, scene),_wrapper(NULL) {};

virtual bool  isObscuredBy(const QGraphicsItem*  item) const;
virtual QPainterPath  opaqueArea() const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QGraphicsRectItem : public QGraphicsRectItem
{ public:
inline bool  promoted_isObscuredBy(const QGraphicsItem*  item) const { return QGraphicsRectItem::isObscuredBy(item); }
inline QPainterPath  promoted_opaqueArea() const { return QGraphicsRectItem::opaqueArea(); }
};

class PythonQtWrapper_QGraphicsRectItem : public QObject
{ Q_OBJECT
public:
Q_ENUMS(enum_1 )
enum enum_1{
  Type = QGraphicsRectItem::Type};
public slots:
QGraphicsRectItem* new_QGraphicsRectItem(QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0);
QGraphicsRectItem* new_QGraphicsRectItem(const QRectF&  rect, QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0);
QGraphicsRectItem* new_QGraphicsRectItem(qreal  x, qreal  y, qreal  w, qreal  h, QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0);
void delete_QGraphicsRectItem(QGraphicsRectItem* obj) { delete obj; } 
   QRectF  boundingRect(QGraphicsRectItem* theWrappedObject) const;
   bool  contains(QGraphicsRectItem* theWrappedObject, const QPointF&  point) const;
   bool  isObscuredBy(QGraphicsRectItem* theWrappedObject, const QGraphicsItem*  item) const;
   QPainterPath  opaqueArea(QGraphicsRectItem* theWrappedObject) const;
   void paint(QGraphicsRectItem* theWrappedObject, QPainter*  painter, const QStyleOptionGraphicsItem*  option, QWidget*  widget = 0);
   QRectF  rect(QGraphicsRectItem* theWrappedObject) const;
   void setRect(QGraphicsRectItem* theWrappedObject, const QRectF&  rect);
   void setRect(QGraphicsRectItem* theWrappedObject, qreal  x, qreal  y, qreal  w, qreal  h);
   QPainterPath  shape(QGraphicsRectItem* theWrappedObject) const;
   int  type(QGraphicsRectItem* theWrappedObject) const;
};





class PythonQtShell_QGraphicsRotation : public QGraphicsRotation
{
public:
    PythonQtShell_QGraphicsRotation(QObject*  parent = 0):QGraphicsRotation(parent),_wrapper(NULL) {};

virtual void applyTo(QMatrix4x4*  matrix) const;
virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QGraphicsRotation : public QGraphicsRotation
{ public:
inline void promoted_applyTo(QMatrix4x4*  matrix) const { QGraphicsRotation::applyTo(matrix); }
};

class PythonQtWrapper_QGraphicsRotation : public QObject
{ Q_OBJECT
public:
public slots:
QGraphicsRotation* new_QGraphicsRotation(QObject*  parent = 0);
void delete_QGraphicsRotation(QGraphicsRotation* obj) { delete obj; } 
   qreal  angle(QGraphicsRotation* theWrappedObject) const;
   void applyTo(QGraphicsRotation* theWrappedObject, QMatrix4x4*  matrix) const;
   QVector3D  axis(QGraphicsRotation* theWrappedObject) const;
   QVector3D  origin(QGraphicsRotation* theWrappedObject) const;
   void setAngle(QGraphicsRotation* theWrappedObject, qreal  arg__1);
   void setAxis(QGraphicsRotation* theWrappedObject, Qt::Axis  axis);
   void setAxis(QGraphicsRotation* theWrappedObject, const QVector3D&  axis);
   void setOrigin(QGraphicsRotation* theWrappedObject, const QVector3D&  point);
};





class PythonQtShell_QGraphicsScale : public QGraphicsScale
{
public:
    PythonQtShell_QGraphicsScale(QObject*  parent = 0):QGraphicsScale(parent),_wrapper(NULL) {};

virtual void applyTo(QMatrix4x4*  matrix) const;
virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QGraphicsScale : public QGraphicsScale
{ public:
inline void promoted_applyTo(QMatrix4x4*  matrix) const { QGraphicsScale::applyTo(matrix); }
};

class PythonQtWrapper_QGraphicsScale : public QObject
{ Q_OBJECT
public:
public slots:
QGraphicsScale* new_QGraphicsScale(QObject*  parent = 0);
void delete_QGraphicsScale(QGraphicsScale* obj) { delete obj; } 
   void applyTo(QGraphicsScale* theWrappedObject, QMatrix4x4*  matrix) const;
   QVector3D  origin(QGraphicsScale* theWrappedObject) const;
   void setOrigin(QGraphicsScale* theWrappedObject, const QVector3D&  point);
   void setXScale(QGraphicsScale* theWrappedObject, qreal  arg__1);
   void setYScale(QGraphicsScale* theWrappedObject, qreal  arg__1);
   void setZScale(QGraphicsScale* theWrappedObject, qreal  arg__1);
   qreal  xScale(QGraphicsScale* theWrappedObject) const;
   qreal  yScale(QGraphicsScale* theWrappedObject) const;
   qreal  zScale(QGraphicsScale* theWrappedObject) const;
};





class PythonQtShell_QGraphicsScene : public QGraphicsScene
{
public:
    PythonQtShell_QGraphicsScene(QObject*  parent = 0):QGraphicsScene(parent),_wrapper(NULL) {};
    PythonQtShell_QGraphicsScene(const QRectF&  sceneRect, QObject*  parent = 0):QGraphicsScene(sceneRect, parent),_wrapper(NULL) {};
    PythonQtShell_QGraphicsScene(qreal  x, qreal  y, qreal  width, qreal  height, QObject*  parent = 0):QGraphicsScene(x, y, width, height, parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent*  event);
virtual void customEvent(QEvent*  arg__1);
virtual void dragEnterEvent(QGraphicsSceneDragDropEvent*  event);
virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent*  event);
virtual void dragMoveEvent(QGraphicsSceneDragDropEvent*  event);
virtual void drawBackground(QPainter*  painter, const QRectF&  rect);
virtual void drawForeground(QPainter*  painter, const QRectF&  rect);
virtual void drawItems(QPainter*  painter, int  numItems, QGraphicsItem**  items, const QStyleOptionGraphicsItem*  options, QWidget*  widget = 0);
virtual void dropEvent(QGraphicsSceneDragDropEvent*  event);
virtual bool  event(QEvent*  event);
virtual bool  eventFilter(QObject*  watched, QEvent*  event);
virtual void focusInEvent(QFocusEvent*  event);
virtual void focusOutEvent(QFocusEvent*  event);
virtual void helpEvent(QGraphicsSceneHelpEvent*  event);
virtual void inputMethodEvent(QInputMethodEvent*  event);
virtual QVariant  inputMethodQuery(Qt::InputMethodQuery  query) const;
virtual void keyPressEvent(QKeyEvent*  event);
virtual void keyReleaseEvent(QKeyEvent*  event);
virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*  event);
virtual void mouseMoveEvent(QGraphicsSceneMouseEvent*  event);
virtual void mousePressEvent(QGraphicsSceneMouseEvent*  event);
virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*  event);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void wheelEvent(QGraphicsSceneWheelEvent*  event);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QGraphicsScene : public QGraphicsScene
{ public:
inline void promoted_contextMenuEvent(QGraphicsSceneContextMenuEvent*  event) { QGraphicsScene::contextMenuEvent(event); }
inline void promoted_dragEnterEvent(QGraphicsSceneDragDropEvent*  event) { QGraphicsScene::dragEnterEvent(event); }
inline void promoted_dragLeaveEvent(QGraphicsSceneDragDropEvent*  event) { QGraphicsScene::dragLeaveEvent(event); }
inline void promoted_dragMoveEvent(QGraphicsSceneDragDropEvent*  event) { QGraphicsScene::dragMoveEvent(event); }
inline void promoted_drawBackground(QPainter*  painter, const QRectF&  rect) { QGraphicsScene::drawBackground(painter, rect); }
inline void promoted_drawForeground(QPainter*  painter, const QRectF&  rect) { QGraphicsScene::drawForeground(painter, rect); }
inline void promoted_drawItems(QPainter*  painter, int  numItems, QGraphicsItem**  items, const QStyleOptionGraphicsItem*  options, QWidget*  widget = 0) { QGraphicsScene::drawItems(painter, numItems, items, options, widget); }
inline void promoted_dropEvent(QGraphicsSceneDragDropEvent*  event) { QGraphicsScene::dropEvent(event); }
inline bool  promoted_event(QEvent*  event) { return QGraphicsScene::event(event); }
inline bool  promoted_eventFilter(QObject*  watched, QEvent*  event) { return QGraphicsScene::eventFilter(watched, event); }
inline void promoted_focusInEvent(QFocusEvent*  event) { QGraphicsScene::focusInEvent(event); }
inline void promoted_focusOutEvent(QFocusEvent*  event) { QGraphicsScene::focusOutEvent(event); }
inline void promoted_helpEvent(QGraphicsSceneHelpEvent*  event) { QGraphicsScene::helpEvent(event); }
inline void promoted_inputMethodEvent(QInputMethodEvent*  event) { QGraphicsScene::inputMethodEvent(event); }
inline QVariant  promoted_inputMethodQuery(Qt::InputMethodQuery  query) const { return QGraphicsScene::inputMethodQuery(query); }
inline void promoted_keyPressEvent(QKeyEvent*  event) { QGraphicsScene::keyPressEvent(event); }
inline void promoted_keyReleaseEvent(QKeyEvent*  event) { QGraphicsScene::keyReleaseEvent(event); }
inline void promoted_mouseDoubleClickEvent(QGraphicsSceneMouseEvent*  event) { QGraphicsScene::mouseDoubleClickEvent(event); }
inline void promoted_mouseMoveEvent(QGraphicsSceneMouseEvent*  event) { QGraphicsScene::mouseMoveEvent(event); }
inline void promoted_mousePressEvent(QGraphicsSceneMouseEvent*  event) { QGraphicsScene::mousePressEvent(event); }
inline void promoted_mouseReleaseEvent(QGraphicsSceneMouseEvent*  event) { QGraphicsScene::mouseReleaseEvent(event); }
inline void promoted_wheelEvent(QGraphicsSceneWheelEvent*  event) { QGraphicsScene::wheelEvent(event); }
};

class PythonQtWrapper_QGraphicsScene : public QObject
{ Q_OBJECT
public:
Q_ENUMS(SceneLayer ItemIndexMethod )
Q_FLAGS(SceneLayers )
enum SceneLayer{
  ItemLayer = QGraphicsScene::ItemLayer,   BackgroundLayer = QGraphicsScene::BackgroundLayer,   ForegroundLayer = QGraphicsScene::ForegroundLayer,   AllLayers = QGraphicsScene::AllLayers};
enum ItemIndexMethod{
  BspTreeIndex = QGraphicsScene::BspTreeIndex,   NoIndex = QGraphicsScene::NoIndex};
Q_DECLARE_FLAGS(SceneLayers, SceneLayer)
public slots:
QGraphicsScene* new_QGraphicsScene(QObject*  parent = 0);
QGraphicsScene* new_QGraphicsScene(const QRectF&  sceneRect, QObject*  parent = 0);
QGraphicsScene* new_QGraphicsScene(qreal  x, qreal  y, qreal  width, qreal  height, QObject*  parent = 0);
void delete_QGraphicsScene(QGraphicsScene* obj) { delete obj; } 
   QGraphicsItem*  activePanel(QGraphicsScene* theWrappedObject) const;
   QGraphicsWidget*  activeWindow(QGraphicsScene* theWrappedObject) const;
   QGraphicsEllipseItem*  addEllipse(QGraphicsScene* theWrappedObject, const QRectF&  rect, const QPen&  pen = QPen(), const QBrush&  brush = QBrush());
   QGraphicsEllipseItem*  addEllipse(QGraphicsScene* theWrappedObject, qreal  x, qreal  y, qreal  w, qreal  h, const QPen&  pen = QPen(), const QBrush&  brush = QBrush());
   void addItem(QGraphicsScene* theWrappedObject, QGraphicsItem*  item);
   QGraphicsLineItem*  addLine(QGraphicsScene* theWrappedObject, const QLineF&  line, const QPen&  pen = QPen());
   QGraphicsLineItem*  addLine(QGraphicsScene* theWrappedObject, qreal  x1, qreal  y1, qreal  x2, qreal  y2, const QPen&  pen = QPen());
   QGraphicsPathItem*  addPath(QGraphicsScene* theWrappedObject, const QPainterPath&  path, const QPen&  pen = QPen(), const QBrush&  brush = QBrush());
   QGraphicsPixmapItem*  addPixmap(QGraphicsScene* theWrappedObject, const QPixmap&  pixmap);
   QGraphicsPolygonItem*  addPolygon(QGraphicsScene* theWrappedObject, const QPolygonF&  polygon, const QPen&  pen = QPen(), const QBrush&  brush = QBrush());
   QGraphicsRectItem*  addRect(QGraphicsScene* theWrappedObject, const QRectF&  rect, const QPen&  pen = QPen(), const QBrush&  brush = QBrush());
   QGraphicsRectItem*  addRect(QGraphicsScene* theWrappedObject, qreal  x, qreal  y, qreal  w, qreal  h, const QPen&  pen = QPen(), const QBrush&  brush = QBrush());
   QGraphicsSimpleTextItem*  addSimpleText(QGraphicsScene* theWrappedObject, const QString&  text, const QFont&  font = QFont());
   QGraphicsTextItem*  addText(QGraphicsScene* theWrappedObject, const QString&  text, const QFont&  font = QFont());
   QGraphicsProxyWidget*  addWidget(QGraphicsScene* theWrappedObject, QWidget*  widget, Qt::WindowFlags  wFlags = 0);
   QBrush  backgroundBrush(QGraphicsScene* theWrappedObject) const;
   int  bspTreeDepth(QGraphicsScene* theWrappedObject) const;
   void clearFocus(QGraphicsScene* theWrappedObject);
   QList<QGraphicsItem* >  collidingItems(QGraphicsScene* theWrappedObject, const QGraphicsItem*  item, Qt::ItemSelectionMode  mode = Qt::IntersectsItemShape) const;
   void contextMenuEvent(QGraphicsScene* theWrappedObject, QGraphicsSceneContextMenuEvent*  event);
   QGraphicsItemGroup*  createItemGroup(QGraphicsScene* theWrappedObject, const QList<QGraphicsItem* >&  items);
   void destroyItemGroup(QGraphicsScene* theWrappedObject, QGraphicsItemGroup*  group);
   void dragEnterEvent(QGraphicsScene* theWrappedObject, QGraphicsSceneDragDropEvent*  event);
   void dragLeaveEvent(QGraphicsScene* theWrappedObject, QGraphicsSceneDragDropEvent*  event);
   void dragMoveEvent(QGraphicsScene* theWrappedObject, QGraphicsSceneDragDropEvent*  event);
   void drawBackground(QGraphicsScene* theWrappedObject, QPainter*  painter, const QRectF&  rect);
   void drawForeground(QGraphicsScene* theWrappedObject, QPainter*  painter, const QRectF&  rect);
   void drawItems(QGraphicsScene* theWrappedObject, QPainter*  painter, int  numItems, QGraphicsItem**  items, const QStyleOptionGraphicsItem*  options, QWidget*  widget = 0);
   void dropEvent(QGraphicsScene* theWrappedObject, QGraphicsSceneDragDropEvent*  event);
   bool  event(QGraphicsScene* theWrappedObject, QEvent*  event);
   bool  eventFilter(QGraphicsScene* theWrappedObject, QObject*  watched, QEvent*  event);
   void focusInEvent(QGraphicsScene* theWrappedObject, QFocusEvent*  event);
   QGraphicsItem*  focusItem(QGraphicsScene* theWrappedObject) const;
   void focusOutEvent(QGraphicsScene* theWrappedObject, QFocusEvent*  event);
   QFont  font(QGraphicsScene* theWrappedObject) const;
   QBrush  foregroundBrush(QGraphicsScene* theWrappedObject) const;
   bool  hasFocus(QGraphicsScene* theWrappedObject) const;
   qreal  height(QGraphicsScene* theWrappedObject) const;
   void helpEvent(QGraphicsScene* theWrappedObject, QGraphicsSceneHelpEvent*  event);
   void inputMethodEvent(QGraphicsScene* theWrappedObject, QInputMethodEvent*  event);
   QVariant  inputMethodQuery(QGraphicsScene* theWrappedObject, Qt::InputMethodQuery  query) const;
   void invalidate(QGraphicsScene* theWrappedObject, qreal  x, qreal  y, qreal  w, qreal  h, QGraphicsScene::SceneLayers  layers = QGraphicsScene::AllLayers);
   bool  isActive(QGraphicsScene* theWrappedObject) const;
   bool  isSortCacheEnabled(QGraphicsScene* theWrappedObject) const;
   QGraphicsItem*  itemAt(QGraphicsScene* theWrappedObject, const QPointF&  pos) const;
   QGraphicsItem*  itemAt(QGraphicsScene* theWrappedObject, const QPointF&  pos, const QTransform&  deviceTransform) const;
   QGraphicsItem*  itemAt(QGraphicsScene* theWrappedObject, qreal  x, qreal  y) const;
   QGraphicsItem*  itemAt(QGraphicsScene* theWrappedObject, qreal  x, qreal  y, const QTransform&  deviceTransform) const;
   QGraphicsScene::ItemIndexMethod  itemIndexMethod(QGraphicsScene* theWrappedObject) const;
   QList<QGraphicsItem* >  items(QGraphicsScene* theWrappedObject) const;
   QList<QGraphicsItem* >  items(QGraphicsScene* theWrappedObject, Qt::SortOrder  order) const;
   QList<QGraphicsItem* >  items(QGraphicsScene* theWrappedObject, const QPainterPath&  path, Qt::ItemSelectionMode  mode = Qt::IntersectsItemShape) const;
   QList<QGraphicsItem* >  items(QGraphicsScene* theWrappedObject, const QPainterPath&  path, Qt::ItemSelectionMode  mode, Qt::SortOrder  order, const QTransform&  deviceTransform = QTransform()) const;
   QList<QGraphicsItem* >  items(QGraphicsScene* theWrappedObject, const QPointF&  pos) const;
   QList<QGraphicsItem* >  items(QGraphicsScene* theWrappedObject, const QPointF&  pos, Qt::ItemSelectionMode  mode, Qt::SortOrder  order, const QTransform&  deviceTransform = QTransform()) const;
   QList<QGraphicsItem* >  items(QGraphicsScene* theWrappedObject, const QPolygonF&  polygon, Qt::ItemSelectionMode  mode = Qt::IntersectsItemShape) const;
   QList<QGraphicsItem* >  items(QGraphicsScene* theWrappedObject, const QPolygonF&  polygon, Qt::ItemSelectionMode  mode, Qt::SortOrder  order, const QTransform&  deviceTransform = QTransform()) const;
   QList<QGraphicsItem* >  items(QGraphicsScene* theWrappedObject, const QRectF&  rect, Qt::ItemSelectionMode  mode = Qt::IntersectsItemShape) const;
   QList<QGraphicsItem* >  items(QGraphicsScene* theWrappedObject, const QRectF&  rect, Qt::ItemSelectionMode  mode, Qt::SortOrder  order, const QTransform&  deviceTransform = QTransform()) const;
   QList<QGraphicsItem* >  items(QGraphicsScene* theWrappedObject, qreal  x, qreal  y, qreal  w, qreal  h, Qt::ItemSelectionMode  mode = Qt::IntersectsItemShape) const;
   QList<QGraphicsItem* >  items(QGraphicsScene* theWrappedObject, qreal  x, qreal  y, qreal  w, qreal  h, Qt::ItemSelectionMode  mode, Qt::SortOrder  order, const QTransform&  deviceTransform = QTransform()) const;
   QRectF  itemsBoundingRect(QGraphicsScene* theWrappedObject) const;
   void keyPressEvent(QGraphicsScene* theWrappedObject, QKeyEvent*  event);
   void keyReleaseEvent(QGraphicsScene* theWrappedObject, QKeyEvent*  event);
   void mouseDoubleClickEvent(QGraphicsScene* theWrappedObject, QGraphicsSceneMouseEvent*  event);
   QGraphicsItem*  mouseGrabberItem(QGraphicsScene* theWrappedObject) const;
   void mouseMoveEvent(QGraphicsScene* theWrappedObject, QGraphicsSceneMouseEvent*  event);
   void mousePressEvent(QGraphicsScene* theWrappedObject, QGraphicsSceneMouseEvent*  event);
   void mouseReleaseEvent(QGraphicsScene* theWrappedObject, QGraphicsSceneMouseEvent*  event);
   QPalette  palette(QGraphicsScene* theWrappedObject) const;
   void removeItem(QGraphicsScene* theWrappedObject, QGraphicsItem*  item);
   void render(QGraphicsScene* theWrappedObject, QPainter*  painter, const QRectF&  target = QRectF(), const QRectF&  source = QRectF(), Qt::AspectRatioMode  aspectRatioMode = Qt::KeepAspectRatio);
   QRectF  sceneRect(QGraphicsScene* theWrappedObject) const;
   QList<QGraphicsItem* >  selectedItems(QGraphicsScene* theWrappedObject) const;
   QPainterPath  selectionArea(QGraphicsScene* theWrappedObject) const;
   bool  sendEvent(QGraphicsScene* theWrappedObject, QGraphicsItem*  item, QEvent*  event);
   void setActivePanel(QGraphicsScene* theWrappedObject, QGraphicsItem*  item);
   void setActiveWindow(QGraphicsScene* theWrappedObject, QGraphicsWidget*  widget);
   void setBackgroundBrush(QGraphicsScene* theWrappedObject, const QBrush&  brush);
   void setBspTreeDepth(QGraphicsScene* theWrappedObject, int  depth);
   void setFocus(QGraphicsScene* theWrappedObject, Qt::FocusReason  focusReason = Qt::OtherFocusReason);
   void setFocusItem(QGraphicsScene* theWrappedObject, QGraphicsItem*  item, Qt::FocusReason  focusReason = Qt::OtherFocusReason);
   void setFont(QGraphicsScene* theWrappedObject, const QFont&  font);
   void setForegroundBrush(QGraphicsScene* theWrappedObject, const QBrush&  brush);
   void setItemIndexMethod(QGraphicsScene* theWrappedObject, QGraphicsScene::ItemIndexMethod  method);
   void setPalette(QGraphicsScene* theWrappedObject, const QPalette&  palette);
   void setSceneRect(QGraphicsScene* theWrappedObject, const QRectF&  rect);
   void setSceneRect(QGraphicsScene* theWrappedObject, qreal  x, qreal  y, qreal  w, qreal  h);
   void setSelectionArea(QGraphicsScene* theWrappedObject, const QPainterPath&  path);
   void setSelectionArea(QGraphicsScene* theWrappedObject, const QPainterPath&  path, Qt::ItemSelectionMode  mode);
   void setSelectionArea(QGraphicsScene* theWrappedObject, const QPainterPath&  path, Qt::ItemSelectionMode  mode, const QTransform&  deviceTransform);
   void setSelectionArea(QGraphicsScene* theWrappedObject, const QPainterPath&  path, const QTransform&  deviceTransform);
   void setSortCacheEnabled(QGraphicsScene* theWrappedObject, bool  enabled);
   void setStickyFocus(QGraphicsScene* theWrappedObject, bool  enabled);
   void setStyle(QGraphicsScene* theWrappedObject, QStyle*  style);
   bool  stickyFocus(QGraphicsScene* theWrappedObject) const;
   QStyle*  style(QGraphicsScene* theWrappedObject) const;
   void update(QGraphicsScene* theWrappedObject, qreal  x, qreal  y, qreal  w, qreal  h);
   QList<QGraphicsView* >  views(QGraphicsScene* theWrappedObject) const;
   void wheelEvent(QGraphicsScene* theWrappedObject, QGraphicsSceneWheelEvent*  event);
   qreal  width(QGraphicsScene* theWrappedObject) const;
};





class PythonQtWrapper_QGraphicsSceneContextMenuEvent : public QObject
{ Q_OBJECT
public:
Q_ENUMS(Reason )
enum Reason{
  Mouse = QGraphicsSceneContextMenuEvent::Mouse,   Keyboard = QGraphicsSceneContextMenuEvent::Keyboard,   Other = QGraphicsSceneContextMenuEvent::Other};
public slots:
QGraphicsSceneContextMenuEvent* new_QGraphicsSceneContextMenuEvent(QEvent::Type  type = QEvent::None);
void delete_QGraphicsSceneContextMenuEvent(QGraphicsSceneContextMenuEvent* obj) { delete obj; } 
   Qt::KeyboardModifiers  modifiers(QGraphicsSceneContextMenuEvent* theWrappedObject) const;
   QPointF  pos(QGraphicsSceneContextMenuEvent* theWrappedObject) const;
   QGraphicsSceneContextMenuEvent::Reason  reason(QGraphicsSceneContextMenuEvent* theWrappedObject) const;
   QPointF  scenePos(QGraphicsSceneContextMenuEvent* theWrappedObject) const;
   QPoint  screenPos(QGraphicsSceneContextMenuEvent* theWrappedObject) const;
   void setModifiers(QGraphicsSceneContextMenuEvent* theWrappedObject, Qt::KeyboardModifiers  modifiers);
   void setPos(QGraphicsSceneContextMenuEvent* theWrappedObject, const QPointF&  pos);
   void setReason(QGraphicsSceneContextMenuEvent* theWrappedObject, QGraphicsSceneContextMenuEvent::Reason  reason);
   void setScenePos(QGraphicsSceneContextMenuEvent* theWrappedObject, const QPointF&  pos);
   void setScreenPos(QGraphicsSceneContextMenuEvent* theWrappedObject, const QPoint&  pos);
};





class PythonQtWrapper_QGraphicsSceneDragDropEvent : public QObject
{ Q_OBJECT
public:
public slots:
QGraphicsSceneDragDropEvent* new_QGraphicsSceneDragDropEvent(QEvent::Type  type = QEvent::None);
void delete_QGraphicsSceneDragDropEvent(QGraphicsSceneDragDropEvent* obj) { delete obj; } 
   void acceptProposedAction(QGraphicsSceneDragDropEvent* theWrappedObject);
   Qt::MouseButtons  buttons(QGraphicsSceneDragDropEvent* theWrappedObject) const;
   Qt::DropAction  dropAction(QGraphicsSceneDragDropEvent* theWrappedObject) const;
   const QMimeData*  mimeData(QGraphicsSceneDragDropEvent* theWrappedObject) const;
   Qt::KeyboardModifiers  modifiers(QGraphicsSceneDragDropEvent* theWrappedObject) const;
   QPointF  pos(QGraphicsSceneDragDropEvent* theWrappedObject) const;
   Qt::DropActions  possibleActions(QGraphicsSceneDragDropEvent* theWrappedObject) const;
   Qt::DropAction  proposedAction(QGraphicsSceneDragDropEvent* theWrappedObject) const;
   QPointF  scenePos(QGraphicsSceneDragDropEvent* theWrappedObject) const;
   QPoint  screenPos(QGraphicsSceneDragDropEvent* theWrappedObject) const;
   void setButtons(QGraphicsSceneDragDropEvent* theWrappedObject, Qt::MouseButtons  buttons);
   void setDropAction(QGraphicsSceneDragDropEvent* theWrappedObject, Qt::DropAction  action);
   void setModifiers(QGraphicsSceneDragDropEvent* theWrappedObject, Qt::KeyboardModifiers  modifiers);
   void setPos(QGraphicsSceneDragDropEvent* theWrappedObject, const QPointF&  pos);
   void setPossibleActions(QGraphicsSceneDragDropEvent* theWrappedObject, Qt::DropActions  actions);
   void setProposedAction(QGraphicsSceneDragDropEvent* theWrappedObject, Qt::DropAction  action);
   void setScenePos(QGraphicsSceneDragDropEvent* theWrappedObject, const QPointF&  pos);
   void setScreenPos(QGraphicsSceneDragDropEvent* theWrappedObject, const QPoint&  pos);
   QWidget*  source(QGraphicsSceneDragDropEvent* theWrappedObject) const;
};





class PythonQtWrapper_QGraphicsSceneEvent : public QObject
{ Q_OBJECT
public:
public slots:
QGraphicsSceneEvent* new_QGraphicsSceneEvent(QEvent::Type  type);
void delete_QGraphicsSceneEvent(QGraphicsSceneEvent* obj) { delete obj; } 
   QWidget*  widget(QGraphicsSceneEvent* theWrappedObject) const;
};





class PythonQtWrapper_QGraphicsSceneHelpEvent : public QObject
{ Q_OBJECT
public:
public slots:
QGraphicsSceneHelpEvent* new_QGraphicsSceneHelpEvent(QEvent::Type  type = QEvent::None);
void delete_QGraphicsSceneHelpEvent(QGraphicsSceneHelpEvent* obj) { delete obj; } 
   QPointF  scenePos(QGraphicsSceneHelpEvent* theWrappedObject) const;
   QPoint  screenPos(QGraphicsSceneHelpEvent* theWrappedObject) const;
   void setScenePos(QGraphicsSceneHelpEvent* theWrappedObject, const QPointF&  pos);
   void setScreenPos(QGraphicsSceneHelpEvent* theWrappedObject, const QPoint&  pos);
};





class PythonQtWrapper_QGraphicsSceneHoverEvent : public QObject
{ Q_OBJECT
public:
public slots:
QGraphicsSceneHoverEvent* new_QGraphicsSceneHoverEvent(QEvent::Type  type = QEvent::None);
void delete_QGraphicsSceneHoverEvent(QGraphicsSceneHoverEvent* obj) { delete obj; } 
   QPointF  lastPos(QGraphicsSceneHoverEvent* theWrappedObject) const;
   QPointF  lastScenePos(QGraphicsSceneHoverEvent* theWrappedObject) const;
   QPoint  lastScreenPos(QGraphicsSceneHoverEvent* theWrappedObject) const;
   Qt::KeyboardModifiers  modifiers(QGraphicsSceneHoverEvent* theWrappedObject) const;
   QPointF  pos(QGraphicsSceneHoverEvent* theWrappedObject) const;
   QPointF  scenePos(QGraphicsSceneHoverEvent* theWrappedObject) const;
   QPoint  screenPos(QGraphicsSceneHoverEvent* theWrappedObject) const;
   void setLastPos(QGraphicsSceneHoverEvent* theWrappedObject, const QPointF&  pos);
   void setLastScenePos(QGraphicsSceneHoverEvent* theWrappedObject, const QPointF&  pos);
   void setLastScreenPos(QGraphicsSceneHoverEvent* theWrappedObject, const QPoint&  pos);
   void setModifiers(QGraphicsSceneHoverEvent* theWrappedObject, Qt::KeyboardModifiers  modifiers);
   void setPos(QGraphicsSceneHoverEvent* theWrappedObject, const QPointF&  pos);
   void setScenePos(QGraphicsSceneHoverEvent* theWrappedObject, const QPointF&  pos);
   void setScreenPos(QGraphicsSceneHoverEvent* theWrappedObject, const QPoint&  pos);
};





class PythonQtWrapper_QGraphicsSceneMouseEvent : public QObject
{ Q_OBJECT
public:
public slots:
QGraphicsSceneMouseEvent* new_QGraphicsSceneMouseEvent(QEvent::Type  type = QEvent::None);
void delete_QGraphicsSceneMouseEvent(QGraphicsSceneMouseEvent* obj) { delete obj; } 
   Qt::MouseButton  button(QGraphicsSceneMouseEvent* theWrappedObject) const;
   QPointF  buttonDownPos(QGraphicsSceneMouseEvent* theWrappedObject, Qt::MouseButton  button) const;
   QPointF  buttonDownScenePos(QGraphicsSceneMouseEvent* theWrappedObject, Qt::MouseButton  button) const;
   QPoint  buttonDownScreenPos(QGraphicsSceneMouseEvent* theWrappedObject, Qt::MouseButton  button) const;
   Qt::MouseButtons  buttons(QGraphicsSceneMouseEvent* theWrappedObject) const;
   QPointF  lastPos(QGraphicsSceneMouseEvent* theWrappedObject) const;
   QPointF  lastScenePos(QGraphicsSceneMouseEvent* theWrappedObject) const;
   QPoint  lastScreenPos(QGraphicsSceneMouseEvent* theWrappedObject) const;
   Qt::KeyboardModifiers  modifiers(QGraphicsSceneMouseEvent* theWrappedObject) const;
   QPointF  pos(QGraphicsSceneMouseEvent* theWrappedObject) const;
   QPointF  scenePos(QGraphicsSceneMouseEvent* theWrappedObject) const;
   QPoint  screenPos(QGraphicsSceneMouseEvent* theWrappedObject) const;
   void setButton(QGraphicsSceneMouseEvent* theWrappedObject, Qt::MouseButton  button);
   void setButtonDownPos(QGraphicsSceneMouseEvent* theWrappedObject, Qt::MouseButton  button, const QPointF&  pos);
   void setButtonDownScenePos(QGraphicsSceneMouseEvent* theWrappedObject, Qt::MouseButton  button, const QPointF&  pos);
   void setButtonDownScreenPos(QGraphicsSceneMouseEvent* theWrappedObject, Qt::MouseButton  button, const QPoint&  pos);
   void setButtons(QGraphicsSceneMouseEvent* theWrappedObject, Qt::MouseButtons  buttons);
   void setLastPos(QGraphicsSceneMouseEvent* theWrappedObject, const QPointF&  pos);
   void setLastScenePos(QGraphicsSceneMouseEvent* theWrappedObject, const QPointF&  pos);
   void setLastScreenPos(QGraphicsSceneMouseEvent* theWrappedObject, const QPoint&  pos);
   void setModifiers(QGraphicsSceneMouseEvent* theWrappedObject, Qt::KeyboardModifiers  modifiers);
   void setPos(QGraphicsSceneMouseEvent* theWrappedObject, const QPointF&  pos);
   void setScenePos(QGraphicsSceneMouseEvent* theWrappedObject, const QPointF&  pos);
   void setScreenPos(QGraphicsSceneMouseEvent* theWrappedObject, const QPoint&  pos);
};





class PythonQtWrapper_QGraphicsSceneMoveEvent : public QObject
{ Q_OBJECT
public:
public slots:
QGraphicsSceneMoveEvent* new_QGraphicsSceneMoveEvent();
void delete_QGraphicsSceneMoveEvent(QGraphicsSceneMoveEvent* obj) { delete obj; } 
   QPointF  newPos(QGraphicsSceneMoveEvent* theWrappedObject) const;
   QPointF  oldPos(QGraphicsSceneMoveEvent* theWrappedObject) const;
   void setNewPos(QGraphicsSceneMoveEvent* theWrappedObject, const QPointF&  pos);
   void setOldPos(QGraphicsSceneMoveEvent* theWrappedObject, const QPointF&  pos);
};





class PythonQtWrapper_QGraphicsSceneResizeEvent : public QObject
{ Q_OBJECT
public:
public slots:
QGraphicsSceneResizeEvent* new_QGraphicsSceneResizeEvent();
void delete_QGraphicsSceneResizeEvent(QGraphicsSceneResizeEvent* obj) { delete obj; } 
   QSizeF  newSize(QGraphicsSceneResizeEvent* theWrappedObject) const;
   QSizeF  oldSize(QGraphicsSceneResizeEvent* theWrappedObject) const;
   void setNewSize(QGraphicsSceneResizeEvent* theWrappedObject, const QSizeF&  size);
   void setOldSize(QGraphicsSceneResizeEvent* theWrappedObject, const QSizeF&  size);
};





class PythonQtWrapper_QGraphicsSceneWheelEvent : public QObject
{ Q_OBJECT
public:
public slots:
QGraphicsSceneWheelEvent* new_QGraphicsSceneWheelEvent(QEvent::Type  type = QEvent::None);
void delete_QGraphicsSceneWheelEvent(QGraphicsSceneWheelEvent* obj) { delete obj; } 
   Qt::MouseButtons  buttons(QGraphicsSceneWheelEvent* theWrappedObject) const;
   int  delta(QGraphicsSceneWheelEvent* theWrappedObject) const;
   Qt::KeyboardModifiers  modifiers(QGraphicsSceneWheelEvent* theWrappedObject) const;
   Qt::Orientation  orientation(QGraphicsSceneWheelEvent* theWrappedObject) const;
   QPointF  pos(QGraphicsSceneWheelEvent* theWrappedObject) const;
   QPointF  scenePos(QGraphicsSceneWheelEvent* theWrappedObject) const;
   QPoint  screenPos(QGraphicsSceneWheelEvent* theWrappedObject) const;
   void setButtons(QGraphicsSceneWheelEvent* theWrappedObject, Qt::MouseButtons  buttons);
   void setDelta(QGraphicsSceneWheelEvent* theWrappedObject, int  delta);
   void setModifiers(QGraphicsSceneWheelEvent* theWrappedObject, Qt::KeyboardModifiers  modifiers);
   void setOrientation(QGraphicsSceneWheelEvent* theWrappedObject, Qt::Orientation  orientation);
   void setPos(QGraphicsSceneWheelEvent* theWrappedObject, const QPointF&  pos);
   void setScenePos(QGraphicsSceneWheelEvent* theWrappedObject, const QPointF&  pos);
   void setScreenPos(QGraphicsSceneWheelEvent* theWrappedObject, const QPoint&  pos);
};





class PythonQtShell_QGraphicsSimpleTextItem : public QGraphicsSimpleTextItem
{
public:
    PythonQtShell_QGraphicsSimpleTextItem(QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0):QGraphicsSimpleTextItem(parent, scene),_wrapper(NULL) {};
    PythonQtShell_QGraphicsSimpleTextItem(const QString&  text, QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0):QGraphicsSimpleTextItem(text, parent, scene),_wrapper(NULL) {};

virtual bool  isObscuredBy(const QGraphicsItem*  item) const;
virtual QPainterPath  opaqueArea() const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QGraphicsSimpleTextItem : public QGraphicsSimpleTextItem
{ public:
inline bool  promoted_isObscuredBy(const QGraphicsItem*  item) const { return QGraphicsSimpleTextItem::isObscuredBy(item); }
inline QPainterPath  promoted_opaqueArea() const { return QGraphicsSimpleTextItem::opaqueArea(); }
};

class PythonQtWrapper_QGraphicsSimpleTextItem : public QObject
{ Q_OBJECT
public:
Q_ENUMS(enum_1 )
enum enum_1{
  Type = QGraphicsSimpleTextItem::Type};
public slots:
QGraphicsSimpleTextItem* new_QGraphicsSimpleTextItem(QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0);
QGraphicsSimpleTextItem* new_QGraphicsSimpleTextItem(const QString&  text, QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0);
void delete_QGraphicsSimpleTextItem(QGraphicsSimpleTextItem* obj) { delete obj; } 
   QRectF  boundingRect(QGraphicsSimpleTextItem* theWrappedObject) const;
   bool  contains(QGraphicsSimpleTextItem* theWrappedObject, const QPointF&  point) const;
   QFont  font(QGraphicsSimpleTextItem* theWrappedObject) const;
   bool  isObscuredBy(QGraphicsSimpleTextItem* theWrappedObject, const QGraphicsItem*  item) const;
   QPainterPath  opaqueArea(QGraphicsSimpleTextItem* theWrappedObject) const;
   void paint(QGraphicsSimpleTextItem* theWrappedObject, QPainter*  painter, const QStyleOptionGraphicsItem*  option, QWidget*  widget);
   void setFont(QGraphicsSimpleTextItem* theWrappedObject, const QFont&  font);
   void setText(QGraphicsSimpleTextItem* theWrappedObject, const QString&  text);
   QPainterPath  shape(QGraphicsSimpleTextItem* theWrappedObject) const;
   QString  text(QGraphicsSimpleTextItem* theWrappedObject) const;
   int  type(QGraphicsSimpleTextItem* theWrappedObject) const;
};





class PythonQtShell_QGraphicsTextItem : public QGraphicsTextItem
{
public:
    PythonQtShell_QGraphicsTextItem(QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0):QGraphicsTextItem(parent, scene),_wrapper(NULL) {};
    PythonQtShell_QGraphicsTextItem(const QString&  text, QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0):QGraphicsTextItem(text, parent, scene),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QGraphicsTextItem : public QObject
{ Q_OBJECT
public:
Q_ENUMS(enum_1 )
enum enum_1{
  Type = QGraphicsTextItem::Type};
public slots:
QGraphicsTextItem* new_QGraphicsTextItem(QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0);
QGraphicsTextItem* new_QGraphicsTextItem(const QString&  text, QGraphicsItem*  parent = 0, QGraphicsScene*  scene = 0);
void delete_QGraphicsTextItem(QGraphicsTextItem* obj) { delete obj; } 
   void adjustSize(QGraphicsTextItem* theWrappedObject);
   QRectF  boundingRect(QGraphicsTextItem* theWrappedObject) const;
   bool  contains(QGraphicsTextItem* theWrappedObject, const QPointF&  point) const;
   QColor  defaultTextColor(QGraphicsTextItem* theWrappedObject) const;
   QTextDocument*  document(QGraphicsTextItem* theWrappedObject) const;
   QFont  font(QGraphicsTextItem* theWrappedObject) const;
   bool  isObscuredBy(QGraphicsTextItem* theWrappedObject, const QGraphicsItem*  item) const;
   QPainterPath  opaqueArea(QGraphicsTextItem* theWrappedObject) const;
   bool  openExternalLinks(QGraphicsTextItem* theWrappedObject) const;
   void paint(QGraphicsTextItem* theWrappedObject, QPainter*  painter, const QStyleOptionGraphicsItem*  option, QWidget*  widget);
   void setDefaultTextColor(QGraphicsTextItem* theWrappedObject, const QColor&  c);
   void setDocument(QGraphicsTextItem* theWrappedObject, QTextDocument*  document);
   void setFont(QGraphicsTextItem* theWrappedObject, const QFont&  font);
   void setHtml(QGraphicsTextItem* theWrappedObject, const QString&  html);
   void setOpenExternalLinks(QGraphicsTextItem* theWrappedObject, bool  open);
   void setPlainText(QGraphicsTextItem* theWrappedObject, const QString&  text);
   void setTabChangesFocus(QGraphicsTextItem* theWrappedObject, bool  b);
   void setTextCursor(QGraphicsTextItem* theWrappedObject, const QTextCursor&  cursor);
   void setTextInteractionFlags(QGraphicsTextItem* theWrappedObject, Qt::TextInteractionFlags  flags);
   void setTextWidth(QGraphicsTextItem* theWrappedObject, qreal  width);
   QPainterPath  shape(QGraphicsTextItem* theWrappedObject) const;
   bool  tabChangesFocus(QGraphicsTextItem* theWrappedObject) const;
   QTextCursor  textCursor(QGraphicsTextItem* theWrappedObject) const;
   Qt::TextInteractionFlags  textInteractionFlags(QGraphicsTextItem* theWrappedObject) const;
   qreal  textWidth(QGraphicsTextItem* theWrappedObject) const;
   QString  toHtml(QGraphicsTextItem* theWrappedObject) const;
   QString  toPlainText(QGraphicsTextItem* theWrappedObject) const;
   int  type(QGraphicsTextItem* theWrappedObject) const;
};





class PythonQtShell_QGraphicsTransform : public QGraphicsTransform
{
public:
    PythonQtShell_QGraphicsTransform(QObject*  parent = 0):QGraphicsTransform(parent),_wrapper(NULL) {};

virtual void applyTo(QMatrix4x4*  matrix) const;
virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QGraphicsTransform : public QObject
{ Q_OBJECT
public:
public slots:
QGraphicsTransform* new_QGraphicsTransform(QObject*  parent = 0);
void delete_QGraphicsTransform(QGraphicsTransform* obj) { delete obj; } 
};





class PythonQtShell_QGraphicsView : public QGraphicsView
{
public:
    PythonQtShell_QGraphicsView(QGraphicsScene*  scene, QWidget*  parent = 0):QGraphicsView(scene, parent),_wrapper(NULL) {};
    PythonQtShell_QGraphicsView(QWidget*  parent = 0):QGraphicsView(parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  arg__1);
virtual void childEvent(QChildEvent*  arg__1);
virtual void closeEvent(QCloseEvent*  arg__1);
virtual void contextMenuEvent(QContextMenuEvent*  event);
virtual void customEvent(QEvent*  arg__1);
virtual int  devType() const;
virtual void dragEnterEvent(QDragEnterEvent*  event);
virtual void dragLeaveEvent(QDragLeaveEvent*  event);
virtual void dragMoveEvent(QDragMoveEvent*  event);
virtual void drawBackground(QPainter*  painter, const QRectF&  rect);
virtual void drawForeground(QPainter*  painter, const QRectF&  rect);
virtual void drawItems(QPainter*  painter, int  numItems, QGraphicsItem**  items, const QStyleOptionGraphicsItem*  options);
virtual void dropEvent(QDropEvent*  event);
virtual void enterEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  event);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void focusInEvent(QFocusEvent*  event);
virtual bool  focusNextPrevChild(bool  next);
virtual void focusOutEvent(QFocusEvent*  event);
virtual int  heightForWidth(int  arg__1) const;
virtual void hideEvent(QHideEvent*  arg__1);
virtual void inputMethodEvent(QInputMethodEvent*  event);
virtual QVariant  inputMethodQuery(Qt::InputMethodQuery  query) const;
virtual void keyPressEvent(QKeyEvent*  event);
virtual void keyReleaseEvent(QKeyEvent*  event);
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
virtual void resizeEvent(QResizeEvent*  event);
virtual void scrollContentsBy(int  dx, int  dy);
virtual void showEvent(QShowEvent*  event);
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual bool  viewportEvent(QEvent*  event);
virtual void wheelEvent(QWheelEvent*  event);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QGraphicsView : public QGraphicsView
{ public:
inline void promoted_contextMenuEvent(QContextMenuEvent*  event) { QGraphicsView::contextMenuEvent(event); }
inline void promoted_dragEnterEvent(QDragEnterEvent*  event) { QGraphicsView::dragEnterEvent(event); }
inline void promoted_dragLeaveEvent(QDragLeaveEvent*  event) { QGraphicsView::dragLeaveEvent(event); }
inline void promoted_dragMoveEvent(QDragMoveEvent*  event) { QGraphicsView::dragMoveEvent(event); }
inline void promoted_drawBackground(QPainter*  painter, const QRectF&  rect) { QGraphicsView::drawBackground(painter, rect); }
inline void promoted_drawForeground(QPainter*  painter, const QRectF&  rect) { QGraphicsView::drawForeground(painter, rect); }
inline void promoted_drawItems(QPainter*  painter, int  numItems, QGraphicsItem**  items, const QStyleOptionGraphicsItem*  options) { QGraphicsView::drawItems(painter, numItems, items, options); }
inline void promoted_dropEvent(QDropEvent*  event) { QGraphicsView::dropEvent(event); }
inline bool  promoted_event(QEvent*  event) { return QGraphicsView::event(event); }
inline void promoted_focusInEvent(QFocusEvent*  event) { QGraphicsView::focusInEvent(event); }
inline bool  promoted_focusNextPrevChild(bool  next) { return QGraphicsView::focusNextPrevChild(next); }
inline void promoted_focusOutEvent(QFocusEvent*  event) { QGraphicsView::focusOutEvent(event); }
inline void promoted_inputMethodEvent(QInputMethodEvent*  event) { QGraphicsView::inputMethodEvent(event); }
inline QVariant  promoted_inputMethodQuery(Qt::InputMethodQuery  query) const { return QGraphicsView::inputMethodQuery(query); }
inline void promoted_keyPressEvent(QKeyEvent*  event) { QGraphicsView::keyPressEvent(event); }
inline void promoted_keyReleaseEvent(QKeyEvent*  event) { QGraphicsView::keyReleaseEvent(event); }
inline void promoted_mouseDoubleClickEvent(QMouseEvent*  event) { QGraphicsView::mouseDoubleClickEvent(event); }
inline void promoted_mouseMoveEvent(QMouseEvent*  event) { QGraphicsView::mouseMoveEvent(event); }
inline void promoted_mousePressEvent(QMouseEvent*  event) { QGraphicsView::mousePressEvent(event); }
inline void promoted_mouseReleaseEvent(QMouseEvent*  event) { QGraphicsView::mouseReleaseEvent(event); }
inline void promoted_paintEvent(QPaintEvent*  event) { QGraphicsView::paintEvent(event); }
inline void promoted_resizeEvent(QResizeEvent*  event) { QGraphicsView::resizeEvent(event); }
inline void promoted_scrollContentsBy(int  dx, int  dy) { QGraphicsView::scrollContentsBy(dx, dy); }
inline void promoted_showEvent(QShowEvent*  event) { QGraphicsView::showEvent(event); }
inline bool  promoted_viewportEvent(QEvent*  event) { return QGraphicsView::viewportEvent(event); }
inline void promoted_wheelEvent(QWheelEvent*  event) { QGraphicsView::wheelEvent(event); }
};

class PythonQtWrapper_QGraphicsView : public QObject
{ Q_OBJECT
public:
Q_ENUMS(OptimizationFlag CacheModeFlag )
Q_FLAGS(OptimizationFlags CacheMode )
enum OptimizationFlag{
  DontClipPainter = QGraphicsView::DontClipPainter,   DontSavePainterState = QGraphicsView::DontSavePainterState,   DontAdjustForAntialiasing = QGraphicsView::DontAdjustForAntialiasing,   IndirectPainting = QGraphicsView::IndirectPainting};
enum CacheModeFlag{
  CacheNone = QGraphicsView::CacheNone,   CacheBackground = QGraphicsView::CacheBackground};
Q_DECLARE_FLAGS(OptimizationFlags, OptimizationFlag)
Q_DECLARE_FLAGS(CacheMode, CacheModeFlag)
public slots:
QGraphicsView* new_QGraphicsView(QGraphicsScene*  scene, QWidget*  parent = 0);
QGraphicsView* new_QGraphicsView(QWidget*  parent = 0);
void delete_QGraphicsView(QGraphicsView* obj) { delete obj; } 
   Qt::Alignment  alignment(QGraphicsView* theWrappedObject) const;
   QBrush  backgroundBrush(QGraphicsView* theWrappedObject) const;
   QGraphicsView::CacheMode  cacheMode(QGraphicsView* theWrappedObject) const;
   void centerOn(QGraphicsView* theWrappedObject, const QGraphicsItem*  item);
   void centerOn(QGraphicsView* theWrappedObject, const QPointF&  pos);
   void centerOn(QGraphicsView* theWrappedObject, qreal  x, qreal  y);
   void contextMenuEvent(QGraphicsView* theWrappedObject, QContextMenuEvent*  event);
   void dragEnterEvent(QGraphicsView* theWrappedObject, QDragEnterEvent*  event);
   void dragLeaveEvent(QGraphicsView* theWrappedObject, QDragLeaveEvent*  event);
   QGraphicsView::DragMode  dragMode(QGraphicsView* theWrappedObject) const;
   void dragMoveEvent(QGraphicsView* theWrappedObject, QDragMoveEvent*  event);
   void drawBackground(QGraphicsView* theWrappedObject, QPainter*  painter, const QRectF&  rect);
   void drawForeground(QGraphicsView* theWrappedObject, QPainter*  painter, const QRectF&  rect);
   void drawItems(QGraphicsView* theWrappedObject, QPainter*  painter, int  numItems, QGraphicsItem**  items, const QStyleOptionGraphicsItem*  options);
   void dropEvent(QGraphicsView* theWrappedObject, QDropEvent*  event);
   void ensureVisible(QGraphicsView* theWrappedObject, const QGraphicsItem*  item, int  xmargin = 50, int  ymargin = 50);
   void ensureVisible(QGraphicsView* theWrappedObject, const QRectF&  rect, int  xmargin = 50, int  ymargin = 50);
   void ensureVisible(QGraphicsView* theWrappedObject, qreal  x, qreal  y, qreal  w, qreal  h, int  xmargin = 50, int  ymargin = 50);
   bool  event(QGraphicsView* theWrappedObject, QEvent*  event);
   void fitInView(QGraphicsView* theWrappedObject, const QGraphicsItem*  item, Qt::AspectRatioMode  aspectRadioMode = Qt::IgnoreAspectRatio);
   void fitInView(QGraphicsView* theWrappedObject, const QRectF&  rect, Qt::AspectRatioMode  aspectRadioMode = Qt::IgnoreAspectRatio);
   void fitInView(QGraphicsView* theWrappedObject, qreal  x, qreal  y, qreal  w, qreal  h, Qt::AspectRatioMode  aspectRadioMode = Qt::IgnoreAspectRatio);
   void focusInEvent(QGraphicsView* theWrappedObject, QFocusEvent*  event);
   bool  focusNextPrevChild(QGraphicsView* theWrappedObject, bool  next);
   void focusOutEvent(QGraphicsView* theWrappedObject, QFocusEvent*  event);
   QBrush  foregroundBrush(QGraphicsView* theWrappedObject) const;
   void inputMethodEvent(QGraphicsView* theWrappedObject, QInputMethodEvent*  event);
   QVariant  inputMethodQuery(QGraphicsView* theWrappedObject, Qt::InputMethodQuery  query) const;
   bool  isInteractive(QGraphicsView* theWrappedObject) const;
   bool  isTransformed(QGraphicsView* theWrappedObject) const;
   QGraphicsItem*  itemAt(QGraphicsView* theWrappedObject, const QPoint&  pos) const;
   QGraphicsItem*  itemAt(QGraphicsView* theWrappedObject, int  x, int  y) const;
   QList<QGraphicsItem* >  items(QGraphicsView* theWrappedObject) const;
   QList<QGraphicsItem* >  items(QGraphicsView* theWrappedObject, const QPainterPath&  path, Qt::ItemSelectionMode  mode = Qt::IntersectsItemShape) const;
   QList<QGraphicsItem* >  items(QGraphicsView* theWrappedObject, const QPoint&  pos) const;
   QList<QGraphicsItem* >  items(QGraphicsView* theWrappedObject, const QPolygon&  polygon, Qt::ItemSelectionMode  mode = Qt::IntersectsItemShape) const;
   QList<QGraphicsItem* >  items(QGraphicsView* theWrappedObject, const QRect&  rect, Qt::ItemSelectionMode  mode = Qt::IntersectsItemShape) const;
   QList<QGraphicsItem* >  items(QGraphicsView* theWrappedObject, int  x, int  y) const;
   QList<QGraphicsItem* >  items(QGraphicsView* theWrappedObject, int  x, int  y, int  w, int  h, Qt::ItemSelectionMode  mode = Qt::IntersectsItemShape) const;
   void keyPressEvent(QGraphicsView* theWrappedObject, QKeyEvent*  event);
   void keyReleaseEvent(QGraphicsView* theWrappedObject, QKeyEvent*  event);
   QPainterPath  mapFromScene(QGraphicsView* theWrappedObject, const QPainterPath&  path) const;
   QPoint  mapFromScene(QGraphicsView* theWrappedObject, const QPointF&  point) const;
   QPolygon  mapFromScene(QGraphicsView* theWrappedObject, const QPolygonF&  polygon) const;
   QPolygon  mapFromScene(QGraphicsView* theWrappedObject, const QRectF&  rect) const;
   QPoint  mapFromScene(QGraphicsView* theWrappedObject, qreal  x, qreal  y) const;
   QPolygon  mapFromScene(QGraphicsView* theWrappedObject, qreal  x, qreal  y, qreal  w, qreal  h) const;
   QPainterPath  mapToScene(QGraphicsView* theWrappedObject, const QPainterPath&  path) const;
   QPointF  mapToScene(QGraphicsView* theWrappedObject, const QPoint&  point) const;
   QPolygonF  mapToScene(QGraphicsView* theWrappedObject, const QPolygon&  polygon) const;
   QPolygonF  mapToScene(QGraphicsView* theWrappedObject, const QRect&  rect) const;
   QPointF  mapToScene(QGraphicsView* theWrappedObject, int  x, int  y) const;
   QPolygonF  mapToScene(QGraphicsView* theWrappedObject, int  x, int  y, int  w, int  h) const;
   QMatrix  matrix(QGraphicsView* theWrappedObject) const;
   void mouseDoubleClickEvent(QGraphicsView* theWrappedObject, QMouseEvent*  event);
   void mouseMoveEvent(QGraphicsView* theWrappedObject, QMouseEvent*  event);
   void mousePressEvent(QGraphicsView* theWrappedObject, QMouseEvent*  event);
   void mouseReleaseEvent(QGraphicsView* theWrappedObject, QMouseEvent*  event);
   QGraphicsView::OptimizationFlags  optimizationFlags(QGraphicsView* theWrappedObject) const;
   void paintEvent(QGraphicsView* theWrappedObject, QPaintEvent*  event);
   void render(QGraphicsView* theWrappedObject, QPainter*  painter, const QRectF&  target = QRectF(), const QRect&  source = QRect(), Qt::AspectRatioMode  aspectRatioMode = Qt::KeepAspectRatio);
   QPainter::RenderHints  renderHints(QGraphicsView* theWrappedObject) const;
   void resetCachedContent(QGraphicsView* theWrappedObject);
   void resetMatrix(QGraphicsView* theWrappedObject);
   void resetTransform(QGraphicsView* theWrappedObject);
   QGraphicsView::ViewportAnchor  resizeAnchor(QGraphicsView* theWrappedObject) const;
   void resizeEvent(QGraphicsView* theWrappedObject, QResizeEvent*  event);
   void rotate(QGraphicsView* theWrappedObject, qreal  angle);
   Qt::ItemSelectionMode  rubberBandSelectionMode(QGraphicsView* theWrappedObject) const;
   void scale(QGraphicsView* theWrappedObject, qreal  sx, qreal  sy);
   QGraphicsScene*  scene(QGraphicsView* theWrappedObject) const;
   QRectF  sceneRect(QGraphicsView* theWrappedObject) const;
   void scrollContentsBy(QGraphicsView* theWrappedObject, int  dx, int  dy);
   void setAlignment(QGraphicsView* theWrappedObject, Qt::Alignment  alignment);
   void setBackgroundBrush(QGraphicsView* theWrappedObject, const QBrush&  brush);
   void setCacheMode(QGraphicsView* theWrappedObject, QGraphicsView::CacheMode  mode);
   void setDragMode(QGraphicsView* theWrappedObject, QGraphicsView::DragMode  mode);
   void setForegroundBrush(QGraphicsView* theWrappedObject, const QBrush&  brush);
   void setInteractive(QGraphicsView* theWrappedObject, bool  allowed);
   void setMatrix(QGraphicsView* theWrappedObject, const QMatrix&  matrix, bool  combine = false);
   void setOptimizationFlag(QGraphicsView* theWrappedObject, QGraphicsView::OptimizationFlag  flag, bool  enabled = true);
   void setOptimizationFlags(QGraphicsView* theWrappedObject, QGraphicsView::OptimizationFlags  flags);
   void setRenderHint(QGraphicsView* theWrappedObject, QPainter::RenderHint  hint, bool  enabled = true);
   void setRenderHints(QGraphicsView* theWrappedObject, QPainter::RenderHints  hints);
   void setResizeAnchor(QGraphicsView* theWrappedObject, QGraphicsView::ViewportAnchor  anchor);
   void setRubberBandSelectionMode(QGraphicsView* theWrappedObject, Qt::ItemSelectionMode  mode);
   void setScene(QGraphicsView* theWrappedObject, QGraphicsScene*  scene);
   void setSceneRect(QGraphicsView* theWrappedObject, const QRectF&  rect);
   void setSceneRect(QGraphicsView* theWrappedObject, qreal  x, qreal  y, qreal  w, qreal  h);
   void setTransform(QGraphicsView* theWrappedObject, const QTransform&  matrix, bool  combine = false);
   void setTransformationAnchor(QGraphicsView* theWrappedObject, QGraphicsView::ViewportAnchor  anchor);
   void setViewportUpdateMode(QGraphicsView* theWrappedObject, QGraphicsView::ViewportUpdateMode  mode);
   void shear(QGraphicsView* theWrappedObject, qreal  sh, qreal  sv);
   void showEvent(QGraphicsView* theWrappedObject, QShowEvent*  event);
   QSize  sizeHint(QGraphicsView* theWrappedObject) const;
   QTransform  transform(QGraphicsView* theWrappedObject) const;
   QGraphicsView::ViewportAnchor  transformationAnchor(QGraphicsView* theWrappedObject) const;
   void translate(QGraphicsView* theWrappedObject, qreal  dx, qreal  dy);
   bool  viewportEvent(QGraphicsView* theWrappedObject, QEvent*  event);
   QTransform  viewportTransform(QGraphicsView* theWrappedObject) const;
   QGraphicsView::ViewportUpdateMode  viewportUpdateMode(QGraphicsView* theWrappedObject) const;
   void wheelEvent(QGraphicsView* theWrappedObject, QWheelEvent*  event);
};





class PythonQtShell_QGraphicsWidget : public QGraphicsWidget
{
public:
    PythonQtShell_QGraphicsWidget(QGraphicsItem*  parent = 0, Qt::WindowFlags  wFlags = 0):QGraphicsWidget(parent, wFlags),_wrapper(NULL) {};

virtual void changeEvent(QEvent*  event);
virtual void childEvent(QChildEvent*  arg__1);
virtual void closeEvent(QCloseEvent*  event);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  event);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual bool  focusNextPrevChild(bool  next);
virtual void getContentsMargins(qreal*  left, qreal*  top, qreal*  right, qreal*  bottom) const;
virtual void grabKeyboardEvent(QEvent*  event);
virtual void grabMouseEvent(QEvent*  event);
virtual void hideEvent(QHideEvent*  event);
virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent*  event);
virtual void hoverMoveEvent(QGraphicsSceneHoverEvent*  event);
virtual void initStyleOption(QStyleOption*  option) const;
virtual void moveEvent(QGraphicsSceneMoveEvent*  event);
virtual void paintWindowFrame(QPainter*  painter, const QStyleOptionGraphicsItem*  option, QWidget*  widget = 0);
virtual void polishEvent();
virtual QVariant  propertyChange(const QString&  propertyName, const QVariant&  value);
virtual void resizeEvent(QGraphicsSceneResizeEvent*  event);
virtual void setGeometry(const QRectF&  rect);
virtual void showEvent(QShowEvent*  event);
virtual QSizeF  sizeHint(Qt::SizeHint  which, const QSizeF&  constraint = QSizeF()) const;
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void ungrabKeyboardEvent(QEvent*  event);
virtual void ungrabMouseEvent(QEvent*  event);
virtual void updateGeometry();
virtual bool  windowFrameEvent(QEvent*  e);
virtual Qt::WindowFrameSection  windowFrameSectionAt(const QPointF&  pos) const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QGraphicsWidget : public QGraphicsWidget
{ public:
inline void promoted_changeEvent(QEvent*  event) { QGraphicsWidget::changeEvent(event); }
inline void promoted_closeEvent(QCloseEvent*  event) { QGraphicsWidget::closeEvent(event); }
inline bool  promoted_event(QEvent*  event) { return QGraphicsWidget::event(event); }
inline bool  promoted_focusNextPrevChild(bool  next) { return QGraphicsWidget::focusNextPrevChild(next); }
inline void promoted_getContentsMargins(qreal*  left, qreal*  top, qreal*  right, qreal*  bottom) const { QGraphicsWidget::getContentsMargins(left, top, right, bottom); }
inline void promoted_grabKeyboardEvent(QEvent*  event) { QGraphicsWidget::grabKeyboardEvent(event); }
inline void promoted_grabMouseEvent(QEvent*  event) { QGraphicsWidget::grabMouseEvent(event); }
inline void promoted_hideEvent(QHideEvent*  event) { QGraphicsWidget::hideEvent(event); }
inline void promoted_hoverLeaveEvent(QGraphicsSceneHoverEvent*  event) { QGraphicsWidget::hoverLeaveEvent(event); }
inline void promoted_hoverMoveEvent(QGraphicsSceneHoverEvent*  event) { QGraphicsWidget::hoverMoveEvent(event); }
inline void promoted_initStyleOption(QStyleOption*  option) const { QGraphicsWidget::initStyleOption(option); }
inline void promoted_moveEvent(QGraphicsSceneMoveEvent*  event) { QGraphicsWidget::moveEvent(event); }
inline void promoted_paintWindowFrame(QPainter*  painter, const QStyleOptionGraphicsItem*  option, QWidget*  widget = 0) { QGraphicsWidget::paintWindowFrame(painter, option, widget); }
inline void promoted_polishEvent() { QGraphicsWidget::polishEvent(); }
inline QVariant  promoted_propertyChange(const QString&  propertyName, const QVariant&  value) { return QGraphicsWidget::propertyChange(propertyName, value); }
inline void promoted_resizeEvent(QGraphicsSceneResizeEvent*  event) { QGraphicsWidget::resizeEvent(event); }
inline void promoted_setGeometry(const QRectF&  rect) { QGraphicsWidget::setGeometry(rect); }
inline void promoted_showEvent(QShowEvent*  event) { QGraphicsWidget::showEvent(event); }
inline QSizeF  promoted_sizeHint(Qt::SizeHint  which, const QSizeF&  constraint = QSizeF()) const { return QGraphicsWidget::sizeHint(which, constraint); }
inline void promoted_ungrabKeyboardEvent(QEvent*  event) { QGraphicsWidget::ungrabKeyboardEvent(event); }
inline void promoted_ungrabMouseEvent(QEvent*  event) { QGraphicsWidget::ungrabMouseEvent(event); }
inline void promoted_updateGeometry() { QGraphicsWidget::updateGeometry(); }
inline bool  promoted_windowFrameEvent(QEvent*  e) { return QGraphicsWidget::windowFrameEvent(e); }
inline Qt::WindowFrameSection  promoted_windowFrameSectionAt(const QPointF&  pos) const { return QGraphicsWidget::windowFrameSectionAt(pos); }
};

class PythonQtWrapper_QGraphicsWidget : public QObject
{ Q_OBJECT
public:
Q_ENUMS(enum_1 )
enum enum_1{
  Type = QGraphicsWidget::Type};
public slots:
QGraphicsWidget* new_QGraphicsWidget(QGraphicsItem*  parent = 0, Qt::WindowFlags  wFlags = 0);
void delete_QGraphicsWidget(QGraphicsWidget* obj) { delete obj; } 
   QList<QAction* >  actions(QGraphicsWidget* theWrappedObject) const;
   void addAction(QGraphicsWidget* theWrappedObject, QAction*  action);
   void addActions(QGraphicsWidget* theWrappedObject, QList<QAction* >  actions);
   void adjustSize(QGraphicsWidget* theWrappedObject);
   QRectF  boundingRect(QGraphicsWidget* theWrappedObject) const;
   void changeEvent(QGraphicsWidget* theWrappedObject, QEvent*  event);
   void closeEvent(QGraphicsWidget* theWrappedObject, QCloseEvent*  event);
   bool  event(QGraphicsWidget* theWrappedObject, QEvent*  event);
   bool  focusNextPrevChild(QGraphicsWidget* theWrappedObject, bool  next);
   Qt::FocusPolicy  focusPolicy(QGraphicsWidget* theWrappedObject) const;
   QGraphicsWidget*  focusWidget(QGraphicsWidget* theWrappedObject) const;
   QFont  font(QGraphicsWidget* theWrappedObject) const;
   void getContentsMargins(QGraphicsWidget* theWrappedObject, qreal*  left, qreal*  top, qreal*  right, qreal*  bottom) const;
   void getWindowFrameMargins(QGraphicsWidget* theWrappedObject, qreal*  left, qreal*  top, qreal*  right, qreal*  bottom) const;
   void grabKeyboardEvent(QGraphicsWidget* theWrappedObject, QEvent*  event);
   void grabMouseEvent(QGraphicsWidget* theWrappedObject, QEvent*  event);
   int  grabShortcut(QGraphicsWidget* theWrappedObject, const QKeySequence&  sequence, Qt::ShortcutContext  context = Qt::WindowShortcut);
   void hideEvent(QGraphicsWidget* theWrappedObject, QHideEvent*  event);
   void hoverLeaveEvent(QGraphicsWidget* theWrappedObject, QGraphicsSceneHoverEvent*  event);
   void hoverMoveEvent(QGraphicsWidget* theWrappedObject, QGraphicsSceneHoverEvent*  event);
   void initStyleOption(QGraphicsWidget* theWrappedObject, QStyleOption*  option) const;
   void insertAction(QGraphicsWidget* theWrappedObject, QAction*  before, QAction*  action);
   void insertActions(QGraphicsWidget* theWrappedObject, QAction*  before, QList<QAction* >  actions);
   bool  isActiveWindow(QGraphicsWidget* theWrappedObject) const;
   QGraphicsLayout*  layout(QGraphicsWidget* theWrappedObject) const;
   Qt::LayoutDirection  layoutDirection(QGraphicsWidget* theWrappedObject) const;
   void moveEvent(QGraphicsWidget* theWrappedObject, QGraphicsSceneMoveEvent*  event);
   void paint(QGraphicsWidget* theWrappedObject, QPainter*  painter, const QStyleOptionGraphicsItem*  option, QWidget*  widget = 0);
   void paintWindowFrame(QGraphicsWidget* theWrappedObject, QPainter*  painter, const QStyleOptionGraphicsItem*  option, QWidget*  widget = 0);
   QPalette  palette(QGraphicsWidget* theWrappedObject) const;
   void polishEvent(QGraphicsWidget* theWrappedObject);
   QVariant  propertyChange(QGraphicsWidget* theWrappedObject, const QString&  propertyName, const QVariant&  value);
   QRectF  rect(QGraphicsWidget* theWrappedObject) const;
   void releaseShortcut(QGraphicsWidget* theWrappedObject, int  id);
   void removeAction(QGraphicsWidget* theWrappedObject, QAction*  action);
   void resize(QGraphicsWidget* theWrappedObject, const QSizeF&  size);
   void resize(QGraphicsWidget* theWrappedObject, qreal  w, qreal  h);
   void resizeEvent(QGraphicsWidget* theWrappedObject, QGraphicsSceneResizeEvent*  event);
   void setAttribute(QGraphicsWidget* theWrappedObject, Qt::WidgetAttribute  attribute, bool  on = true);
   void setContentsMargins(QGraphicsWidget* theWrappedObject, qreal  left, qreal  top, qreal  right, qreal  bottom);
   void setFocusPolicy(QGraphicsWidget* theWrappedObject, Qt::FocusPolicy  policy);
   void setFont(QGraphicsWidget* theWrappedObject, const QFont&  font);
   void setGeometry(QGraphicsWidget* theWrappedObject, const QRectF&  rect);
   void setGeometry(QGraphicsWidget* theWrappedObject, qreal  x, qreal  y, qreal  w, qreal  h);
   void setLayout(QGraphicsWidget* theWrappedObject, QGraphicsLayout*  layout);
   void setLayoutDirection(QGraphicsWidget* theWrappedObject, Qt::LayoutDirection  direction);
   void setPalette(QGraphicsWidget* theWrappedObject, const QPalette&  palette);
   void setShortcutAutoRepeat(QGraphicsWidget* theWrappedObject, int  id, bool  enabled = true);
   void setShortcutEnabled(QGraphicsWidget* theWrappedObject, int  id, bool  enabled = true);
   void setStyle(QGraphicsWidget* theWrappedObject, QStyle*  style);
   void static_QGraphicsWidget_setTabOrder(QGraphicsWidget*  first, QGraphicsWidget*  second);
   void setWindowFlags(QGraphicsWidget* theWrappedObject, Qt::WindowFlags  wFlags);
   void setWindowFrameMargins(QGraphicsWidget* theWrappedObject, qreal  left, qreal  top, qreal  right, qreal  bottom);
   void setWindowTitle(QGraphicsWidget* theWrappedObject, const QString&  title);
   QPainterPath  shape(QGraphicsWidget* theWrappedObject) const;
   void showEvent(QGraphicsWidget* theWrappedObject, QShowEvent*  event);
   QSizeF  size(QGraphicsWidget* theWrappedObject) const;
   QSizeF  sizeHint(QGraphicsWidget* theWrappedObject, Qt::SizeHint  which, const QSizeF&  constraint = QSizeF()) const;
   QStyle*  style(QGraphicsWidget* theWrappedObject) const;
   bool  testAttribute(QGraphicsWidget* theWrappedObject, Qt::WidgetAttribute  attribute) const;
   int  type(QGraphicsWidget* theWrappedObject) const;
   void ungrabKeyboardEvent(QGraphicsWidget* theWrappedObject, QEvent*  event);
   void ungrabMouseEvent(QGraphicsWidget* theWrappedObject, QEvent*  event);
   void unsetLayoutDirection(QGraphicsWidget* theWrappedObject);
   void unsetWindowFrameMargins(QGraphicsWidget* theWrappedObject);
   void updateGeometry(QGraphicsWidget* theWrappedObject);
   Qt::WindowFlags  windowFlags(QGraphicsWidget* theWrappedObject) const;
   bool  windowFrameEvent(QGraphicsWidget* theWrappedObject, QEvent*  e);
   QRectF  windowFrameGeometry(QGraphicsWidget* theWrappedObject) const;
   QRectF  windowFrameRect(QGraphicsWidget* theWrappedObject) const;
   Qt::WindowFrameSection  windowFrameSectionAt(QGraphicsWidget* theWrappedObject, const QPointF&  pos) const;
   QString  windowTitle(QGraphicsWidget* theWrappedObject) const;
   Qt::WindowType  windowType(QGraphicsWidget* theWrappedObject) const;
};





class PythonQtShell_QGridLayout : public QGridLayout
{
public:
    PythonQtShell_QGridLayout():QGridLayout(),_wrapper(NULL) {};
    PythonQtShell_QGridLayout(QWidget*  parent):QGridLayout(parent),_wrapper(NULL) {};

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
virtual QLayoutItem*  itemAt(int  index) const;
virtual QLayout*  layout();
virtual QSize  maximumSize() const;
virtual QSize  minimumSize() const;
virtual void setGeometry(const QRect&  arg__1);
virtual QLayoutItem*  takeAt(int  index);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QGridLayout : public QGridLayout
{ public:
inline void promoted_addItem(QLayoutItem*  arg__1) { QGridLayout::addItem(arg__1); }
inline int  promoted_count() const { return QGridLayout::count(); }
inline Qt::Orientations  promoted_expandingDirections() const { return QGridLayout::expandingDirections(); }
inline void promoted_invalidate() { QGridLayout::invalidate(); }
inline QLayoutItem*  promoted_itemAt(int  index) const { return QGridLayout::itemAt(index); }
inline QSize  promoted_maximumSize() const { return QGridLayout::maximumSize(); }
inline QSize  promoted_minimumSize() const { return QGridLayout::minimumSize(); }
inline void promoted_setGeometry(const QRect&  arg__1) { QGridLayout::setGeometry(arg__1); }
inline QLayoutItem*  promoted_takeAt(int  index) { return QGridLayout::takeAt(index); }
};

class PythonQtWrapper_QGridLayout : public QObject
{ Q_OBJECT
public:
public slots:
QGridLayout* new_QGridLayout();
QGridLayout* new_QGridLayout(QWidget*  parent);
void delete_QGridLayout(QGridLayout* obj) { delete obj; } 
   void addItem(QGridLayout* theWrappedObject, QLayoutItem*  arg__1);
   void addItem(QGridLayout* theWrappedObject, QLayoutItem*  item, int  row, int  column, int  rowSpan = 1, int  columnSpan = 1, Qt::Alignment  arg__6 = 0);
   void addLayout(QGridLayout* theWrappedObject, QLayout*  arg__1, int  row, int  column, Qt::Alignment  arg__4 = 0);
   void addLayout(QGridLayout* theWrappedObject, QLayout*  arg__1, int  row, int  column, int  rowSpan, int  columnSpan, Qt::Alignment  arg__6 = 0);
   void addWidget(QGridLayout* theWrappedObject, QWidget*  arg__1, int  row, int  column, Qt::Alignment  arg__4 = 0);
   void addWidget(QGridLayout* theWrappedObject, QWidget*  arg__1, int  row, int  column, int  rowSpan, int  columnSpan, Qt::Alignment  arg__6 = 0);
   QRect  cellRect(QGridLayout* theWrappedObject, int  row, int  column) const;
   int  columnCount(QGridLayout* theWrappedObject) const;
   int  columnMinimumWidth(QGridLayout* theWrappedObject, int  column) const;
   int  columnStretch(QGridLayout* theWrappedObject, int  column) const;
   int  count(QGridLayout* theWrappedObject) const;
   Qt::Orientations  expandingDirections(QGridLayout* theWrappedObject) const;
   void getItemPosition(QGridLayout* theWrappedObject, int  idx, int*  row, int*  column, int*  rowSpan, int*  columnSpan);
   bool  hasHeightForWidth(QGridLayout* theWrappedObject) const;
   int  heightForWidth(QGridLayout* theWrappedObject, int  arg__1) const;
   int  horizontalSpacing(QGridLayout* theWrappedObject) const;
   void invalidate(QGridLayout* theWrappedObject);
   QLayoutItem*  itemAt(QGridLayout* theWrappedObject, int  index) const;
   QLayoutItem*  itemAtPosition(QGridLayout* theWrappedObject, int  row, int  column) const;
   QSize  maximumSize(QGridLayout* theWrappedObject) const;
   int  minimumHeightForWidth(QGridLayout* theWrappedObject, int  arg__1) const;
   QSize  minimumSize(QGridLayout* theWrappedObject) const;
   Qt::Corner  originCorner(QGridLayout* theWrappedObject) const;
   int  rowCount(QGridLayout* theWrappedObject) const;
   int  rowMinimumHeight(QGridLayout* theWrappedObject, int  row) const;
   int  rowStretch(QGridLayout* theWrappedObject, int  row) const;
   void setColumnMinimumWidth(QGridLayout* theWrappedObject, int  column, int  minSize);
   void setColumnStretch(QGridLayout* theWrappedObject, int  column, int  stretch);
   void setDefaultPositioning(QGridLayout* theWrappedObject, int  n, Qt::Orientation  orient);
   void setGeometry(QGridLayout* theWrappedObject, const QRect&  arg__1);
   void setHorizontalSpacing(QGridLayout* theWrappedObject, int  spacing);
   void setOriginCorner(QGridLayout* theWrappedObject, Qt::Corner  arg__1);
   void setRowMinimumHeight(QGridLayout* theWrappedObject, int  row, int  minSize);
   void setRowStretch(QGridLayout* theWrappedObject, int  row, int  stretch);
   void setSpacing(QGridLayout* theWrappedObject, int  spacing);
   void setVerticalSpacing(QGridLayout* theWrappedObject, int  spacing);
   QSize  sizeHint(QGridLayout* theWrappedObject) const;
   int  spacing(QGridLayout* theWrappedObject) const;
   QLayoutItem*  takeAt(QGridLayout* theWrappedObject, int  index);
   int  verticalSpacing(QGridLayout* theWrappedObject) const;
};





class PythonQtShell_QGroupBox : public QGroupBox
{
public:
    PythonQtShell_QGroupBox(QWidget*  parent = 0):QGroupBox(parent),_wrapper(NULL) {};
    PythonQtShell_QGroupBox(const QString&  title, QWidget*  parent = 0):QGroupBox(title, parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  event);
virtual void childEvent(QChildEvent*  event);
virtual void closeEvent(QCloseEvent*  arg__1);
virtual void contextMenuEvent(QContextMenuEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual int  devType() const;
virtual void dragEnterEvent(QDragEnterEvent*  arg__1);
virtual void dragLeaveEvent(QDragLeaveEvent*  arg__1);
virtual void dragMoveEvent(QDragMoveEvent*  arg__1);
virtual void dropEvent(QDropEvent*  arg__1);
virtual void enterEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  event);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void focusInEvent(QFocusEvent*  event);
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
virtual void mouseDoubleClickEvent(QMouseEvent*  arg__1);
virtual void mouseMoveEvent(QMouseEvent*  event);
virtual void mousePressEvent(QMouseEvent*  event);
virtual void mouseReleaseEvent(QMouseEvent*  event);
virtual void moveEvent(QMoveEvent*  arg__1);
virtual QPaintEngine*  paintEngine() const;
virtual void paintEvent(QPaintEvent*  event);
virtual void resizeEvent(QResizeEvent*  event);
virtual void showEvent(QShowEvent*  arg__1);
virtual QSize  sizeHint() const;
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QGroupBox : public QGroupBox
{ public:
inline void promoted_changeEvent(QEvent*  event) { QGroupBox::changeEvent(event); }
inline void promoted_childEvent(QChildEvent*  event) { QGroupBox::childEvent(event); }
inline bool  promoted_event(QEvent*  event) { return QGroupBox::event(event); }
inline void promoted_focusInEvent(QFocusEvent*  event) { QGroupBox::focusInEvent(event); }
inline void promoted_mouseMoveEvent(QMouseEvent*  event) { QGroupBox::mouseMoveEvent(event); }
inline void promoted_mousePressEvent(QMouseEvent*  event) { QGroupBox::mousePressEvent(event); }
inline void promoted_mouseReleaseEvent(QMouseEvent*  event) { QGroupBox::mouseReleaseEvent(event); }
inline void promoted_paintEvent(QPaintEvent*  event) { QGroupBox::paintEvent(event); }
inline void promoted_resizeEvent(QResizeEvent*  event) { QGroupBox::resizeEvent(event); }
};

class PythonQtWrapper_QGroupBox : public QObject
{ Q_OBJECT
public:
public slots:
QGroupBox* new_QGroupBox(QWidget*  parent = 0);
QGroupBox* new_QGroupBox(const QString&  title, QWidget*  parent = 0);
void delete_QGroupBox(QGroupBox* obj) { delete obj; } 
   Qt::Alignment  alignment(QGroupBox* theWrappedObject) const;
   void changeEvent(QGroupBox* theWrappedObject, QEvent*  event);
   void childEvent(QGroupBox* theWrappedObject, QChildEvent*  event);
   bool  event(QGroupBox* theWrappedObject, QEvent*  event);
   void focusInEvent(QGroupBox* theWrappedObject, QFocusEvent*  event);
   bool  isCheckable(QGroupBox* theWrappedObject) const;
   bool  isChecked(QGroupBox* theWrappedObject) const;
   bool  isFlat(QGroupBox* theWrappedObject) const;
   QSize  minimumSizeHint(QGroupBox* theWrappedObject) const;
   void mouseMoveEvent(QGroupBox* theWrappedObject, QMouseEvent*  event);
   void mousePressEvent(QGroupBox* theWrappedObject, QMouseEvent*  event);
   void mouseReleaseEvent(QGroupBox* theWrappedObject, QMouseEvent*  event);
   void paintEvent(QGroupBox* theWrappedObject, QPaintEvent*  event);
   void resizeEvent(QGroupBox* theWrappedObject, QResizeEvent*  event);
   void setAlignment(QGroupBox* theWrappedObject, int  alignment);
   void setCheckable(QGroupBox* theWrappedObject, bool  checkable);
   void setFlat(QGroupBox* theWrappedObject, bool  flat);
   void setTitle(QGroupBox* theWrappedObject, const QString&  title);
   QString  title(QGroupBox* theWrappedObject) const;
};





class PythonQtShell_QHBoxLayout : public QHBoxLayout
{
public:
    PythonQtShell_QHBoxLayout():QHBoxLayout(),_wrapper(NULL) {};
    PythonQtShell_QHBoxLayout(QWidget*  parent):QHBoxLayout(parent),_wrapper(NULL) {};

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

class PythonQtWrapper_QHBoxLayout : public QObject
{ Q_OBJECT
public:
public slots:
QHBoxLayout* new_QHBoxLayout();
QHBoxLayout* new_QHBoxLayout(QWidget*  parent);
void delete_QHBoxLayout(QHBoxLayout* obj) { delete obj; } 
};





class PythonQtShell_QHeaderView : public QHeaderView
{
public:
    PythonQtShell_QHeaderView(Qt::Orientation  orientation, QWidget*  parent = 0):QHeaderView(orientation, parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  arg__1);
virtual void childEvent(QChildEvent*  arg__1);
virtual void closeEditor(QWidget*  editor, QAbstractItemDelegate::EndEditHint  hint);
virtual void closeEvent(QCloseEvent*  arg__1);
virtual void commitData(QWidget*  editor);
virtual void contextMenuEvent(QContextMenuEvent*  arg__1);
virtual void currentChanged(const QModelIndex&  current, const QModelIndex&  old);
virtual void customEvent(QEvent*  arg__1);
virtual void dataChanged(const QModelIndex&  topLeft, const QModelIndex&  bottomRight);
virtual int  devType() const;
virtual void doItemsLayout();
virtual void dragEnterEvent(QDragEnterEvent*  event);
virtual void dragLeaveEvent(QDragLeaveEvent*  event);
virtual void dragMoveEvent(QDragMoveEvent*  event);
virtual void dropEvent(QDropEvent*  event);
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
virtual void mouseDoubleClickEvent(QMouseEvent*  e);
virtual void mouseMoveEvent(QMouseEvent*  e);
virtual void mousePressEvent(QMouseEvent*  e);
virtual void mouseReleaseEvent(QMouseEvent*  e);
virtual void moveEvent(QMoveEvent*  arg__1);
virtual QPaintEngine*  paintEngine() const;
virtual void paintEvent(QPaintEvent*  e);
virtual void paintSection(QPainter*  painter, const QRect&  rect, int  logicalIndex) const;
virtual void reset();
virtual void resizeEvent(QResizeEvent*  event);
virtual void rowsAboutToBeRemoved(const QModelIndex&  parent, int  start, int  end);
virtual void rowsInserted(const QModelIndex&  parent, int  start, int  end);
virtual void scrollContentsBy(int  dx, int  dy);
virtual void scrollTo(const QModelIndex&  index, QAbstractItemView::ScrollHint  hint);
virtual QSize  sectionSizeFromContents(int  logicalIndex) const;
virtual void selectAll();
virtual QList<QModelIndex >  selectedIndexes() const;
virtual void selectionChanged(const QItemSelection&  selected, const QItemSelection&  deselected);
virtual QItemSelectionModel::SelectionFlags  selectionCommand(const QModelIndex&  index, const QEvent*  event) const;
virtual void setModel(QAbstractItemModel*  model);
virtual void setRootIndex(const QModelIndex&  index);
virtual void setSelection(const QRect&  rect, QItemSelectionModel::SelectionFlags  flags);
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
virtual bool  viewportEvent(QEvent*  e);
virtual QRect  visualRect(const QModelIndex&  index) const;
virtual QRegion  visualRegionForSelection(const QItemSelection&  selection) const;
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QHeaderView : public QHeaderView
{ public:
inline void promoted_currentChanged(const QModelIndex&  current, const QModelIndex&  old) { QHeaderView::currentChanged(current, old); }
inline void promoted_dataChanged(const QModelIndex&  topLeft, const QModelIndex&  bottomRight) { QHeaderView::dataChanged(topLeft, bottomRight); }
inline void promoted_doItemsLayout() { QHeaderView::doItemsLayout(); }
inline bool  promoted_event(QEvent*  e) { return QHeaderView::event(e); }
inline int  promoted_horizontalOffset() const { return QHeaderView::horizontalOffset(); }
inline QModelIndex  promoted_indexAt(const QPoint&  p) const { return QHeaderView::indexAt(p); }
inline bool  promoted_isIndexHidden(const QModelIndex&  index) const { return QHeaderView::isIndexHidden(index); }
inline void promoted_mouseDoubleClickEvent(QMouseEvent*  e) { QHeaderView::mouseDoubleClickEvent(e); }
inline void promoted_mouseMoveEvent(QMouseEvent*  e) { QHeaderView::mouseMoveEvent(e); }
inline void promoted_mousePressEvent(QMouseEvent*  e) { QHeaderView::mousePressEvent(e); }
inline void promoted_mouseReleaseEvent(QMouseEvent*  e) { QHeaderView::mouseReleaseEvent(e); }
inline void promoted_paintEvent(QPaintEvent*  e) { QHeaderView::paintEvent(e); }
inline void promoted_paintSection(QPainter*  painter, const QRect&  rect, int  logicalIndex) const { QHeaderView::paintSection(painter, rect, logicalIndex); }
inline void promoted_reset() { QHeaderView::reset(); }
inline void promoted_rowsInserted(const QModelIndex&  parent, int  start, int  end) { QHeaderView::rowsInserted(parent, start, end); }
inline void promoted_scrollContentsBy(int  dx, int  dy) { QHeaderView::scrollContentsBy(dx, dy); }
inline void promoted_scrollTo(const QModelIndex&  index, QAbstractItemView::ScrollHint  hint) { QHeaderView::scrollTo(index, hint); }
inline QSize  promoted_sectionSizeFromContents(int  logicalIndex) const { return QHeaderView::sectionSizeFromContents(logicalIndex); }
inline void promoted_setModel(QAbstractItemModel*  model) { QHeaderView::setModel(model); }
inline void promoted_setSelection(const QRect&  rect, QItemSelectionModel::SelectionFlags  flags) { QHeaderView::setSelection(rect, flags); }
inline void promoted_updateGeometries() { QHeaderView::updateGeometries(); }
inline int  promoted_verticalOffset() const { return QHeaderView::verticalOffset(); }
inline bool  promoted_viewportEvent(QEvent*  e) { return QHeaderView::viewportEvent(e); }
inline QRect  promoted_visualRect(const QModelIndex&  index) const { return QHeaderView::visualRect(index); }
inline QRegion  promoted_visualRegionForSelection(const QItemSelection&  selection) const { return QHeaderView::visualRegionForSelection(selection); }
};

class PythonQtWrapper_QHeaderView : public QObject
{ Q_OBJECT
public:
public slots:
QHeaderView* new_QHeaderView(Qt::Orientation  orientation, QWidget*  parent = 0);
void delete_QHeaderView(QHeaderView* obj) { delete obj; } 
   bool  cascadingSectionResizes(QHeaderView* theWrappedObject) const;
   int  count(QHeaderView* theWrappedObject) const;
   void currentChanged(QHeaderView* theWrappedObject, const QModelIndex&  current, const QModelIndex&  old);
   void dataChanged(QHeaderView* theWrappedObject, const QModelIndex&  topLeft, const QModelIndex&  bottomRight);
   Qt::Alignment  defaultAlignment(QHeaderView* theWrappedObject) const;
   int  defaultSectionSize(QHeaderView* theWrappedObject) const;
   void doItemsLayout(QHeaderView* theWrappedObject);
   bool  event(QHeaderView* theWrappedObject, QEvent*  e);
   int  hiddenSectionCount(QHeaderView* theWrappedObject) const;
   void hideSection(QHeaderView* theWrappedObject, int  logicalIndex);
   bool  highlightSections(QHeaderView* theWrappedObject) const;
   int  horizontalOffset(QHeaderView* theWrappedObject) const;
   QModelIndex  indexAt(QHeaderView* theWrappedObject, const QPoint&  p) const;
   bool  isClickable(QHeaderView* theWrappedObject) const;
   bool  isIndexHidden(QHeaderView* theWrappedObject, const QModelIndex&  index) const;
   bool  isMovable(QHeaderView* theWrappedObject) const;
   bool  isSectionHidden(QHeaderView* theWrappedObject, int  logicalIndex) const;
   bool  isSortIndicatorShown(QHeaderView* theWrappedObject) const;
   int  length(QHeaderView* theWrappedObject) const;
   int  logicalIndex(QHeaderView* theWrappedObject, int  visualIndex) const;
   int  logicalIndexAt(QHeaderView* theWrappedObject, const QPoint&  pos) const;
   int  logicalIndexAt(QHeaderView* theWrappedObject, int  position) const;
   int  logicalIndexAt(QHeaderView* theWrappedObject, int  x, int  y) const;
   int  minimumSectionSize(QHeaderView* theWrappedObject) const;
   void mouseDoubleClickEvent(QHeaderView* theWrappedObject, QMouseEvent*  e);
   void mouseMoveEvent(QHeaderView* theWrappedObject, QMouseEvent*  e);
   void mousePressEvent(QHeaderView* theWrappedObject, QMouseEvent*  e);
   void mouseReleaseEvent(QHeaderView* theWrappedObject, QMouseEvent*  e);
   void moveSection(QHeaderView* theWrappedObject, int  from, int  to);
   int  offset(QHeaderView* theWrappedObject) const;
   Qt::Orientation  orientation(QHeaderView* theWrappedObject) const;
   void paintEvent(QHeaderView* theWrappedObject, QPaintEvent*  e);
   void paintSection(QHeaderView* theWrappedObject, QPainter*  painter, const QRect&  rect, int  logicalIndex) const;
   void reset(QHeaderView* theWrappedObject);
   QHeaderView::ResizeMode  resizeMode(QHeaderView* theWrappedObject, int  logicalIndex) const;
   void resizeSection(QHeaderView* theWrappedObject, int  logicalIndex, int  size);
   void resizeSections(QHeaderView* theWrappedObject, QHeaderView::ResizeMode  mode);
   bool  restoreState(QHeaderView* theWrappedObject, const QByteArray&  state);
   void rowsInserted(QHeaderView* theWrappedObject, const QModelIndex&  parent, int  start, int  end);
   QByteArray  saveState(QHeaderView* theWrappedObject) const;
   void scrollContentsBy(QHeaderView* theWrappedObject, int  dx, int  dy);
   void scrollTo(QHeaderView* theWrappedObject, const QModelIndex&  index, QAbstractItemView::ScrollHint  hint);
   int  sectionPosition(QHeaderView* theWrappedObject, int  logicalIndex) const;
   int  sectionSize(QHeaderView* theWrappedObject, int  logicalIndex) const;
   QSize  sectionSizeFromContents(QHeaderView* theWrappedObject, int  logicalIndex) const;
   int  sectionSizeHint(QHeaderView* theWrappedObject, int  logicalIndex) const;
   int  sectionViewportPosition(QHeaderView* theWrappedObject, int  logicalIndex) const;
   bool  sectionsHidden(QHeaderView* theWrappedObject) const;
   bool  sectionsMoved(QHeaderView* theWrappedObject) const;
   void setCascadingSectionResizes(QHeaderView* theWrappedObject, bool  enable);
   void setClickable(QHeaderView* theWrappedObject, bool  clickable);
   void setDefaultAlignment(QHeaderView* theWrappedObject, Qt::Alignment  alignment);
   void setDefaultSectionSize(QHeaderView* theWrappedObject, int  size);
   void setHighlightSections(QHeaderView* theWrappedObject, bool  highlight);
   void setMinimumSectionSize(QHeaderView* theWrappedObject, int  size);
   void setModel(QHeaderView* theWrappedObject, QAbstractItemModel*  model);
   void setMovable(QHeaderView* theWrappedObject, bool  movable);
   void setResizeMode(QHeaderView* theWrappedObject, QHeaderView::ResizeMode  mode);
   void setResizeMode(QHeaderView* theWrappedObject, int  logicalIndex, QHeaderView::ResizeMode  mode);
   void setSectionHidden(QHeaderView* theWrappedObject, int  logicalIndex, bool  hide);
   void setSelection(QHeaderView* theWrappedObject, const QRect&  rect, QItemSelectionModel::SelectionFlags  flags);
   void setSortIndicator(QHeaderView* theWrappedObject, int  logicalIndex, Qt::SortOrder  order);
   void setSortIndicatorShown(QHeaderView* theWrappedObject, bool  show);
   void setStretchLastSection(QHeaderView* theWrappedObject, bool  stretch);
   void showSection(QHeaderView* theWrappedObject, int  logicalIndex);
   QSize  sizeHint(QHeaderView* theWrappedObject) const;
   Qt::SortOrder  sortIndicatorOrder(QHeaderView* theWrappedObject) const;
   int  sortIndicatorSection(QHeaderView* theWrappedObject) const;
   bool  stretchLastSection(QHeaderView* theWrappedObject) const;
   int  stretchSectionCount(QHeaderView* theWrappedObject) const;
   void swapSections(QHeaderView* theWrappedObject, int  first, int  second);
   void updateGeometries(QHeaderView* theWrappedObject);
   int  verticalOffset(QHeaderView* theWrappedObject) const;
   bool  viewportEvent(QHeaderView* theWrappedObject, QEvent*  e);
   int  visualIndex(QHeaderView* theWrappedObject, int  logicalIndex) const;
   int  visualIndexAt(QHeaderView* theWrappedObject, int  position) const;
   QRect  visualRect(QHeaderView* theWrappedObject, const QModelIndex&  index) const;
   QRegion  visualRegionForSelection(QHeaderView* theWrappedObject, const QItemSelection&  selection) const;
};





class PythonQtWrapper_QHelpEvent : public QObject
{ Q_OBJECT
public:
public slots:
QHelpEvent* new_QHelpEvent(QEvent::Type  type, const QPoint&  pos, const QPoint&  globalPos);
void delete_QHelpEvent(QHelpEvent* obj) { delete obj; } 
   const QPoint*  globalPos(QHelpEvent* theWrappedObject) const;
   int  globalX(QHelpEvent* theWrappedObject) const;
   int  globalY(QHelpEvent* theWrappedObject) const;
   const QPoint*  pos(QHelpEvent* theWrappedObject) const;
   int  x(QHelpEvent* theWrappedObject) const;
   int  y(QHelpEvent* theWrappedObject) const;
};





class PythonQtWrapper_QHideEvent : public QObject
{ Q_OBJECT
public:
public slots:
QHideEvent* new_QHideEvent();
void delete_QHideEvent(QHideEvent* obj) { delete obj; } 
};





class PythonQtShell_QHoverEvent : public QHoverEvent
{
public:
    PythonQtShell_QHoverEvent(QEvent::Type  type, const QPoint&  pos, const QPoint&  oldPos):QHoverEvent(type, pos, oldPos),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QHoverEvent : public QObject
{ Q_OBJECT
public:
public slots:
QHoverEvent* new_QHoverEvent(QEvent::Type  type, const QPoint&  pos, const QPoint&  oldPos);
void delete_QHoverEvent(QHoverEvent* obj) { delete obj; } 
   const QPoint*  oldPos(QHoverEvent* theWrappedObject) const;
   const QPoint*  pos(QHoverEvent* theWrappedObject) const;
};





class PythonQtWrapper_QIconDragEvent : public QObject
{ Q_OBJECT
public:
public slots:
QIconDragEvent* new_QIconDragEvent();
void delete_QIconDragEvent(QIconDragEvent* obj) { delete obj; } 
};





class PythonQtShell_QIconEngine : public QIconEngine
{
public:
    PythonQtShell_QIconEngine():QIconEngine(),_wrapper(NULL) {};

virtual QSize  actualSize(const QSize&  size, QIcon::Mode  mode, QIcon::State  state);
virtual void addFile(const QString&  fileName, const QSize&  size, QIcon::Mode  mode, QIcon::State  state);
virtual void addPixmap(const QPixmap&  pixmap, QIcon::Mode  mode, QIcon::State  state);
virtual void paint(QPainter*  painter, const QRect&  rect, QIcon::Mode  mode, QIcon::State  state);
virtual QPixmap  pixmap(const QSize&  size, QIcon::Mode  mode, QIcon::State  state);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QIconEngine : public QIconEngine
{ public:
inline QSize  promoted_actualSize(const QSize&  size, QIcon::Mode  mode, QIcon::State  state) { return QIconEngine::actualSize(size, mode, state); }
inline void promoted_addFile(const QString&  fileName, const QSize&  size, QIcon::Mode  mode, QIcon::State  state) { QIconEngine::addFile(fileName, size, mode, state); }
inline void promoted_addPixmap(const QPixmap&  pixmap, QIcon::Mode  mode, QIcon::State  state) { QIconEngine::addPixmap(pixmap, mode, state); }
inline QPixmap  promoted_pixmap(const QSize&  size, QIcon::Mode  mode, QIcon::State  state) { return QIconEngine::pixmap(size, mode, state); }
};

class PythonQtWrapper_QIconEngine : public QObject
{ Q_OBJECT
public:
public slots:
QIconEngine* new_QIconEngine();
void delete_QIconEngine(QIconEngine* obj) { delete obj; } 
   QSize  actualSize(QIconEngine* theWrappedObject, const QSize&  size, QIcon::Mode  mode, QIcon::State  state);
   void addFile(QIconEngine* theWrappedObject, const QString&  fileName, const QSize&  size, QIcon::Mode  mode, QIcon::State  state);
   void addPixmap(QIconEngine* theWrappedObject, const QPixmap&  pixmap, QIcon::Mode  mode, QIcon::State  state);
   QPixmap  pixmap(QIconEngine* theWrappedObject, const QSize&  size, QIcon::Mode  mode, QIcon::State  state);
};


