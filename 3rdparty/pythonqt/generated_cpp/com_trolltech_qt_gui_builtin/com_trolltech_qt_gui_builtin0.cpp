#include "com_trolltech_qt_gui_builtin0.h"
#include <PythonQtConversion.h>
#include <PythonQtMethodInfo.h>
#include <PythonQtSignalReceiver.h>
#include <QBrush>
#include <QMatrix>
#include <QPainterPath>
#include <QPixmap>
#include <QStringList>
#include <QVariant>
#include <qbitmap.h>
#include <qbrush.h>
#include <qbytearray.h>
#include <qcolor.h>
#include <qcursor.h>
#include <qdatastream.h>
#include <qfont.h>
#include <qicon.h>
#include <qiconengine.h>
#include <qimage.h>
#include <qimagewriter.h>
#include <qiodevice.h>
#include <qkeysequence.h>
#include <qline.h>
#include <qlist.h>
#include <qmatrix.h>
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
#include <qsize.h>
#include <qsizepolicy.h>
#include <qstringlist.h>
#include <qtextformat.h>
#include <qtransform.h>
#include <qvector.h>
#include <qwidget.h>

int  PythonQtShell_QBitmap::devType() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "devType");
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
          PythonQt::priv()->handleVirtualOverloadReturnError("devType", methodInfo, result);
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
  return QBitmap::devType();
}
int  PythonQtShell_QBitmap::metric(QPaintDevice::PaintDeviceMetric  arg__1) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "metric");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"int" , "QPaintDevice::PaintDeviceMetric"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      int returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("metric", methodInfo, result);
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
  return QBitmap::metric(arg__1);
}
QPaintEngine*  PythonQtShell_QBitmap::paintEngine() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "paintEngine");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QPaintEngine*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QPaintEngine* returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("paintEngine", methodInfo, result);
        } else {
          returnValue = *((QPaintEngine**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QBitmap::paintEngine();
}
QBitmap* PythonQtWrapper_QBitmap::new_QBitmap()
{ 
return new PythonQtShell_QBitmap(); }

QBitmap* PythonQtWrapper_QBitmap::new_QBitmap(const QPixmap&  arg__1)
{ 
return new PythonQtShell_QBitmap(arg__1); }

QBitmap* PythonQtWrapper_QBitmap::new_QBitmap(const QSize&  arg__1)
{ 
return new PythonQtShell_QBitmap(arg__1); }

QBitmap* PythonQtWrapper_QBitmap::new_QBitmap(const QString&  fileName, const char*  format)
{ 
return new PythonQtShell_QBitmap(fileName, format); }

QBitmap* PythonQtWrapper_QBitmap::new_QBitmap(int  w, int  h)
{ 
return new PythonQtShell_QBitmap(w, h); }

void PythonQtWrapper_QBitmap::clear(QBitmap* theWrappedObject)
{
  ( theWrappedObject->clear());
}

QBitmap  PythonQtWrapper_QBitmap::static_QBitmap_fromImage(const QImage&  image, Qt::ImageConversionFlags  flags)
{
  return (QBitmap::fromImage(image, flags));
}

QBitmap  PythonQtWrapper_QBitmap::transformed(QBitmap* theWrappedObject, const QMatrix&  arg__1) const
{
  return ( theWrappedObject->transformed(arg__1));
}

QBitmap  PythonQtWrapper_QBitmap::transformed(QBitmap* theWrappedObject, const QTransform&  matrix) const
{
  return ( theWrappedObject->transformed(matrix));
}



QBrush* PythonQtWrapper_QBrush::new_QBrush()
{ 
return new QBrush(); }

QBrush* PythonQtWrapper_QBrush::new_QBrush(Qt::BrushStyle  bs)
{ 
return new QBrush(bs); }

QBrush* PythonQtWrapper_QBrush::new_QBrush(Qt::GlobalColor  color, const QPixmap&  pixmap)
{ 
return new QBrush(color, pixmap); }

QBrush* PythonQtWrapper_QBrush::new_QBrush(const QBrush&  brush)
{ 
return new QBrush(brush); }

QBrush* PythonQtWrapper_QBrush::new_QBrush(const QColor&  color, Qt::BrushStyle  bs)
{ 
return new QBrush(color, bs); }

QBrush* PythonQtWrapper_QBrush::new_QBrush(const QColor&  color, const QPixmap&  pixmap)
{ 
return new QBrush(color, pixmap); }

QBrush* PythonQtWrapper_QBrush::new_QBrush(const QGradient&  gradient)
{ 
return new QBrush(gradient); }

QBrush* PythonQtWrapper_QBrush::new_QBrush(const QImage&  image)
{ 
return new QBrush(image); }

QBrush* PythonQtWrapper_QBrush::new_QBrush(const QPixmap&  pixmap)
{ 
return new QBrush(pixmap); }

const QColor*  PythonQtWrapper_QBrush::color(QBrush* theWrappedObject) const
{
  return &( theWrappedObject->color());
}

const QGradient*  PythonQtWrapper_QBrush::gradient(QBrush* theWrappedObject) const
{
  return ( theWrappedObject->gradient());
}

bool  PythonQtWrapper_QBrush::isOpaque(QBrush* theWrappedObject) const
{
  return ( theWrappedObject->isOpaque());
}

const QMatrix*  PythonQtWrapper_QBrush::matrix(QBrush* theWrappedObject) const
{
  return &( theWrappedObject->matrix());
}

bool  PythonQtWrapper_QBrush::__ne__(QBrush* theWrappedObject, const QBrush&  b) const
{
  return ( (*theWrappedObject)!= b);
}

void PythonQtWrapper_QBrush::writeTo(QBrush* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 <<  (*theWrappedObject);
}

bool  PythonQtWrapper_QBrush::__eq__(QBrush* theWrappedObject, const QBrush&  b) const
{
  return ( (*theWrappedObject)== b);
}

void PythonQtWrapper_QBrush::readFrom(QBrush* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 >>  (*theWrappedObject);
}

void PythonQtWrapper_QBrush::setColor(QBrush* theWrappedObject, Qt::GlobalColor  color)
{
  ( theWrappedObject->setColor(color));
}

void PythonQtWrapper_QBrush::setColor(QBrush* theWrappedObject, const QColor&  color)
{
  ( theWrappedObject->setColor(color));
}

void PythonQtWrapper_QBrush::setMatrix(QBrush* theWrappedObject, const QMatrix&  mat)
{
  ( theWrappedObject->setMatrix(mat));
}

void PythonQtWrapper_QBrush::setStyle(QBrush* theWrappedObject, Qt::BrushStyle  arg__1)
{
  ( theWrappedObject->setStyle(arg__1));
}

void PythonQtWrapper_QBrush::setTexture(QBrush* theWrappedObject, const QPixmap&  pixmap)
{
  ( theWrappedObject->setTexture(pixmap));
}

void PythonQtWrapper_QBrush::setTextureImage(QBrush* theWrappedObject, const QImage&  image)
{
  ( theWrappedObject->setTextureImage(image));
}

void PythonQtWrapper_QBrush::setTransform(QBrush* theWrappedObject, const QTransform&  arg__1)
{
  ( theWrappedObject->setTransform(arg__1));
}

Qt::BrushStyle  PythonQtWrapper_QBrush::style(QBrush* theWrappedObject) const
{
  return ( theWrappedObject->style());
}

QPixmap  PythonQtWrapper_QBrush::texture(QBrush* theWrappedObject) const
{
  return ( theWrappedObject->texture());
}

QImage  PythonQtWrapper_QBrush::textureImage(QBrush* theWrappedObject) const
{
  return ( theWrappedObject->textureImage());
}

QTransform  PythonQtWrapper_QBrush::transform(QBrush* theWrappedObject) const
{
  return ( theWrappedObject->transform());
}

QString PythonQtWrapper_QBrush::py_toString(QBrush* obj) {
  QString result;
  QDebug d(&result);
  d << *obj;
  return result;
}



QColor* PythonQtWrapper_QColor::new_QColor()
{ 
return new QColor(); }

QColor* PythonQtWrapper_QColor::new_QColor(Qt::GlobalColor  color)
{ 
return new QColor(color); }

QColor* PythonQtWrapper_QColor::new_QColor(const QColor&  color)
{ 
return new QColor(color); }

QColor* PythonQtWrapper_QColor::new_QColor(const QString&  name)
{ 
return new QColor(name); }

QColor* PythonQtWrapper_QColor::new_QColor(int  r, int  g, int  b, int  a)
{ 
return new QColor(r, g, b, a); }

QColor* PythonQtWrapper_QColor::new_QColor(unsigned int  rgb)
{ 
return new QColor(rgb); }

int  PythonQtWrapper_QColor::alpha(QColor* theWrappedObject) const
{
  return ( theWrappedObject->alpha());
}

qreal  PythonQtWrapper_QColor::alphaF(QColor* theWrappedObject) const
{
  return ( theWrappedObject->alphaF());
}

int  PythonQtWrapper_QColor::black(QColor* theWrappedObject) const
{
  return ( theWrappedObject->black());
}

qreal  PythonQtWrapper_QColor::blackF(QColor* theWrappedObject) const
{
  return ( theWrappedObject->blackF());
}

int  PythonQtWrapper_QColor::blue(QColor* theWrappedObject) const
{
  return ( theWrappedObject->blue());
}

qreal  PythonQtWrapper_QColor::blueF(QColor* theWrappedObject) const
{
  return ( theWrappedObject->blueF());
}

QStringList  PythonQtWrapper_QColor::static_QColor_colorNames()
{
  return (QColor::colorNames());
}

QColor  PythonQtWrapper_QColor::convertTo(QColor* theWrappedObject, QColor::Spec  colorSpec) const
{
  return ( theWrappedObject->convertTo(colorSpec));
}

int  PythonQtWrapper_QColor::cyan(QColor* theWrappedObject) const
{
  return ( theWrappedObject->cyan());
}

qreal  PythonQtWrapper_QColor::cyanF(QColor* theWrappedObject) const
{
  return ( theWrappedObject->cyanF());
}

QColor  PythonQtWrapper_QColor::darker(QColor* theWrappedObject, int  f) const
{
  return ( theWrappedObject->darker(f));
}

QColor  PythonQtWrapper_QColor::static_QColor_fromCmyk(int  c, int  m, int  y, int  k, int  a)
{
  return (QColor::fromCmyk(c, m, y, k, a));
}

QColor  PythonQtWrapper_QColor::static_QColor_fromCmykF(qreal  c, qreal  m, qreal  y, qreal  k, qreal  a)
{
  return (QColor::fromCmykF(c, m, y, k, a));
}

QColor  PythonQtWrapper_QColor::static_QColor_fromHsl(int  h, int  s, int  l, int  a)
{
  return (QColor::fromHsl(h, s, l, a));
}

QColor  PythonQtWrapper_QColor::static_QColor_fromHslF(qreal  h, qreal  s, qreal  l, qreal  a)
{
  return (QColor::fromHslF(h, s, l, a));
}

QColor  PythonQtWrapper_QColor::static_QColor_fromHsv(int  h, int  s, int  v, int  a)
{
  return (QColor::fromHsv(h, s, v, a));
}

QColor  PythonQtWrapper_QColor::static_QColor_fromHsvF(qreal  h, qreal  s, qreal  v, qreal  a)
{
  return (QColor::fromHsvF(h, s, v, a));
}

QColor  PythonQtWrapper_QColor::static_QColor_fromRgb(int  r, int  g, int  b, int  a)
{
  return (QColor::fromRgb(r, g, b, a));
}

QColor  PythonQtWrapper_QColor::static_QColor_fromRgb(unsigned int  rgb)
{
  return (QColor::fromRgb(rgb));
}

QColor  PythonQtWrapper_QColor::static_QColor_fromRgbF(qreal  r, qreal  g, qreal  b, qreal  a)
{
  return (QColor::fromRgbF(r, g, b, a));
}

QColor  PythonQtWrapper_QColor::static_QColor_fromRgba(unsigned int  rgba)
{
  return (QColor::fromRgba(rgba));
}

void PythonQtWrapper_QColor::getCmykF(QColor* theWrappedObject, qreal*  c, qreal*  m, qreal*  y, qreal*  k, qreal*  a)
{
  ( theWrappedObject->getCmykF(c, m, y, k, a));
}

void PythonQtWrapper_QColor::getHsl(QColor* theWrappedObject, int*  h, int*  s, int*  l, int*  a) const
{
  ( theWrappedObject->getHsl(h, s, l, a));
}

void PythonQtWrapper_QColor::getHslF(QColor* theWrappedObject, qreal*  h, qreal*  s, qreal*  l, qreal*  a) const
{
  ( theWrappedObject->getHslF(h, s, l, a));
}

void PythonQtWrapper_QColor::getHsvF(QColor* theWrappedObject, qreal*  h, qreal*  s, qreal*  v, qreal*  a) const
{
  ( theWrappedObject->getHsvF(h, s, v, a));
}

void PythonQtWrapper_QColor::getRgbF(QColor* theWrappedObject, qreal*  r, qreal*  g, qreal*  b, qreal*  a) const
{
  ( theWrappedObject->getRgbF(r, g, b, a));
}

int  PythonQtWrapper_QColor::green(QColor* theWrappedObject) const
{
  return ( theWrappedObject->green());
}

qreal  PythonQtWrapper_QColor::greenF(QColor* theWrappedObject) const
{
  return ( theWrappedObject->greenF());
}

int  PythonQtWrapper_QColor::hslHue(QColor* theWrappedObject) const
{
  return ( theWrappedObject->hslHue());
}

qreal  PythonQtWrapper_QColor::hslHueF(QColor* theWrappedObject) const
{
  return ( theWrappedObject->hslHueF());
}

int  PythonQtWrapper_QColor::hslSaturation(QColor* theWrappedObject) const
{
  return ( theWrappedObject->hslSaturation());
}

qreal  PythonQtWrapper_QColor::hslSaturationF(QColor* theWrappedObject) const
{
  return ( theWrappedObject->hslSaturationF());
}

int  PythonQtWrapper_QColor::hsvHue(QColor* theWrappedObject) const
{
  return ( theWrappedObject->hsvHue());
}

qreal  PythonQtWrapper_QColor::hsvHueF(QColor* theWrappedObject) const
{
  return ( theWrappedObject->hsvHueF());
}

int  PythonQtWrapper_QColor::hsvSaturation(QColor* theWrappedObject) const
{
  return ( theWrappedObject->hsvSaturation());
}

qreal  PythonQtWrapper_QColor::hsvSaturationF(QColor* theWrappedObject) const
{
  return ( theWrappedObject->hsvSaturationF());
}

int  PythonQtWrapper_QColor::hue(QColor* theWrappedObject) const
{
  return ( theWrappedObject->hue());
}

qreal  PythonQtWrapper_QColor::hueF(QColor* theWrappedObject) const
{
  return ( theWrappedObject->hueF());
}

bool  PythonQtWrapper_QColor::isValid(QColor* theWrappedObject) const
{
  return ( theWrappedObject->isValid());
}

QColor  PythonQtWrapper_QColor::lighter(QColor* theWrappedObject, int  f) const
{
  return ( theWrappedObject->lighter(f));
}

int  PythonQtWrapper_QColor::lightness(QColor* theWrappedObject) const
{
  return ( theWrappedObject->lightness());
}

qreal  PythonQtWrapper_QColor::lightnessF(QColor* theWrappedObject) const
{
  return ( theWrappedObject->lightnessF());
}

int  PythonQtWrapper_QColor::magenta(QColor* theWrappedObject) const
{
  return ( theWrappedObject->magenta());
}

qreal  PythonQtWrapper_QColor::magentaF(QColor* theWrappedObject) const
{
  return ( theWrappedObject->magentaF());
}

QString  PythonQtWrapper_QColor::name(QColor* theWrappedObject) const
{
  return ( theWrappedObject->name());
}

bool  PythonQtWrapper_QColor::__ne__(QColor* theWrappedObject, const QColor&  c) const
{
  return ( (*theWrappedObject)!= c);
}

void PythonQtWrapper_QColor::writeTo(QColor* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 <<  (*theWrappedObject);
}

bool  PythonQtWrapper_QColor::__eq__(QColor* theWrappedObject, const QColor&  c) const
{
  return ( (*theWrappedObject)== c);
}

void PythonQtWrapper_QColor::readFrom(QColor* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 >>  (*theWrappedObject);
}

int  PythonQtWrapper_QColor::red(QColor* theWrappedObject) const
{
  return ( theWrappedObject->red());
}

qreal  PythonQtWrapper_QColor::redF(QColor* theWrappedObject) const
{
  return ( theWrappedObject->redF());
}

unsigned int  PythonQtWrapper_QColor::rgb(QColor* theWrappedObject) const
{
  return ( theWrappedObject->rgb());
}

unsigned int  PythonQtWrapper_QColor::rgba(QColor* theWrappedObject) const
{
  return ( theWrappedObject->rgba());
}

int  PythonQtWrapper_QColor::saturation(QColor* theWrappedObject) const
{
  return ( theWrappedObject->saturation());
}

qreal  PythonQtWrapper_QColor::saturationF(QColor* theWrappedObject) const
{
  return ( theWrappedObject->saturationF());
}

void PythonQtWrapper_QColor::setAlpha(QColor* theWrappedObject, int  alpha)
{
  ( theWrappedObject->setAlpha(alpha));
}

void PythonQtWrapper_QColor::setAlphaF(QColor* theWrappedObject, qreal  alpha)
{
  ( theWrappedObject->setAlphaF(alpha));
}

void PythonQtWrapper_QColor::setBlue(QColor* theWrappedObject, int  blue)
{
  ( theWrappedObject->setBlue(blue));
}

void PythonQtWrapper_QColor::setBlueF(QColor* theWrappedObject, qreal  blue)
{
  ( theWrappedObject->setBlueF(blue));
}

void PythonQtWrapper_QColor::setCmyk(QColor* theWrappedObject, int  c, int  m, int  y, int  k, int  a)
{
  ( theWrappedObject->setCmyk(c, m, y, k, a));
}

void PythonQtWrapper_QColor::setCmykF(QColor* theWrappedObject, qreal  c, qreal  m, qreal  y, qreal  k, qreal  a)
{
  ( theWrappedObject->setCmykF(c, m, y, k, a));
}

void PythonQtWrapper_QColor::setGreen(QColor* theWrappedObject, int  green)
{
  ( theWrappedObject->setGreen(green));
}

void PythonQtWrapper_QColor::setGreenF(QColor* theWrappedObject, qreal  green)
{
  ( theWrappedObject->setGreenF(green));
}

void PythonQtWrapper_QColor::setHsl(QColor* theWrappedObject, int  h, int  s, int  l, int  a)
{
  ( theWrappedObject->setHsl(h, s, l, a));
}

void PythonQtWrapper_QColor::setHslF(QColor* theWrappedObject, qreal  h, qreal  s, qreal  l, qreal  a)
{
  ( theWrappedObject->setHslF(h, s, l, a));
}

void PythonQtWrapper_QColor::setHsv(QColor* theWrappedObject, int  h, int  s, int  v, int  a)
{
  ( theWrappedObject->setHsv(h, s, v, a));
}

void PythonQtWrapper_QColor::setHsvF(QColor* theWrappedObject, qreal  h, qreal  s, qreal  v, qreal  a)
{
  ( theWrappedObject->setHsvF(h, s, v, a));
}

void PythonQtWrapper_QColor::setNamedColor(QColor* theWrappedObject, const QString&  name)
{
  ( theWrappedObject->setNamedColor(name));
}

void PythonQtWrapper_QColor::setRed(QColor* theWrappedObject, int  red)
{
  ( theWrappedObject->setRed(red));
}

void PythonQtWrapper_QColor::setRedF(QColor* theWrappedObject, qreal  red)
{
  ( theWrappedObject->setRedF(red));
}

void PythonQtWrapper_QColor::setRgb(QColor* theWrappedObject, int  r, int  g, int  b, int  a)
{
  ( theWrappedObject->setRgb(r, g, b, a));
}

void PythonQtWrapper_QColor::setRgb(QColor* theWrappedObject, unsigned int  rgb)
{
  ( theWrappedObject->setRgb(rgb));
}

void PythonQtWrapper_QColor::setRgbF(QColor* theWrappedObject, qreal  r, qreal  g, qreal  b, qreal  a)
{
  ( theWrappedObject->setRgbF(r, g, b, a));
}

void PythonQtWrapper_QColor::setRgba(QColor* theWrappedObject, unsigned int  rgba)
{
  ( theWrappedObject->setRgba(rgba));
}

QColor::Spec  PythonQtWrapper_QColor::spec(QColor* theWrappedObject) const
{
  return ( theWrappedObject->spec());
}

QColor  PythonQtWrapper_QColor::toCmyk(QColor* theWrappedObject) const
{
  return ( theWrappedObject->toCmyk());
}

QColor  PythonQtWrapper_QColor::toHsl(QColor* theWrappedObject) const
{
  return ( theWrappedObject->toHsl());
}

QColor  PythonQtWrapper_QColor::toHsv(QColor* theWrappedObject) const
{
  return ( theWrappedObject->toHsv());
}

QColor  PythonQtWrapper_QColor::toRgb(QColor* theWrappedObject) const
{
  return ( theWrappedObject->toRgb());
}

int  PythonQtWrapper_QColor::value(QColor* theWrappedObject) const
{
  return ( theWrappedObject->value());
}

qreal  PythonQtWrapper_QColor::valueF(QColor* theWrappedObject) const
{
  return ( theWrappedObject->valueF());
}

int  PythonQtWrapper_QColor::yellow(QColor* theWrappedObject) const
{
  return ( theWrappedObject->yellow());
}

qreal  PythonQtWrapper_QColor::yellowF(QColor* theWrappedObject) const
{
  return ( theWrappedObject->yellowF());
}

QString PythonQtWrapper_QColor::py_toString(QColor* obj) {
  QString result;
  QDebug d(&result);
  d << *obj;
  return result;
}



QCursor* PythonQtWrapper_QCursor::new_QCursor()
{ 
return new QCursor(); }

QCursor* PythonQtWrapper_QCursor::new_QCursor(Qt::CursorShape  shape)
{ 
return new QCursor(shape); }

QCursor* PythonQtWrapper_QCursor::new_QCursor(const QBitmap&  bitmap, const QBitmap&  mask, int  hotX, int  hotY)
{ 
return new QCursor(bitmap, mask, hotX, hotY); }

QCursor* PythonQtWrapper_QCursor::new_QCursor(const QCursor&  cursor)
{ 
return new QCursor(cursor); }

QCursor* PythonQtWrapper_QCursor::new_QCursor(const QPixmap&  pixmap, int  hotX, int  hotY)
{ 
return new QCursor(pixmap, hotX, hotY); }

const QBitmap*  PythonQtWrapper_QCursor::bitmap(QCursor* theWrappedObject) const
{
  return ( theWrappedObject->bitmap());
}

QPoint  PythonQtWrapper_QCursor::hotSpot(QCursor* theWrappedObject) const
{
  return ( theWrappedObject->hotSpot());
}

const QBitmap*  PythonQtWrapper_QCursor::mask(QCursor* theWrappedObject) const
{
  return ( theWrappedObject->mask());
}

void PythonQtWrapper_QCursor::writeTo(QCursor* theWrappedObject, QDataStream&  outS)
{
  outS <<  (*theWrappedObject);
}

void PythonQtWrapper_QCursor::readFrom(QCursor* theWrappedObject, QDataStream&  inS)
{
  inS >>  (*theWrappedObject);
}

QPixmap  PythonQtWrapper_QCursor::pixmap(QCursor* theWrappedObject) const
{
  return ( theWrappedObject->pixmap());
}

QPoint  PythonQtWrapper_QCursor::static_QCursor_pos()
{
  return (QCursor::pos());
}

void PythonQtWrapper_QCursor::static_QCursor_setPos(const QPoint&  p)
{
  (QCursor::setPos(p));
}

void PythonQtWrapper_QCursor::static_QCursor_setPos(int  x, int  y)
{
  (QCursor::setPos(x, y));
}

void PythonQtWrapper_QCursor::setShape(QCursor* theWrappedObject, Qt::CursorShape  newShape)
{
  ( theWrappedObject->setShape(newShape));
}

Qt::CursorShape  PythonQtWrapper_QCursor::shape(QCursor* theWrappedObject) const
{
  return ( theWrappedObject->shape());
}



QFont* PythonQtWrapper_QFont::new_QFont()
{ 
return new QFont(); }

QFont* PythonQtWrapper_QFont::new_QFont(const QFont&  arg__1)
{ 
return new QFont(arg__1); }

QFont* PythonQtWrapper_QFont::new_QFont(const QFont&  arg__1, QPaintDevice*  pd)
{ 
return new QFont(arg__1, pd); }

QFont* PythonQtWrapper_QFont::new_QFont(const QString&  family, int  pointSize, int  weight, bool  italic)
{ 
return new QFont(family, pointSize, weight, italic); }

bool  PythonQtWrapper_QFont::bold(QFont* theWrappedObject) const
{
  return ( theWrappedObject->bold());
}

void PythonQtWrapper_QFont::static_QFont_cacheStatistics()
{
  (QFont::cacheStatistics());
}

QFont::Capitalization  PythonQtWrapper_QFont::capitalization(QFont* theWrappedObject) const
{
  return ( theWrappedObject->capitalization());
}

void PythonQtWrapper_QFont::static_QFont_cleanup()
{
  (QFont::cleanup());
}

QString  PythonQtWrapper_QFont::defaultFamily(QFont* theWrappedObject) const
{
  return ( theWrappedObject->defaultFamily());
}

bool  PythonQtWrapper_QFont::exactMatch(QFont* theWrappedObject) const
{
  return ( theWrappedObject->exactMatch());
}

QString  PythonQtWrapper_QFont::family(QFont* theWrappedObject) const
{
  return ( theWrappedObject->family());
}

bool  PythonQtWrapper_QFont::fixedPitch(QFont* theWrappedObject) const
{
  return ( theWrappedObject->fixedPitch());
}

bool  PythonQtWrapper_QFont::fromString(QFont* theWrappedObject, const QString&  arg__1)
{
  return ( theWrappedObject->fromString(arg__1));
}

Qt::HANDLE  PythonQtWrapper_QFont::handle(QFont* theWrappedObject) const
{
  return ( theWrappedObject->handle());
}

void PythonQtWrapper_QFont::static_QFont_initialize()
{
  (QFont::initialize());
}

void PythonQtWrapper_QFont::static_QFont_insertSubstitution(const QString&  arg__1, const QString&  arg__2)
{
  (QFont::insertSubstitution(arg__1, arg__2));
}

void PythonQtWrapper_QFont::static_QFont_insertSubstitutions(const QString&  arg__1, const QStringList&  arg__2)
{
  (QFont::insertSubstitutions(arg__1, arg__2));
}

bool  PythonQtWrapper_QFont::isCopyOf(QFont* theWrappedObject, const QFont&  arg__1) const
{
  return ( theWrappedObject->isCopyOf(arg__1));
}

bool  PythonQtWrapper_QFont::italic(QFont* theWrappedObject) const
{
  return ( theWrappedObject->italic());
}

bool  PythonQtWrapper_QFont::kerning(QFont* theWrappedObject) const
{
  return ( theWrappedObject->kerning());
}

QString  PythonQtWrapper_QFont::key(QFont* theWrappedObject) const
{
  return ( theWrappedObject->key());
}

QString  PythonQtWrapper_QFont::lastResortFamily(QFont* theWrappedObject) const
{
  return ( theWrappedObject->lastResortFamily());
}

QString  PythonQtWrapper_QFont::lastResortFont(QFont* theWrappedObject) const
{
  return ( theWrappedObject->lastResortFont());
}

qreal  PythonQtWrapper_QFont::letterSpacing(QFont* theWrappedObject) const
{
  return ( theWrappedObject->letterSpacing());
}

QFont::SpacingType  PythonQtWrapper_QFont::letterSpacingType(QFont* theWrappedObject) const
{
  return ( theWrappedObject->letterSpacingType());
}

bool  PythonQtWrapper_QFont::__ne__(QFont* theWrappedObject, const QFont&  arg__1) const
{
  return ( (*theWrappedObject)!= arg__1);
}

bool  PythonQtWrapper_QFont::__lt__(QFont* theWrappedObject, const QFont&  arg__1) const
{
  return ( (*theWrappedObject)< arg__1);
}

void PythonQtWrapper_QFont::writeTo(QFont* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 <<  (*theWrappedObject);
}

bool  PythonQtWrapper_QFont::__eq__(QFont* theWrappedObject, const QFont&  arg__1) const
{
  return ( (*theWrappedObject)== arg__1);
}

void PythonQtWrapper_QFont::readFrom(QFont* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 >>  (*theWrappedObject);
}

bool  PythonQtWrapper_QFont::overline(QFont* theWrappedObject) const
{
  return ( theWrappedObject->overline());
}

int  PythonQtWrapper_QFont::pixelSize(QFont* theWrappedObject) const
{
  return ( theWrappedObject->pixelSize());
}

int  PythonQtWrapper_QFont::pointSize(QFont* theWrappedObject) const
{
  return ( theWrappedObject->pointSize());
}

qreal  PythonQtWrapper_QFont::pointSizeF(QFont* theWrappedObject) const
{
  return ( theWrappedObject->pointSizeF());
}

bool  PythonQtWrapper_QFont::rawMode(QFont* theWrappedObject) const
{
  return ( theWrappedObject->rawMode());
}

QString  PythonQtWrapper_QFont::rawName(QFont* theWrappedObject) const
{
  return ( theWrappedObject->rawName());
}

void PythonQtWrapper_QFont::static_QFont_removeSubstitution(const QString&  arg__1)
{
  (QFont::removeSubstitution(arg__1));
}

uint  PythonQtWrapper_QFont::resolve(QFont* theWrappedObject) const
{
  return ( theWrappedObject->resolve());
}

QFont  PythonQtWrapper_QFont::resolve(QFont* theWrappedObject, const QFont&  arg__1) const
{
  return ( theWrappedObject->resolve(arg__1));
}

void PythonQtWrapper_QFont::resolve(QFont* theWrappedObject, uint  mask)
{
  ( theWrappedObject->resolve(mask));
}

void PythonQtWrapper_QFont::setBold(QFont* theWrappedObject, bool  arg__1)
{
  ( theWrappedObject->setBold(arg__1));
}

void PythonQtWrapper_QFont::setCapitalization(QFont* theWrappedObject, QFont::Capitalization  arg__1)
{
  ( theWrappedObject->setCapitalization(arg__1));
}

void PythonQtWrapper_QFont::setFamily(QFont* theWrappedObject, const QString&  arg__1)
{
  ( theWrappedObject->setFamily(arg__1));
}

void PythonQtWrapper_QFont::setFixedPitch(QFont* theWrappedObject, bool  arg__1)
{
  ( theWrappedObject->setFixedPitch(arg__1));
}

void PythonQtWrapper_QFont::setItalic(QFont* theWrappedObject, bool  b)
{
  ( theWrappedObject->setItalic(b));
}

void PythonQtWrapper_QFont::setKerning(QFont* theWrappedObject, bool  arg__1)
{
  ( theWrappedObject->setKerning(arg__1));
}

void PythonQtWrapper_QFont::setLetterSpacing(QFont* theWrappedObject, QFont::SpacingType  type, qreal  spacing)
{
  ( theWrappedObject->setLetterSpacing(type, spacing));
}

void PythonQtWrapper_QFont::setOverline(QFont* theWrappedObject, bool  arg__1)
{
  ( theWrappedObject->setOverline(arg__1));
}

void PythonQtWrapper_QFont::setPixelSize(QFont* theWrappedObject, int  arg__1)
{
  ( theWrappedObject->setPixelSize(arg__1));
}

void PythonQtWrapper_QFont::setPointSize(QFont* theWrappedObject, int  arg__1)
{
  ( theWrappedObject->setPointSize(arg__1));
}

void PythonQtWrapper_QFont::setPointSizeF(QFont* theWrappedObject, qreal  arg__1)
{
  ( theWrappedObject->setPointSizeF(arg__1));
}

void PythonQtWrapper_QFont::setRawMode(QFont* theWrappedObject, bool  arg__1)
{
  ( theWrappedObject->setRawMode(arg__1));
}

void PythonQtWrapper_QFont::setRawName(QFont* theWrappedObject, const QString&  arg__1)
{
  ( theWrappedObject->setRawName(arg__1));
}

void PythonQtWrapper_QFont::setStretch(QFont* theWrappedObject, int  arg__1)
{
  ( theWrappedObject->setStretch(arg__1));
}

void PythonQtWrapper_QFont::setStrikeOut(QFont* theWrappedObject, bool  arg__1)
{
  ( theWrappedObject->setStrikeOut(arg__1));
}

void PythonQtWrapper_QFont::setStyle(QFont* theWrappedObject, QFont::Style  style)
{
  ( theWrappedObject->setStyle(style));
}

void PythonQtWrapper_QFont::setStyleHint(QFont* theWrappedObject, QFont::StyleHint  arg__1, QFont::StyleStrategy  arg__2)
{
  ( theWrappedObject->setStyleHint(arg__1, arg__2));
}

void PythonQtWrapper_QFont::setStyleStrategy(QFont* theWrappedObject, QFont::StyleStrategy  s)
{
  ( theWrappedObject->setStyleStrategy(s));
}

void PythonQtWrapper_QFont::setUnderline(QFont* theWrappedObject, bool  arg__1)
{
  ( theWrappedObject->setUnderline(arg__1));
}

void PythonQtWrapper_QFont::setWeight(QFont* theWrappedObject, int  arg__1)
{
  ( theWrappedObject->setWeight(arg__1));
}

void PythonQtWrapper_QFont::setWordSpacing(QFont* theWrappedObject, qreal  spacing)
{
  ( theWrappedObject->setWordSpacing(spacing));
}

int  PythonQtWrapper_QFont::stretch(QFont* theWrappedObject) const
{
  return ( theWrappedObject->stretch());
}

bool  PythonQtWrapper_QFont::strikeOut(QFont* theWrappedObject) const
{
  return ( theWrappedObject->strikeOut());
}

QFont::Style  PythonQtWrapper_QFont::style(QFont* theWrappedObject) const
{
  return ( theWrappedObject->style());
}

QFont::StyleHint  PythonQtWrapper_QFont::styleHint(QFont* theWrappedObject) const
{
  return ( theWrappedObject->styleHint());
}

QFont::StyleStrategy  PythonQtWrapper_QFont::styleStrategy(QFont* theWrappedObject) const
{
  return ( theWrappedObject->styleStrategy());
}

QString  PythonQtWrapper_QFont::static_QFont_substitute(const QString&  arg__1)
{
  return (QFont::substitute(arg__1));
}

QStringList  PythonQtWrapper_QFont::static_QFont_substitutes(const QString&  arg__1)
{
  return (QFont::substitutes(arg__1));
}

QStringList  PythonQtWrapper_QFont::static_QFont_substitutions()
{
  return (QFont::substitutions());
}

QString  PythonQtWrapper_QFont::toString(QFont* theWrappedObject) const
{
  return ( theWrappedObject->toString());
}

bool  PythonQtWrapper_QFont::underline(QFont* theWrappedObject) const
{
  return ( theWrappedObject->underline());
}

int  PythonQtWrapper_QFont::weight(QFont* theWrappedObject) const
{
  return ( theWrappedObject->weight());
}

qreal  PythonQtWrapper_QFont::wordSpacing(QFont* theWrappedObject) const
{
  return ( theWrappedObject->wordSpacing());
}

QString PythonQtWrapper_QFont::py_toString(QFont* obj) { return obj->toString(); }


QIcon* PythonQtWrapper_QIcon::new_QIcon()
{ 
return new QIcon(); }

QIcon* PythonQtWrapper_QIcon::new_QIcon(QIconEngine*  engine)
{ 
return new QIcon(engine); }

QIcon* PythonQtWrapper_QIcon::new_QIcon(QIconEngineV2*  engine)
{ 
return new QIcon(engine); }

QIcon* PythonQtWrapper_QIcon::new_QIcon(const QIcon&  other)
{ 
return new QIcon(other); }

QIcon* PythonQtWrapper_QIcon::new_QIcon(const QPixmap&  pixmap)
{ 
return new QIcon(pixmap); }

QIcon* PythonQtWrapper_QIcon::new_QIcon(const QString&  fileName)
{ 
return new QIcon(fileName); }

QSize  PythonQtWrapper_QIcon::actualSize(QIcon* theWrappedObject, const QSize&  size, QIcon::Mode  mode, QIcon::State  state) const
{
  return ( theWrappedObject->actualSize(size, mode, state));
}

void PythonQtWrapper_QIcon::addFile(QIcon* theWrappedObject, const QString&  fileName, const QSize&  size, QIcon::Mode  mode, QIcon::State  state)
{
  ( theWrappedObject->addFile(fileName, size, mode, state));
}

void PythonQtWrapper_QIcon::addPixmap(QIcon* theWrappedObject, const QPixmap&  pixmap, QIcon::Mode  mode, QIcon::State  state)
{
  ( theWrappedObject->addPixmap(pixmap, mode, state));
}

QList<QSize >  PythonQtWrapper_QIcon::availableSizes(QIcon* theWrappedObject, QIcon::Mode  mode, QIcon::State  state) const
{
  return ( theWrappedObject->availableSizes(mode, state));
}

qint64  PythonQtWrapper_QIcon::cacheKey(QIcon* theWrappedObject) const
{
  return ( theWrappedObject->cacheKey());
}

QIcon  PythonQtWrapper_QIcon::static_QIcon_fromTheme(const QString&  name, const QIcon&  fallback)
{
  return (QIcon::fromTheme(name, fallback));
}

bool  PythonQtWrapper_QIcon::static_QIcon_hasThemeIcon(const QString&  name)
{
  return (QIcon::hasThemeIcon(name));
}

bool  PythonQtWrapper_QIcon::isNull(QIcon* theWrappedObject) const
{
  return ( theWrappedObject->isNull());
}

void PythonQtWrapper_QIcon::writeTo(QIcon* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 <<  (*theWrappedObject);
}

void PythonQtWrapper_QIcon::readFrom(QIcon* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 >>  (*theWrappedObject);
}

void PythonQtWrapper_QIcon::paint(QIcon* theWrappedObject, QPainter*  painter, const QRect&  rect, Qt::Alignment  alignment, QIcon::Mode  mode, QIcon::State  state) const
{
  ( theWrappedObject->paint(painter, rect, alignment, mode, state));
}

void PythonQtWrapper_QIcon::paint(QIcon* theWrappedObject, QPainter*  painter, int  x, int  y, int  w, int  h, Qt::Alignment  alignment, QIcon::Mode  mode, QIcon::State  state) const
{
  ( theWrappedObject->paint(painter, x, y, w, h, alignment, mode, state));
}

QPixmap  PythonQtWrapper_QIcon::pixmap(QIcon* theWrappedObject, const QSize&  size, QIcon::Mode  mode, QIcon::State  state) const
{
  return ( theWrappedObject->pixmap(size, mode, state));
}

QPixmap  PythonQtWrapper_QIcon::pixmap(QIcon* theWrappedObject, int  extent, QIcon::Mode  mode, QIcon::State  state) const
{
  return ( theWrappedObject->pixmap(extent, mode, state));
}

QPixmap  PythonQtWrapper_QIcon::pixmap(QIcon* theWrappedObject, int  w, int  h, QIcon::Mode  mode, QIcon::State  state) const
{
  return ( theWrappedObject->pixmap(w, h, mode, state));
}

void PythonQtWrapper_QIcon::static_QIcon_setThemeName(const QString&  path)
{
  (QIcon::setThemeName(path));
}

void PythonQtWrapper_QIcon::static_QIcon_setThemeSearchPaths(const QStringList&  searchpath)
{
  (QIcon::setThemeSearchPaths(searchpath));
}

QString  PythonQtWrapper_QIcon::static_QIcon_themeName()
{
  return (QIcon::themeName());
}

QStringList  PythonQtWrapper_QIcon::static_QIcon_themeSearchPaths()
{
  return (QIcon::themeSearchPaths());
}



int  PythonQtShell_QImage::devType() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "devType");
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
          PythonQt::priv()->handleVirtualOverloadReturnError("devType", methodInfo, result);
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
  return QImage::devType();
}
int  PythonQtShell_QImage::metric(QPaintDevice::PaintDeviceMetric  metric) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "metric");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"int" , "QPaintDevice::PaintDeviceMetric"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      int returnValue;
    void* args[2] = {NULL, (void*)&metric};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("metric", methodInfo, result);
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
  return QImage::metric(metric);
}
QPaintEngine*  PythonQtShell_QImage::paintEngine() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "paintEngine");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QPaintEngine*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QPaintEngine* returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("paintEngine", methodInfo, result);
        } else {
          returnValue = *((QPaintEngine**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QImage::paintEngine();
}
QImage* PythonQtWrapper_QImage::new_QImage()
{ 
return new PythonQtShell_QImage(); }

QImage* PythonQtWrapper_QImage::new_QImage(const QImage&  arg__1)
{ 
return new PythonQtShell_QImage(arg__1); }

QImage* PythonQtWrapper_QImage::new_QImage(const QSize&  size, QImage::Format  format)
{ 
return new PythonQtShell_QImage(size, format); }

QImage* PythonQtWrapper_QImage::new_QImage(const QString&  fileName, const char*  format)
{ 
return new PythonQtShell_QImage(fileName, format); }

QImage* PythonQtWrapper_QImage::new_QImage(int  width, int  height, QImage::Format  format)
{ 
return new PythonQtShell_QImage(width, height, format); }

bool  PythonQtWrapper_QImage::allGray(QImage* theWrappedObject) const
{
  return ( theWrappedObject->allGray());
}

QImage  PythonQtWrapper_QImage::alphaChannel(QImage* theWrappedObject) const
{
  return ( theWrappedObject->alphaChannel());
}

int  PythonQtWrapper_QImage::byteCount(QImage* theWrappedObject) const
{
  return ( theWrappedObject->byteCount());
}

int  PythonQtWrapper_QImage::bytesPerLine(QImage* theWrappedObject) const
{
  return ( theWrappedObject->bytesPerLine());
}

qint64  PythonQtWrapper_QImage::cacheKey(QImage* theWrappedObject) const
{
  return ( theWrappedObject->cacheKey());
}

unsigned int  PythonQtWrapper_QImage::color(QImage* theWrappedObject, int  i) const
{
  return ( theWrappedObject->color(i));
}

int  PythonQtWrapper_QImage::colorCount(QImage* theWrappedObject) const
{
  return ( theWrappedObject->colorCount());
}

QVector<unsigned int >  PythonQtWrapper_QImage::colorTable(QImage* theWrappedObject) const
{
  return ( theWrappedObject->colorTable());
}

QImage  PythonQtWrapper_QImage::convertToFormat(QImage* theWrappedObject, QImage::Format  f, Qt::ImageConversionFlags  flags) const
{
  return ( theWrappedObject->convertToFormat(f, flags));
}

QImage  PythonQtWrapper_QImage::convertToFormat(QImage* theWrappedObject, QImage::Format  f, const QVector<unsigned int >&  colorTable, Qt::ImageConversionFlags  flags) const
{
  return ( theWrappedObject->convertToFormat(f, colorTable, flags));
}

QImage  PythonQtWrapper_QImage::copy(QImage* theWrappedObject, const QRect&  rect) const
{
  return ( theWrappedObject->copy(rect));
}

QImage  PythonQtWrapper_QImage::copy(QImage* theWrappedObject, int  x, int  y, int  w, int  h) const
{
  return ( theWrappedObject->copy(x, y, w, h));
}

QImage  PythonQtWrapper_QImage::createAlphaMask(QImage* theWrappedObject, Qt::ImageConversionFlags  flags) const
{
  return ( theWrappedObject->createAlphaMask(flags));
}

QImage  PythonQtWrapper_QImage::createHeuristicMask(QImage* theWrappedObject, bool  clipTight) const
{
  return ( theWrappedObject->createHeuristicMask(clipTight));
}

QImage  PythonQtWrapper_QImage::createMaskFromColor(QImage* theWrappedObject, unsigned int  color, Qt::MaskMode  mode) const
{
  return ( theWrappedObject->createMaskFromColor(color, mode));
}

int  PythonQtWrapper_QImage::depth(QImage* theWrappedObject) const
{
  return ( theWrappedObject->depth());
}

int  PythonQtWrapper_QImage::devType(QImage* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_QImage*)theWrappedObject)->promoted_devType());
}

int  PythonQtWrapper_QImage::dotsPerMeterX(QImage* theWrappedObject) const
{
  return ( theWrappedObject->dotsPerMeterX());
}

int  PythonQtWrapper_QImage::dotsPerMeterY(QImage* theWrappedObject) const
{
  return ( theWrappedObject->dotsPerMeterY());
}

void PythonQtWrapper_QImage::fill(QImage* theWrappedObject, uint  pixel)
{
  ( theWrappedObject->fill(pixel));
}

QImage::Format  PythonQtWrapper_QImage::format(QImage* theWrappedObject) const
{
  return ( theWrappedObject->format());
}

QImage  PythonQtWrapper_QImage::static_QImage_fromData(const QByteArray&  data, const char*  format)
{
  return (QImage::fromData(data, format));
}

bool  PythonQtWrapper_QImage::hasAlphaChannel(QImage* theWrappedObject) const
{
  return ( theWrappedObject->hasAlphaChannel());
}

int  PythonQtWrapper_QImage::height(QImage* theWrappedObject) const
{
  return ( theWrappedObject->height());
}

void PythonQtWrapper_QImage::invertPixels(QImage* theWrappedObject, QImage::InvertMode  arg__1)
{
  ( theWrappedObject->invertPixels(arg__1));
}

bool  PythonQtWrapper_QImage::isGrayscale(QImage* theWrappedObject) const
{
  return ( theWrappedObject->isGrayscale());
}

bool  PythonQtWrapper_QImage::isNull(QImage* theWrappedObject) const
{
  return ( theWrappedObject->isNull());
}

bool  PythonQtWrapper_QImage::load(QImage* theWrappedObject, QIODevice*  device, const char*  format)
{
  return ( theWrappedObject->load(device, format));
}

bool  PythonQtWrapper_QImage::load(QImage* theWrappedObject, const QString&  fileName, const char*  format)
{
  return ( theWrappedObject->load(fileName, format));
}

bool  PythonQtWrapper_QImage::loadFromData(QImage* theWrappedObject, const QByteArray&  data, const char*  aformat)
{
  return ( theWrappedObject->loadFromData(data, aformat));
}

int  PythonQtWrapper_QImage::metric(QImage* theWrappedObject, QPaintDevice::PaintDeviceMetric  metric) const
{
  return ( ((PythonQtPublicPromoter_QImage*)theWrappedObject)->promoted_metric(metric));
}

QImage  PythonQtWrapper_QImage::mirrored(QImage* theWrappedObject, bool  horizontally, bool  vertically) const
{
  return ( theWrappedObject->mirrored(horizontally, vertically));
}

int  PythonQtWrapper_QImage::numBytes(QImage* theWrappedObject) const
{
  return ( theWrappedObject->numBytes());
}

int  PythonQtWrapper_QImage::numColors(QImage* theWrappedObject) const
{
  return ( theWrappedObject->numColors());
}

QPoint  PythonQtWrapper_QImage::offset(QImage* theWrappedObject) const
{
  return ( theWrappedObject->offset());
}

bool  PythonQtWrapper_QImage::__ne__(QImage* theWrappedObject, const QImage&  arg__1) const
{
  return ( (*theWrappedObject)!= arg__1);
}

void PythonQtWrapper_QImage::writeTo(QImage* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 <<  (*theWrappedObject);
}

bool  PythonQtWrapper_QImage::__eq__(QImage* theWrappedObject, const QImage&  arg__1) const
{
  return ( (*theWrappedObject)== arg__1);
}

void PythonQtWrapper_QImage::readFrom(QImage* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 >>  (*theWrappedObject);
}

QPaintEngine*  PythonQtWrapper_QImage::paintEngine(QImage* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_QImage*)theWrappedObject)->promoted_paintEngine());
}

unsigned int  PythonQtWrapper_QImage::pixel(QImage* theWrappedObject, const QPoint&  pt) const
{
  return ( theWrappedObject->pixel(pt));
}

unsigned int  PythonQtWrapper_QImage::pixel(QImage* theWrappedObject, int  x, int  y) const
{
  return ( theWrappedObject->pixel(x, y));
}

int  PythonQtWrapper_QImage::pixelIndex(QImage* theWrappedObject, const QPoint&  pt) const
{
  return ( theWrappedObject->pixelIndex(pt));
}

int  PythonQtWrapper_QImage::pixelIndex(QImage* theWrappedObject, int  x, int  y) const
{
  return ( theWrappedObject->pixelIndex(x, y));
}

QRect  PythonQtWrapper_QImage::rect(QImage* theWrappedObject) const
{
  return ( theWrappedObject->rect());
}

QImage  PythonQtWrapper_QImage::rgbSwapped(QImage* theWrappedObject) const
{
  return ( theWrappedObject->rgbSwapped());
}

bool  PythonQtWrapper_QImage::save(QImage* theWrappedObject, QIODevice*  device, const char*  format, int  quality) const
{
  return ( theWrappedObject->save(device, format, quality));
}

bool  PythonQtWrapper_QImage::save(QImage* theWrappedObject, const QString&  fileName, const char*  format, int  quality) const
{
  return ( theWrappedObject->save(fileName, format, quality));
}

QImage  PythonQtWrapper_QImage::scaled(QImage* theWrappedObject, const QSize&  s, Qt::AspectRatioMode  aspectMode, Qt::TransformationMode  mode) const
{
  return ( theWrappedObject->scaled(s, aspectMode, mode));
}

QImage  PythonQtWrapper_QImage::scaled(QImage* theWrappedObject, int  w, int  h, Qt::AspectRatioMode  aspectMode, Qt::TransformationMode  mode) const
{
  return ( theWrappedObject->scaled(w, h, aspectMode, mode));
}

QImage  PythonQtWrapper_QImage::scaledToHeight(QImage* theWrappedObject, int  h, Qt::TransformationMode  mode) const
{
  return ( theWrappedObject->scaledToHeight(h, mode));
}

QImage  PythonQtWrapper_QImage::scaledToWidth(QImage* theWrappedObject, int  w, Qt::TransformationMode  mode) const
{
  return ( theWrappedObject->scaledToWidth(w, mode));
}

void PythonQtWrapper_QImage::setAlphaChannel(QImage* theWrappedObject, const QImage&  alphaChannel)
{
  ( theWrappedObject->setAlphaChannel(alphaChannel));
}

void PythonQtWrapper_QImage::setColor(QImage* theWrappedObject, int  i, unsigned int  c)
{
  ( theWrappedObject->setColor(i, c));
}

void PythonQtWrapper_QImage::setColorCount(QImage* theWrappedObject, int  arg__1)
{
  ( theWrappedObject->setColorCount(arg__1));
}

void PythonQtWrapper_QImage::setDotsPerMeterX(QImage* theWrappedObject, int  arg__1)
{
  ( theWrappedObject->setDotsPerMeterX(arg__1));
}

void PythonQtWrapper_QImage::setDotsPerMeterY(QImage* theWrappedObject, int  arg__1)
{
  ( theWrappedObject->setDotsPerMeterY(arg__1));
}

void PythonQtWrapper_QImage::setNumColors(QImage* theWrappedObject, int  arg__1)
{
  ( theWrappedObject->setNumColors(arg__1));
}

void PythonQtWrapper_QImage::setOffset(QImage* theWrappedObject, const QPoint&  arg__1)
{
  ( theWrappedObject->setOffset(arg__1));
}

void PythonQtWrapper_QImage::setPixel(QImage* theWrappedObject, const QPoint&  pt, uint  index_or_rgb)
{
  ( theWrappedObject->setPixel(pt, index_or_rgb));
}

void PythonQtWrapper_QImage::setPixel(QImage* theWrappedObject, int  x, int  y, uint  index_or_rgb)
{
  ( theWrappedObject->setPixel(x, y, index_or_rgb));
}

void PythonQtWrapper_QImage::setText(QImage* theWrappedObject, const QString&  key, const QString&  value)
{
  ( theWrappedObject->setText(key, value));
}

QSize  PythonQtWrapper_QImage::size(QImage* theWrappedObject) const
{
  return ( theWrappedObject->size());
}

QString  PythonQtWrapper_QImage::text(QImage* theWrappedObject, const QString&  key) const
{
  return ( theWrappedObject->text(key));
}

QStringList  PythonQtWrapper_QImage::textKeys(QImage* theWrappedObject) const
{
  return ( theWrappedObject->textKeys());
}

QImage  PythonQtWrapper_QImage::transformed(QImage* theWrappedObject, const QMatrix&  matrix, Qt::TransformationMode  mode) const
{
  return ( theWrappedObject->transformed(matrix, mode));
}

QImage  PythonQtWrapper_QImage::transformed(QImage* theWrappedObject, const QTransform&  matrix, Qt::TransformationMode  mode) const
{
  return ( theWrappedObject->transformed(matrix, mode));
}

QMatrix  PythonQtWrapper_QImage::static_QImage_trueMatrix(const QMatrix&  arg__1, int  w, int  h)
{
  return (QImage::trueMatrix(arg__1, w, h));
}

QTransform  PythonQtWrapper_QImage::static_QImage_trueMatrix(const QTransform&  arg__1, int  w, int  h)
{
  return (QImage::trueMatrix(arg__1, w, h));
}

bool  PythonQtWrapper_QImage::valid(QImage* theWrappedObject, const QPoint&  pt) const
{
  return ( theWrappedObject->valid(pt));
}

bool  PythonQtWrapper_QImage::valid(QImage* theWrappedObject, int  x, int  y) const
{
  return ( theWrappedObject->valid(x, y));
}

int  PythonQtWrapper_QImage::width(QImage* theWrappedObject) const
{
  return ( theWrappedObject->width());
}



QKeySequence* PythonQtWrapper_QKeySequence::new_QKeySequence()
{ 
return new QKeySequence(); }

QKeySequence* PythonQtWrapper_QKeySequence::new_QKeySequence(QKeySequence::StandardKey  key)
{ 
return new QKeySequence(key); }

QKeySequence* PythonQtWrapper_QKeySequence::new_QKeySequence(const QKeySequence&  ks)
{ 
return new QKeySequence(ks); }

QKeySequence* PythonQtWrapper_QKeySequence::new_QKeySequence(const QString&  key)
{ 
return new QKeySequence(key); }

QKeySequence* PythonQtWrapper_QKeySequence::new_QKeySequence(int  k1, int  k2, int  k3, int  k4)
{ 
return new QKeySequence(k1, k2, k3, k4); }

uint  PythonQtWrapper_QKeySequence::count(QKeySequence* theWrappedObject) const
{
  return ( theWrappedObject->count());
}

QKeySequence  PythonQtWrapper_QKeySequence::static_QKeySequence_fromString(const QString&  str, QKeySequence::SequenceFormat  format)
{
  return (QKeySequence::fromString(str, format));
}

bool  PythonQtWrapper_QKeySequence::isEmpty(QKeySequence* theWrappedObject) const
{
  return ( theWrappedObject->isEmpty());
}

QList<QKeySequence >  PythonQtWrapper_QKeySequence::static_QKeySequence_keyBindings(QKeySequence::StandardKey  key)
{
  return (QKeySequence::keyBindings(key));
}

QKeySequence::SequenceMatch  PythonQtWrapper_QKeySequence::matches(QKeySequence* theWrappedObject, const QKeySequence&  seq) const
{
  return ( theWrappedObject->matches(seq));
}

QKeySequence  PythonQtWrapper_QKeySequence::static_QKeySequence_mnemonic(const QString&  text)
{
  return (QKeySequence::mnemonic(text));
}

int  PythonQtWrapper_QKeySequence::operator_cast_int(QKeySequence* theWrappedObject) const
{
  return ( theWrappedObject->operator int());
}

bool  PythonQtWrapper_QKeySequence::__ne__(QKeySequence* theWrappedObject, const QKeySequence&  other) const
{
  return ( (*theWrappedObject)!= other);
}

bool  PythonQtWrapper_QKeySequence::__lt__(QKeySequence* theWrappedObject, const QKeySequence&  ks) const
{
  return ( (*theWrappedObject)< ks);
}

void PythonQtWrapper_QKeySequence::writeTo(QKeySequence* theWrappedObject, QDataStream&  in)
{
  in <<  (*theWrappedObject);
}

bool  PythonQtWrapper_QKeySequence::__le__(QKeySequence* theWrappedObject, const QKeySequence&  other) const
{
  return ( (*theWrappedObject)<= other);
}

bool  PythonQtWrapper_QKeySequence::__eq__(QKeySequence* theWrappedObject, const QKeySequence&  other) const
{
  return ( (*theWrappedObject)== other);
}

bool  PythonQtWrapper_QKeySequence::__gt__(QKeySequence* theWrappedObject, const QKeySequence&  other) const
{
  return ( (*theWrappedObject)> other);
}

bool  PythonQtWrapper_QKeySequence::__ge__(QKeySequence* theWrappedObject, const QKeySequence&  other) const
{
  return ( (*theWrappedObject)>= other);
}

void PythonQtWrapper_QKeySequence::readFrom(QKeySequence* theWrappedObject, QDataStream&  out)
{
  out >>  (*theWrappedObject);
}

int  PythonQtWrapper_QKeySequence::operator_subscript(QKeySequence* theWrappedObject, uint  i) const
{
  return ( (*theWrappedObject)[i]);
}

QString  PythonQtWrapper_QKeySequence::toString(QKeySequence* theWrappedObject, QKeySequence::SequenceFormat  format) const
{
  return ( theWrappedObject->toString(format));
}

QString PythonQtWrapper_QKeySequence::py_toString(QKeySequence* obj) { return obj->toString(); }


QMatrix* PythonQtWrapper_QMatrix::new_QMatrix()
{ 
return new QMatrix(); }

QMatrix* PythonQtWrapper_QMatrix::new_QMatrix(const QMatrix&  matrix)
{ 
return new QMatrix(matrix); }

QMatrix* PythonQtWrapper_QMatrix::new_QMatrix(qreal  m11, qreal  m12, qreal  m21, qreal  m22, qreal  dx, qreal  dy)
{ 
return new QMatrix(m11, m12, m21, m22, dx, dy); }

qreal  PythonQtWrapper_QMatrix::det(QMatrix* theWrappedObject) const
{
  return ( theWrappedObject->det());
}

qreal  PythonQtWrapper_QMatrix::determinant(QMatrix* theWrappedObject) const
{
  return ( theWrappedObject->determinant());
}

qreal  PythonQtWrapper_QMatrix::dx(QMatrix* theWrappedObject) const
{
  return ( theWrappedObject->dx());
}

qreal  PythonQtWrapper_QMatrix::dy(QMatrix* theWrappedObject) const
{
  return ( theWrappedObject->dy());
}

QMatrix  PythonQtWrapper_QMatrix::inverted(QMatrix* theWrappedObject, bool*  invertible) const
{
  return ( theWrappedObject->inverted(invertible));
}

bool  PythonQtWrapper_QMatrix::isIdentity(QMatrix* theWrappedObject) const
{
  return ( theWrappedObject->isIdentity());
}

bool  PythonQtWrapper_QMatrix::isInvertible(QMatrix* theWrappedObject) const
{
  return ( theWrappedObject->isInvertible());
}

qreal  PythonQtWrapper_QMatrix::m11(QMatrix* theWrappedObject) const
{
  return ( theWrappedObject->m11());
}

qreal  PythonQtWrapper_QMatrix::m12(QMatrix* theWrappedObject) const
{
  return ( theWrappedObject->m12());
}

qreal  PythonQtWrapper_QMatrix::m21(QMatrix* theWrappedObject) const
{
  return ( theWrappedObject->m21());
}

qreal  PythonQtWrapper_QMatrix::m22(QMatrix* theWrappedObject) const
{
  return ( theWrappedObject->m22());
}

QLine  PythonQtWrapper_QMatrix::map(QMatrix* theWrappedObject, const QLine&  l) const
{
  return ( theWrappedObject->map(l));
}

QLineF  PythonQtWrapper_QMatrix::map(QMatrix* theWrappedObject, const QLineF&  l) const
{
  return ( theWrappedObject->map(l));
}

QPainterPath  PythonQtWrapper_QMatrix::map(QMatrix* theWrappedObject, const QPainterPath&  p) const
{
  return ( theWrappedObject->map(p));
}

QPoint  PythonQtWrapper_QMatrix::map(QMatrix* theWrappedObject, const QPoint&  p) const
{
  return ( theWrappedObject->map(p));
}

QPointF  PythonQtWrapper_QMatrix::map(QMatrix* theWrappedObject, const QPointF&  p) const
{
  return ( theWrappedObject->map(p));
}

QPolygon  PythonQtWrapper_QMatrix::map(QMatrix* theWrappedObject, const QPolygon&  a) const
{
  return ( theWrappedObject->map(a));
}

QPolygonF  PythonQtWrapper_QMatrix::map(QMatrix* theWrappedObject, const QPolygonF&  a) const
{
  return ( theWrappedObject->map(a));
}

QRegion  PythonQtWrapper_QMatrix::map(QMatrix* theWrappedObject, const QRegion&  r) const
{
  return ( theWrappedObject->map(r));
}

void PythonQtWrapper_QMatrix::map(QMatrix* theWrappedObject, qreal  x, qreal  y, qreal*  tx, qreal*  ty) const
{
  ( theWrappedObject->map(x, y, tx, ty));
}

QRect  PythonQtWrapper_QMatrix::mapRect(QMatrix* theWrappedObject, const QRect&  arg__1) const
{
  return ( theWrappedObject->mapRect(arg__1));
}

QRectF  PythonQtWrapper_QMatrix::mapRect(QMatrix* theWrappedObject, const QRectF&  arg__1) const
{
  return ( theWrappedObject->mapRect(arg__1));
}

QPolygon  PythonQtWrapper_QMatrix::mapToPolygon(QMatrix* theWrappedObject, const QRect&  r) const
{
  return ( theWrappedObject->mapToPolygon(r));
}

bool  PythonQtWrapper_QMatrix::__ne__(QMatrix* theWrappedObject, const QMatrix&  arg__1) const
{
  return ( (*theWrappedObject)!= arg__1);
}

QMatrix  PythonQtWrapper_QMatrix::__mul__(QMatrix* theWrappedObject, const QMatrix&  o) const
{
  return ( (*theWrappedObject)* o);
}

QMatrix*  PythonQtWrapper_QMatrix::__imul__(QMatrix* theWrappedObject, const QMatrix&  arg__1)
{
  return &( (*theWrappedObject)*= arg__1);
}

void PythonQtWrapper_QMatrix::writeTo(QMatrix* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 <<  (*theWrappedObject);
}

bool  PythonQtWrapper_QMatrix::__eq__(QMatrix* theWrappedObject, const QMatrix&  arg__1) const
{
  return ( (*theWrappedObject)== arg__1);
}

void PythonQtWrapper_QMatrix::readFrom(QMatrix* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 >>  (*theWrappedObject);
}

void PythonQtWrapper_QMatrix::reset(QMatrix* theWrappedObject)
{
  ( theWrappedObject->reset());
}

QMatrix*  PythonQtWrapper_QMatrix::rotate(QMatrix* theWrappedObject, qreal  a)
{
  return &( theWrappedObject->rotate(a));
}

QMatrix*  PythonQtWrapper_QMatrix::scale(QMatrix* theWrappedObject, qreal  sx, qreal  sy)
{
  return &( theWrappedObject->scale(sx, sy));
}

void PythonQtWrapper_QMatrix::setMatrix(QMatrix* theWrappedObject, qreal  m11, qreal  m12, qreal  m21, qreal  m22, qreal  dx, qreal  dy)
{
  ( theWrappedObject->setMatrix(m11, m12, m21, m22, dx, dy));
}

QMatrix*  PythonQtWrapper_QMatrix::shear(QMatrix* theWrappedObject, qreal  sh, qreal  sv)
{
  return &( theWrappedObject->shear(sh, sv));
}

QMatrix*  PythonQtWrapper_QMatrix::translate(QMatrix* theWrappedObject, qreal  dx, qreal  dy)
{
  return &( theWrappedObject->translate(dx, dy));
}

QString PythonQtWrapper_QMatrix::py_toString(QMatrix* obj) {
  QString result;
  QDebug d(&result);
  d << *obj;
  return result;
}



QPalette* PythonQtWrapper_QPalette::new_QPalette()
{ 
return new QPalette(); }

QPalette* PythonQtWrapper_QPalette::new_QPalette(Qt::GlobalColor  button)
{ 
return new QPalette(button); }

QPalette* PythonQtWrapper_QPalette::new_QPalette(const QBrush&  windowText, const QBrush&  button, const QBrush&  light, const QBrush&  dark, const QBrush&  mid, const QBrush&  text, const QBrush&  bright_text, const QBrush&  base, const QBrush&  window)
{ 
return new QPalette(windowText, button, light, dark, mid, text, bright_text, base, window); }

QPalette* PythonQtWrapper_QPalette::new_QPalette(const QColor&  button)
{ 
return new QPalette(button); }

QPalette* PythonQtWrapper_QPalette::new_QPalette(const QColor&  button, const QColor&  window)
{ 
return new QPalette(button, window); }

QPalette* PythonQtWrapper_QPalette::new_QPalette(const QPalette&  palette)
{ 
return new QPalette(palette); }

const QBrush*  PythonQtWrapper_QPalette::alternateBase(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->alternateBase());
}

const QBrush*  PythonQtWrapper_QPalette::base(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->base());
}

const QBrush*  PythonQtWrapper_QPalette::brightText(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->brightText());
}

const QBrush*  PythonQtWrapper_QPalette::brush(QPalette* theWrappedObject, QPalette::ColorGroup  cg, QPalette::ColorRole  cr) const
{
  return &( theWrappedObject->brush(cg, cr));
}

const QBrush*  PythonQtWrapper_QPalette::brush(QPalette* theWrappedObject, QPalette::ColorRole  cr) const
{
  return &( theWrappedObject->brush(cr));
}

const QBrush*  PythonQtWrapper_QPalette::button(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->button());
}

const QBrush*  PythonQtWrapper_QPalette::buttonText(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->buttonText());
}

qint64  PythonQtWrapper_QPalette::cacheKey(QPalette* theWrappedObject) const
{
  return ( theWrappedObject->cacheKey());
}

const QColor*  PythonQtWrapper_QPalette::color(QPalette* theWrappedObject, QPalette::ColorGroup  cg, QPalette::ColorRole  cr) const
{
  return &( theWrappedObject->color(cg, cr));
}

const QColor*  PythonQtWrapper_QPalette::color(QPalette* theWrappedObject, QPalette::ColorRole  cr) const
{
  return &( theWrappedObject->color(cr));
}

QPalette::ColorGroup  PythonQtWrapper_QPalette::currentColorGroup(QPalette* theWrappedObject) const
{
  return ( theWrappedObject->currentColorGroup());
}

const QBrush*  PythonQtWrapper_QPalette::dark(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->dark());
}

const QBrush*  PythonQtWrapper_QPalette::highlight(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->highlight());
}

const QBrush*  PythonQtWrapper_QPalette::highlightedText(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->highlightedText());
}

bool  PythonQtWrapper_QPalette::isBrushSet(QPalette* theWrappedObject, QPalette::ColorGroup  cg, QPalette::ColorRole  cr) const
{
  return ( theWrappedObject->isBrushSet(cg, cr));
}

bool  PythonQtWrapper_QPalette::isCopyOf(QPalette* theWrappedObject, const QPalette&  p) const
{
  return ( theWrappedObject->isCopyOf(p));
}

bool  PythonQtWrapper_QPalette::isEqual(QPalette* theWrappedObject, QPalette::ColorGroup  cr1, QPalette::ColorGroup  cr2) const
{
  return ( theWrappedObject->isEqual(cr1, cr2));
}

const QBrush*  PythonQtWrapper_QPalette::light(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->light());
}

const QBrush*  PythonQtWrapper_QPalette::link(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->link());
}

const QBrush*  PythonQtWrapper_QPalette::linkVisited(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->linkVisited());
}

const QBrush*  PythonQtWrapper_QPalette::mid(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->mid());
}

const QBrush*  PythonQtWrapper_QPalette::midlight(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->midlight());
}

bool  PythonQtWrapper_QPalette::__ne__(QPalette* theWrappedObject, const QPalette&  p) const
{
  return ( (*theWrappedObject)!= p);
}

void PythonQtWrapper_QPalette::writeTo(QPalette* theWrappedObject, QDataStream&  ds)
{
  ds <<  (*theWrappedObject);
}

bool  PythonQtWrapper_QPalette::__eq__(QPalette* theWrappedObject, const QPalette&  p) const
{
  return ( (*theWrappedObject)== p);
}

void PythonQtWrapper_QPalette::readFrom(QPalette* theWrappedObject, QDataStream&  ds)
{
  ds >>  (*theWrappedObject);
}

uint  PythonQtWrapper_QPalette::resolve(QPalette* theWrappedObject) const
{
  return ( theWrappedObject->resolve());
}

QPalette  PythonQtWrapper_QPalette::resolve(QPalette* theWrappedObject, const QPalette&  arg__1) const
{
  return ( theWrappedObject->resolve(arg__1));
}

void PythonQtWrapper_QPalette::resolve(QPalette* theWrappedObject, uint  mask)
{
  ( theWrappedObject->resolve(mask));
}

void PythonQtWrapper_QPalette::setBrush(QPalette* theWrappedObject, QPalette::ColorGroup  cg, QPalette::ColorRole  cr, const QBrush&  brush)
{
  ( theWrappedObject->setBrush(cg, cr, brush));
}

void PythonQtWrapper_QPalette::setBrush(QPalette* theWrappedObject, QPalette::ColorRole  cr, const QBrush&  brush)
{
  ( theWrappedObject->setBrush(cr, brush));
}

void PythonQtWrapper_QPalette::setColor(QPalette* theWrappedObject, QPalette::ColorGroup  cg, QPalette::ColorRole  cr, const QColor&  color)
{
  ( theWrappedObject->setColor(cg, cr, color));
}

void PythonQtWrapper_QPalette::setColor(QPalette* theWrappedObject, QPalette::ColorRole  cr, const QColor&  color)
{
  ( theWrappedObject->setColor(cr, color));
}

void PythonQtWrapper_QPalette::setColorGroup(QPalette* theWrappedObject, QPalette::ColorGroup  cr, const QBrush&  windowText, const QBrush&  button, const QBrush&  light, const QBrush&  dark, const QBrush&  mid, const QBrush&  text, const QBrush&  bright_text, const QBrush&  base, const QBrush&  window)
{
  ( theWrappedObject->setColorGroup(cr, windowText, button, light, dark, mid, text, bright_text, base, window));
}

void PythonQtWrapper_QPalette::setCurrentColorGroup(QPalette* theWrappedObject, QPalette::ColorGroup  cg)
{
  ( theWrappedObject->setCurrentColorGroup(cg));
}

const QBrush*  PythonQtWrapper_QPalette::shadow(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->shadow());
}

const QBrush*  PythonQtWrapper_QPalette::text(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->text());
}

const QBrush*  PythonQtWrapper_QPalette::toolTipBase(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->toolTipBase());
}

const QBrush*  PythonQtWrapper_QPalette::toolTipText(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->toolTipText());
}

const QBrush*  PythonQtWrapper_QPalette::window(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->window());
}

const QBrush*  PythonQtWrapper_QPalette::windowText(QPalette* theWrappedObject) const
{
  return &( theWrappedObject->windowText());
}



QPen* PythonQtWrapper_QPen::new_QPen()
{ 
return new QPen(); }

QPen* PythonQtWrapper_QPen::new_QPen(Qt::PenStyle  arg__1)
{ 
return new QPen(arg__1); }

QPen* PythonQtWrapper_QPen::new_QPen(const QBrush&  brush, qreal  width, Qt::PenStyle  s, Qt::PenCapStyle  c, Qt::PenJoinStyle  j)
{ 
return new QPen(brush, width, s, c, j); }

QPen* PythonQtWrapper_QPen::new_QPen(const QColor&  color)
{ 
return new QPen(color); }

QPen* PythonQtWrapper_QPen::new_QPen(const QPen&  pen)
{ 
return new QPen(pen); }

QBrush  PythonQtWrapper_QPen::brush(QPen* theWrappedObject) const
{
  return ( theWrappedObject->brush());
}

Qt::PenCapStyle  PythonQtWrapper_QPen::capStyle(QPen* theWrappedObject) const
{
  return ( theWrappedObject->capStyle());
}

QColor  PythonQtWrapper_QPen::color(QPen* theWrappedObject) const
{
  return ( theWrappedObject->color());
}

qreal  PythonQtWrapper_QPen::dashOffset(QPen* theWrappedObject) const
{
  return ( theWrappedObject->dashOffset());
}

QVector<qreal >  PythonQtWrapper_QPen::dashPattern(QPen* theWrappedObject) const
{
  return ( theWrappedObject->dashPattern());
}

bool  PythonQtWrapper_QPen::isCosmetic(QPen* theWrappedObject) const
{
  return ( theWrappedObject->isCosmetic());
}

bool  PythonQtWrapper_QPen::isSolid(QPen* theWrappedObject) const
{
  return ( theWrappedObject->isSolid());
}

Qt::PenJoinStyle  PythonQtWrapper_QPen::joinStyle(QPen* theWrappedObject) const
{
  return ( theWrappedObject->joinStyle());
}

qreal  PythonQtWrapper_QPen::miterLimit(QPen* theWrappedObject) const
{
  return ( theWrappedObject->miterLimit());
}

bool  PythonQtWrapper_QPen::__ne__(QPen* theWrappedObject, const QPen&  p) const
{
  return ( (*theWrappedObject)!= p);
}

void PythonQtWrapper_QPen::writeTo(QPen* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 <<  (*theWrappedObject);
}

bool  PythonQtWrapper_QPen::__eq__(QPen* theWrappedObject, const QPen&  p) const
{
  return ( (*theWrappedObject)== p);
}

void PythonQtWrapper_QPen::readFrom(QPen* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 >>  (*theWrappedObject);
}

void PythonQtWrapper_QPen::setBrush(QPen* theWrappedObject, const QBrush&  brush)
{
  ( theWrappedObject->setBrush(brush));
}

void PythonQtWrapper_QPen::setCapStyle(QPen* theWrappedObject, Qt::PenCapStyle  pcs)
{
  ( theWrappedObject->setCapStyle(pcs));
}

void PythonQtWrapper_QPen::setColor(QPen* theWrappedObject, const QColor&  color)
{
  ( theWrappedObject->setColor(color));
}

void PythonQtWrapper_QPen::setCosmetic(QPen* theWrappedObject, bool  cosmetic)
{
  ( theWrappedObject->setCosmetic(cosmetic));
}

void PythonQtWrapper_QPen::setDashOffset(QPen* theWrappedObject, qreal  doffset)
{
  ( theWrappedObject->setDashOffset(doffset));
}

void PythonQtWrapper_QPen::setDashPattern(QPen* theWrappedObject, const QVector<qreal >&  pattern)
{
  ( theWrappedObject->setDashPattern(pattern));
}

void PythonQtWrapper_QPen::setJoinStyle(QPen* theWrappedObject, Qt::PenJoinStyle  pcs)
{
  ( theWrappedObject->setJoinStyle(pcs));
}

void PythonQtWrapper_QPen::setMiterLimit(QPen* theWrappedObject, qreal  limit)
{
  ( theWrappedObject->setMiterLimit(limit));
}

void PythonQtWrapper_QPen::setStyle(QPen* theWrappedObject, Qt::PenStyle  arg__1)
{
  ( theWrappedObject->setStyle(arg__1));
}

void PythonQtWrapper_QPen::setWidth(QPen* theWrappedObject, int  width)
{
  ( theWrappedObject->setWidth(width));
}

void PythonQtWrapper_QPen::setWidthF(QPen* theWrappedObject, qreal  width)
{
  ( theWrappedObject->setWidthF(width));
}

Qt::PenStyle  PythonQtWrapper_QPen::style(QPen* theWrappedObject) const
{
  return ( theWrappedObject->style());
}

int  PythonQtWrapper_QPen::width(QPen* theWrappedObject) const
{
  return ( theWrappedObject->width());
}

qreal  PythonQtWrapper_QPen::widthF(QPen* theWrappedObject) const
{
  return ( theWrappedObject->widthF());
}

QString PythonQtWrapper_QPen::py_toString(QPen* obj) {
  QString result;
  QDebug d(&result);
  d << *obj;
  return result;
}



int  PythonQtShell_QPixmap::devType() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "devType");
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
          PythonQt::priv()->handleVirtualOverloadReturnError("devType", methodInfo, result);
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
  return QPixmap::devType();
}
int  PythonQtShell_QPixmap::metric(QPaintDevice::PaintDeviceMetric  arg__1) const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "metric");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"int" , "QPaintDevice::PaintDeviceMetric"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(2, argumentList);
      int returnValue;
    void* args[2] = {NULL, (void*)&arg__1};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("metric", methodInfo, result);
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
  return QPixmap::metric(arg__1);
}
QPaintEngine*  PythonQtShell_QPixmap::paintEngine() const
{
if (_wrapper) {
  PyObject* obj = PyObject_GetAttrString((PyObject*)_wrapper, "paintEngine");
  PyErr_Clear();
  if (obj && !PythonQtSlotFunction_Check(obj)) {
    static const char* argumentList[] ={"QPaintEngine*"};
    static const PythonQtMethodInfo* methodInfo = PythonQtMethodInfo::getCachedMethodInfoFromArgumentList(1, argumentList);
      QPaintEngine* returnValue;
    void* args[1] = {NULL};
    PyObject* result = PythonQtSignalTarget::call(obj, methodInfo, args, true);
    if (result) {
      args[0] = PythonQtConv::ConvertPythonToQt(methodInfo->parameters().at(0), result, false, NULL, &returnValue);
      if (args[0]!=&returnValue) {
        if (args[0]==NULL) {
          PythonQt::priv()->handleVirtualOverloadReturnError("paintEngine", methodInfo, result);
        } else {
          returnValue = *((QPaintEngine**)args[0]);
        }
      }
    }
    if (result) { Py_DECREF(result); } 
    Py_DECREF(obj);
    return returnValue;
  }
}
  return QPixmap::paintEngine();
}
QPixmap* PythonQtWrapper_QPixmap::new_QPixmap()
{ 
return new PythonQtShell_QPixmap(); }

QPixmap* PythonQtWrapper_QPixmap::new_QPixmap(const QPixmap&  arg__1)
{ 
return new PythonQtShell_QPixmap(arg__1); }

QPixmap* PythonQtWrapper_QPixmap::new_QPixmap(const QSize&  arg__1)
{ 
return new PythonQtShell_QPixmap(arg__1); }

QPixmap* PythonQtWrapper_QPixmap::new_QPixmap(const QString&  fileName, const char*  format, Qt::ImageConversionFlags  flags)
{ 
return new PythonQtShell_QPixmap(fileName, format, flags); }

QPixmap* PythonQtWrapper_QPixmap::new_QPixmap(const char**  xpm)
{ 
return new PythonQtShell_QPixmap(xpm); }

QPixmap* PythonQtWrapper_QPixmap::new_QPixmap(int  w, int  h)
{ 
return new PythonQtShell_QPixmap(w, h); }

QPixmap  PythonQtWrapper_QPixmap::alphaChannel(QPixmap* theWrappedObject) const
{
  return ( theWrappedObject->alphaChannel());
}

qint64  PythonQtWrapper_QPixmap::cacheKey(QPixmap* theWrappedObject) const
{
  return ( theWrappedObject->cacheKey());
}

QPixmap  PythonQtWrapper_QPixmap::copy(QPixmap* theWrappedObject, const QRect&  rect) const
{
  return ( theWrappedObject->copy(rect));
}

QPixmap  PythonQtWrapper_QPixmap::copy(QPixmap* theWrappedObject, int  x, int  y, int  width, int  height) const
{
  return ( theWrappedObject->copy(x, y, width, height));
}

QBitmap  PythonQtWrapper_QPixmap::createHeuristicMask(QPixmap* theWrappedObject, bool  clipTight) const
{
  return ( theWrappedObject->createHeuristicMask(clipTight));
}

QBitmap  PythonQtWrapper_QPixmap::createMaskFromColor(QPixmap* theWrappedObject, const QColor&  maskColor) const
{
  return ( theWrappedObject->createMaskFromColor(maskColor));
}

QBitmap  PythonQtWrapper_QPixmap::createMaskFromColor(QPixmap* theWrappedObject, const QColor&  maskColor, Qt::MaskMode  mode) const
{
  return ( theWrappedObject->createMaskFromColor(maskColor, mode));
}

int  PythonQtWrapper_QPixmap::static_QPixmap_defaultDepth()
{
  return (QPixmap::defaultDepth());
}

int  PythonQtWrapper_QPixmap::depth(QPixmap* theWrappedObject) const
{
  return ( theWrappedObject->depth());
}

int  PythonQtWrapper_QPixmap::devType(QPixmap* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_QPixmap*)theWrappedObject)->promoted_devType());
}

void PythonQtWrapper_QPixmap::fill(QPixmap* theWrappedObject, const QColor&  fillColor)
{
  ( theWrappedObject->fill(fillColor));
}

void PythonQtWrapper_QPixmap::fill(QPixmap* theWrappedObject, const QWidget*  widget, const QPoint&  ofs)
{
  ( theWrappedObject->fill(widget, ofs));
}

void PythonQtWrapper_QPixmap::fill(QPixmap* theWrappedObject, const QWidget*  widget, int  xofs, int  yofs)
{
  ( theWrappedObject->fill(widget, xofs, yofs));
}

QPixmap  PythonQtWrapper_QPixmap::static_QPixmap_fromImage(const QImage&  image, Qt::ImageConversionFlags  flags)
{
  return (QPixmap::fromImage(image, flags));
}

QPixmap  PythonQtWrapper_QPixmap::static_QPixmap_grabWidget(QWidget*  widget, const QRect&  rect)
{
  return (QPixmap::grabWidget(widget, rect));
}

QPixmap  PythonQtWrapper_QPixmap::static_QPixmap_grabWidget(QWidget*  widget, int  x, int  y, int  w, int  h)
{
  return (QPixmap::grabWidget(widget, x, y, w, h));
}

QPixmap  PythonQtWrapper_QPixmap::static_QPixmap_grabWindow(WId  arg__1, int  x, int  y, int  w, int  h)
{
  return (QPixmap::grabWindow(arg__1, x, y, w, h));
}

bool  PythonQtWrapper_QPixmap::hasAlpha(QPixmap* theWrappedObject) const
{
  return ( theWrappedObject->hasAlpha());
}

bool  PythonQtWrapper_QPixmap::hasAlphaChannel(QPixmap* theWrappedObject) const
{
  return ( theWrappedObject->hasAlphaChannel());
}

int  PythonQtWrapper_QPixmap::height(QPixmap* theWrappedObject) const
{
  return ( theWrappedObject->height());
}

bool  PythonQtWrapper_QPixmap::isNull(QPixmap* theWrappedObject) const
{
  return ( theWrappedObject->isNull());
}

bool  PythonQtWrapper_QPixmap::isQBitmap(QPixmap* theWrappedObject) const
{
  return ( theWrappedObject->isQBitmap());
}

bool  PythonQtWrapper_QPixmap::load(QPixmap* theWrappedObject, const QString&  fileName, const char*  format, Qt::ImageConversionFlags  flags)
{
  return ( theWrappedObject->load(fileName, format, flags));
}

bool  PythonQtWrapper_QPixmap::loadFromData(QPixmap* theWrappedObject, const QByteArray&  data, const char*  format, Qt::ImageConversionFlags  flags)
{
  return ( theWrappedObject->loadFromData(data, format, flags));
}

QBitmap  PythonQtWrapper_QPixmap::mask(QPixmap* theWrappedObject) const
{
  return ( theWrappedObject->mask());
}

int  PythonQtWrapper_QPixmap::metric(QPixmap* theWrappedObject, QPaintDevice::PaintDeviceMetric  arg__1) const
{
  return ( ((PythonQtPublicPromoter_QPixmap*)theWrappedObject)->promoted_metric(arg__1));
}

void PythonQtWrapper_QPixmap::writeTo(QPixmap* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 <<  (*theWrappedObject);
}

void PythonQtWrapper_QPixmap::readFrom(QPixmap* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 >>  (*theWrappedObject);
}

QPaintEngine*  PythonQtWrapper_QPixmap::paintEngine(QPixmap* theWrappedObject) const
{
  return ( ((PythonQtPublicPromoter_QPixmap*)theWrappedObject)->promoted_paintEngine());
}

QRect  PythonQtWrapper_QPixmap::rect(QPixmap* theWrappedObject) const
{
  return ( theWrappedObject->rect());
}

bool  PythonQtWrapper_QPixmap::save(QPixmap* theWrappedObject, QIODevice*  device, const char*  format, int  quality) const
{
  return ( theWrappedObject->save(device, format, quality));
}

bool  PythonQtWrapper_QPixmap::save(QPixmap* theWrappedObject, const QString&  fileName, const char*  format, int  quality) const
{
  return ( theWrappedObject->save(fileName, format, quality));
}

QPixmap  PythonQtWrapper_QPixmap::scaled(QPixmap* theWrappedObject, const QSize&  s, Qt::AspectRatioMode  aspectMode, Qt::TransformationMode  mode) const
{
  return ( theWrappedObject->scaled(s, aspectMode, mode));
}

QPixmap  PythonQtWrapper_QPixmap::scaled(QPixmap* theWrappedObject, int  w, int  h, Qt::AspectRatioMode  aspectMode, Qt::TransformationMode  mode) const
{
  return ( theWrappedObject->scaled(w, h, aspectMode, mode));
}

QPixmap  PythonQtWrapper_QPixmap::scaledToHeight(QPixmap* theWrappedObject, int  h, Qt::TransformationMode  mode) const
{
  return ( theWrappedObject->scaledToHeight(h, mode));
}

QPixmap  PythonQtWrapper_QPixmap::scaledToWidth(QPixmap* theWrappedObject, int  w, Qt::TransformationMode  mode) const
{
  return ( theWrappedObject->scaledToWidth(w, mode));
}

void PythonQtWrapper_QPixmap::scroll(QPixmap* theWrappedObject, int  dx, int  dy, const QRect&  rect, QRegion*  exposed)
{
  ( theWrappedObject->scroll(dx, dy, rect, exposed));
}

void PythonQtWrapper_QPixmap::scroll(QPixmap* theWrappedObject, int  dx, int  dy, int  x, int  y, int  width, int  height, QRegion*  exposed)
{
  ( theWrappedObject->scroll(dx, dy, x, y, width, height, exposed));
}

void PythonQtWrapper_QPixmap::setAlphaChannel(QPixmap* theWrappedObject, const QPixmap&  arg__1)
{
  ( theWrappedObject->setAlphaChannel(arg__1));
}

void PythonQtWrapper_QPixmap::setMask(QPixmap* theWrappedObject, const QBitmap&  arg__1)
{
  ( theWrappedObject->setMask(arg__1));
}

QSize  PythonQtWrapper_QPixmap::size(QPixmap* theWrappedObject) const
{
  return ( theWrappedObject->size());
}

QImage  PythonQtWrapper_QPixmap::toImage(QPixmap* theWrappedObject) const
{
  return ( theWrappedObject->toImage());
}

QPixmap  PythonQtWrapper_QPixmap::transformed(QPixmap* theWrappedObject, const QMatrix&  arg__1, Qt::TransformationMode  mode) const
{
  return ( theWrappedObject->transformed(arg__1, mode));
}

QPixmap  PythonQtWrapper_QPixmap::transformed(QPixmap* theWrappedObject, const QTransform&  arg__1, Qt::TransformationMode  mode) const
{
  return ( theWrappedObject->transformed(arg__1, mode));
}

QMatrix  PythonQtWrapper_QPixmap::static_QPixmap_trueMatrix(const QMatrix&  m, int  w, int  h)
{
  return (QPixmap::trueMatrix(m, w, h));
}

QTransform  PythonQtWrapper_QPixmap::static_QPixmap_trueMatrix(const QTransform&  m, int  w, int  h)
{
  return (QPixmap::trueMatrix(m, w, h));
}

int  PythonQtWrapper_QPixmap::width(QPixmap* theWrappedObject) const
{
  return ( theWrappedObject->width());
}



QPolygon* PythonQtWrapper_QPolygon::new_QPolygon()
{ 
return new QPolygon(); }

QPolygon* PythonQtWrapper_QPolygon::new_QPolygon(const QPolygon&  a)
{ 
return new QPolygon(a); }

QPolygon* PythonQtWrapper_QPolygon::new_QPolygon(const QRect&  r, bool  closed)
{ 
return new QPolygon(r, closed); }

QPolygon* PythonQtWrapper_QPolygon::new_QPolygon(const QVector<QPoint >&  v)
{ 
return new QPolygon(v); }

QPolygon* PythonQtWrapper_QPolygon::new_QPolygon(int  size)
{ 
return new QPolygon(size); }

void PythonQtWrapper_QPolygon::append(QPolygon* theWrappedObject, const QPoint&  t)
{
  ( theWrappedObject->append(t));
}

const QPoint*  PythonQtWrapper_QPolygon::at(QPolygon* theWrappedObject, int  i) const
{
  return &( theWrappedObject->at(i));
}

QRect  PythonQtWrapper_QPolygon::boundingRect(QPolygon* theWrappedObject) const
{
  return ( theWrappedObject->boundingRect());
}

int  PythonQtWrapper_QPolygon::capacity(QPolygon* theWrappedObject) const
{
  return ( theWrappedObject->capacity());
}

void PythonQtWrapper_QPolygon::clear(QPolygon* theWrappedObject)
{
  ( theWrappedObject->clear());
}

bool  PythonQtWrapper_QPolygon::contains(QPolygon* theWrappedObject, const QPoint&  t) const
{
  return ( theWrappedObject->contains(t));
}

bool  PythonQtWrapper_QPolygon::containsPoint(QPolygon* theWrappedObject, const QPoint&  pt, Qt::FillRule  fillRule) const
{
  return ( theWrappedObject->containsPoint(pt, fillRule));
}

int  PythonQtWrapper_QPolygon::count(QPolygon* theWrappedObject) const
{
  return ( theWrappedObject->count());
}

int  PythonQtWrapper_QPolygon::count(QPolygon* theWrappedObject, const QPoint&  t) const
{
  return ( theWrappedObject->count(t));
}

bool  PythonQtWrapper_QPolygon::empty(QPolygon* theWrappedObject) const
{
  return ( theWrappedObject->empty());
}

bool  PythonQtWrapper_QPolygon::endsWith(QPolygon* theWrappedObject, const QPoint&  t) const
{
  return ( theWrappedObject->endsWith(t));
}

QVector<QPoint >*  PythonQtWrapper_QPolygon::fill(QPolygon* theWrappedObject, const QPoint&  t, int  size)
{
  return &( theWrappedObject->fill(t, size));
}

const QPoint*  PythonQtWrapper_QPolygon::first(QPolygon* theWrappedObject) const
{
  return &( theWrappedObject->first());
}

QVector<QPoint >  PythonQtWrapper_QPolygon::static_QPolygon_fromList(const QList<QPoint >&  list)
{
  return (QPolygon::fromList(list));
}

int  PythonQtWrapper_QPolygon::indexOf(QPolygon* theWrappedObject, const QPoint&  t, int  from) const
{
  return ( theWrappedObject->indexOf(t, from));
}

QPolygon  PythonQtWrapper_QPolygon::intersected(QPolygon* theWrappedObject, const QPolygon&  r) const
{
  return ( theWrappedObject->intersected(r));
}

bool  PythonQtWrapper_QPolygon::isEmpty(QPolygon* theWrappedObject) const
{
  return ( theWrappedObject->isEmpty());
}

const QPoint*  PythonQtWrapper_QPolygon::last(QPolygon* theWrappedObject) const
{
  return &( theWrappedObject->last());
}

int  PythonQtWrapper_QPolygon::lastIndexOf(QPolygon* theWrappedObject, const QPoint&  t, int  from) const
{
  return ( theWrappedObject->lastIndexOf(t, from));
}

QVector<QPoint >  PythonQtWrapper_QPolygon::mid(QPolygon* theWrappedObject, int  pos, int  length) const
{
  return ( theWrappedObject->mid(pos, length));
}

bool  PythonQtWrapper_QPolygon::__ne__(QPolygon* theWrappedObject, const QVector<QPoint >&  v) const
{
  return ( (*theWrappedObject)!= v);
}

QPolygon  PythonQtWrapper_QPolygon::__mul__(QPolygon* theWrappedObject, const QMatrix&  m)
{
  return ( (*theWrappedObject)* m);
}

QPolygon  PythonQtWrapper_QPolygon::__mul__(QPolygon* theWrappedObject, const QTransform&  m)
{
  return ( (*theWrappedObject)* m);
}

void PythonQtWrapper_QPolygon::writeTo(QPolygon* theWrappedObject, QDataStream&  stream)
{
  stream <<  (*theWrappedObject);
}

bool  PythonQtWrapper_QPolygon::__eq__(QPolygon* theWrappedObject, const QVector<QPoint >&  v) const
{
  return ( (*theWrappedObject)== v);
}

void PythonQtWrapper_QPolygon::readFrom(QPolygon* theWrappedObject, QDataStream&  stream)
{
  stream >>  (*theWrappedObject);
}

void PythonQtWrapper_QPolygon::pop_back(QPolygon* theWrappedObject)
{
  ( theWrappedObject->pop_back());
}

void PythonQtWrapper_QPolygon::pop_front(QPolygon* theWrappedObject)
{
  ( theWrappedObject->pop_front());
}

void PythonQtWrapper_QPolygon::prepend(QPolygon* theWrappedObject, const QPoint&  t)
{
  ( theWrappedObject->prepend(t));
}

void PythonQtWrapper_QPolygon::push_back(QPolygon* theWrappedObject, const QPoint&  t)
{
  ( theWrappedObject->push_back(t));
}

void PythonQtWrapper_QPolygon::push_front(QPolygon* theWrappedObject, const QPoint&  t)
{
  ( theWrappedObject->push_front(t));
}

void PythonQtWrapper_QPolygon::remove(QPolygon* theWrappedObject, int  i)
{
  ( theWrappedObject->remove(i));
}

void PythonQtWrapper_QPolygon::remove(QPolygon* theWrappedObject, int  i, int  n)
{
  ( theWrappedObject->remove(i, n));
}

void PythonQtWrapper_QPolygon::replace(QPolygon* theWrappedObject, int  i, const QPoint&  t)
{
  ( theWrappedObject->replace(i, t));
}

void PythonQtWrapper_QPolygon::reserve(QPolygon* theWrappedObject, int  size)
{
  ( theWrappedObject->reserve(size));
}

void PythonQtWrapper_QPolygon::resize(QPolygon* theWrappedObject, int  size)
{
  ( theWrappedObject->resize(size));
}

void PythonQtWrapper_QPolygon::setSharable(QPolygon* theWrappedObject, bool  sharable)
{
  ( theWrappedObject->setSharable(sharable));
}

int  PythonQtWrapper_QPolygon::size(QPolygon* theWrappedObject) const
{
  return ( theWrappedObject->size());
}

void PythonQtWrapper_QPolygon::squeeze(QPolygon* theWrappedObject)
{
  ( theWrappedObject->squeeze());
}

bool  PythonQtWrapper_QPolygon::startsWith(QPolygon* theWrappedObject, const QPoint&  t) const
{
  return ( theWrappedObject->startsWith(t));
}

QPolygon  PythonQtWrapper_QPolygon::subtracted(QPolygon* theWrappedObject, const QPolygon&  r) const
{
  return ( theWrappedObject->subtracted(r));
}

QList<QPoint >  PythonQtWrapper_QPolygon::toList(QPolygon* theWrappedObject) const
{
  return ( theWrappedObject->toList());
}

void PythonQtWrapper_QPolygon::translate(QPolygon* theWrappedObject, const QPoint&  offset)
{
  ( theWrappedObject->translate(offset));
}

void PythonQtWrapper_QPolygon::translate(QPolygon* theWrappedObject, int  dx, int  dy)
{
  ( theWrappedObject->translate(dx, dy));
}

QPolygon  PythonQtWrapper_QPolygon::translated(QPolygon* theWrappedObject, const QPoint&  offset) const
{
  return ( theWrappedObject->translated(offset));
}

QPolygon  PythonQtWrapper_QPolygon::translated(QPolygon* theWrappedObject, int  dx, int  dy) const
{
  return ( theWrappedObject->translated(dx, dy));
}

QPolygon  PythonQtWrapper_QPolygon::united(QPolygon* theWrappedObject, const QPolygon&  r) const
{
  return ( theWrappedObject->united(r));
}

QPoint  PythonQtWrapper_QPolygon::value(QPolygon* theWrappedObject, int  i) const
{
  return ( theWrappedObject->value(i));
}

QPoint  PythonQtWrapper_QPolygon::value(QPolygon* theWrappedObject, int  i, const QPoint&  defaultValue) const
{
  return ( theWrappedObject->value(i, defaultValue));
}

QString PythonQtWrapper_QPolygon::py_toString(QPolygon* obj) {
  QString result;
  QDebug d(&result);
  d << *obj;
  return result;
}



QRegion* PythonQtWrapper_QRegion::new_QRegion()
{ 
return new QRegion(); }

QRegion* PythonQtWrapper_QRegion::new_QRegion(const QBitmap&  bitmap)
{ 
return new QRegion(bitmap); }

QRegion* PythonQtWrapper_QRegion::new_QRegion(const QPolygon&  pa, Qt::FillRule  fillRule)
{ 
return new QRegion(pa, fillRule); }

QRegion* PythonQtWrapper_QRegion::new_QRegion(const QRect&  r, QRegion::RegionType  t)
{ 
return new QRegion(r, t); }

QRegion* PythonQtWrapper_QRegion::new_QRegion(const QRegion&  region)
{ 
return new QRegion(region); }

QRegion* PythonQtWrapper_QRegion::new_QRegion(int  x, int  y, int  w, int  h, QRegion::RegionType  t)
{ 
return new QRegion(x, y, w, h, t); }

QRect  PythonQtWrapper_QRegion::boundingRect(QRegion* theWrappedObject) const
{
  return ( theWrappedObject->boundingRect());
}

bool  PythonQtWrapper_QRegion::contains(QRegion* theWrappedObject, const QPoint&  p) const
{
  return ( theWrappedObject->contains(p));
}

bool  PythonQtWrapper_QRegion::contains(QRegion* theWrappedObject, const QRect&  r) const
{
  return ( theWrappedObject->contains(r));
}

QRegion  PythonQtWrapper_QRegion::intersect(QRegion* theWrappedObject, const QRect&  r) const
{
  return ( theWrappedObject->intersect(r));
}

QRegion  PythonQtWrapper_QRegion::intersected(QRegion* theWrappedObject, const QRect&  r) const
{
  return ( theWrappedObject->intersected(r));
}

QRegion  PythonQtWrapper_QRegion::intersected(QRegion* theWrappedObject, const QRegion&  r) const
{
  return ( theWrappedObject->intersected(r));
}

bool  PythonQtWrapper_QRegion::intersects(QRegion* theWrappedObject, const QRect&  r) const
{
  return ( theWrappedObject->intersects(r));
}

bool  PythonQtWrapper_QRegion::intersects(QRegion* theWrappedObject, const QRegion&  r) const
{
  return ( theWrappedObject->intersects(r));
}

bool  PythonQtWrapper_QRegion::isEmpty(QRegion* theWrappedObject) const
{
  return ( theWrappedObject->isEmpty());
}

int  PythonQtWrapper_QRegion::numRects(QRegion* theWrappedObject) const
{
  return ( theWrappedObject->numRects());
}

bool  PythonQtWrapper_QRegion::__ne__(QRegion* theWrappedObject, const QRegion&  r) const
{
  return ( (*theWrappedObject)!= r);
}

const QRegion  PythonQtWrapper_QRegion::__and__(QRegion* theWrappedObject, const QRect&  r) const
{
  return ( (*theWrappedObject)& r);
}

QRegion  PythonQtWrapper_QRegion::__mul__(QRegion* theWrappedObject, const QMatrix&  m)
{
  return ( (*theWrappedObject)* m);
}

QRegion  PythonQtWrapper_QRegion::__mul__(QRegion* theWrappedObject, const QTransform&  m)
{
  return ( (*theWrappedObject)* m);
}

const QRegion  PythonQtWrapper_QRegion::__add__(QRegion* theWrappedObject, const QRect&  r) const
{
  return ( (*theWrappedObject)+ r);
}

void PythonQtWrapper_QRegion::writeTo(QRegion* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 <<  (*theWrappedObject);
}

bool  PythonQtWrapper_QRegion::__eq__(QRegion* theWrappedObject, const QRegion&  r) const
{
  return ( (*theWrappedObject)== r);
}

void PythonQtWrapper_QRegion::readFrom(QRegion* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 >>  (*theWrappedObject);
}

int  PythonQtWrapper_QRegion::rectCount(QRegion* theWrappedObject) const
{
  return ( theWrappedObject->rectCount());
}

QVector<QRect >  PythonQtWrapper_QRegion::rects(QRegion* theWrappedObject) const
{
  return ( theWrappedObject->rects());
}

void PythonQtWrapper_QRegion::setRects(QRegion* theWrappedObject, const QRect*  rect, int  num)
{
  ( theWrappedObject->setRects(rect, num));
}

QRegion  PythonQtWrapper_QRegion::subtracted(QRegion* theWrappedObject, const QRegion&  r) const
{
  return ( theWrappedObject->subtracted(r));
}

void PythonQtWrapper_QRegion::translate(QRegion* theWrappedObject, const QPoint&  p)
{
  ( theWrappedObject->translate(p));
}

void PythonQtWrapper_QRegion::translate(QRegion* theWrappedObject, int  dx, int  dy)
{
  ( theWrappedObject->translate(dx, dy));
}

QRegion  PythonQtWrapper_QRegion::translated(QRegion* theWrappedObject, const QPoint&  p) const
{
  return ( theWrappedObject->translated(p));
}

QRegion  PythonQtWrapper_QRegion::translated(QRegion* theWrappedObject, int  dx, int  dy) const
{
  return ( theWrappedObject->translated(dx, dy));
}

QRegion  PythonQtWrapper_QRegion::unite(QRegion* theWrappedObject, const QRect&  r) const
{
  return ( theWrappedObject->unite(r));
}

QRegion  PythonQtWrapper_QRegion::united(QRegion* theWrappedObject, const QRect&  r) const
{
  return ( theWrappedObject->united(r));
}

QRegion  PythonQtWrapper_QRegion::united(QRegion* theWrappedObject, const QRegion&  r) const
{
  return ( theWrappedObject->united(r));
}

QRegion  PythonQtWrapper_QRegion::xored(QRegion* theWrappedObject, const QRegion&  r) const
{
  return ( theWrappedObject->xored(r));
}

QString PythonQtWrapper_QRegion::py_toString(QRegion* obj) {
  QString result;
  QDebug d(&result);
  d << *obj;
  return result;
}



QSizePolicy* PythonQtWrapper_QSizePolicy::new_QSizePolicy()
{ 
return new QSizePolicy(); }

QSizePolicy* PythonQtWrapper_QSizePolicy::new_QSizePolicy(QSizePolicy::Policy  horizontal, QSizePolicy::Policy  vertical)
{ 
return new QSizePolicy(horizontal, vertical); }

QSizePolicy* PythonQtWrapper_QSizePolicy::new_QSizePolicy(QSizePolicy::Policy  horizontal, QSizePolicy::Policy  vertical, QSizePolicy::ControlType  type)
{ 
return new QSizePolicy(horizontal, vertical, type); }

QSizePolicy::ControlType  PythonQtWrapper_QSizePolicy::controlType(QSizePolicy* theWrappedObject) const
{
  return ( theWrappedObject->controlType());
}

Qt::Orientations  PythonQtWrapper_QSizePolicy::expandingDirections(QSizePolicy* theWrappedObject) const
{
  return ( theWrappedObject->expandingDirections());
}

bool  PythonQtWrapper_QSizePolicy::hasHeightForWidth(QSizePolicy* theWrappedObject) const
{
  return ( theWrappedObject->hasHeightForWidth());
}

QSizePolicy::Policy  PythonQtWrapper_QSizePolicy::horizontalPolicy(QSizePolicy* theWrappedObject) const
{
  return ( theWrappedObject->horizontalPolicy());
}

int  PythonQtWrapper_QSizePolicy::horizontalStretch(QSizePolicy* theWrappedObject) const
{
  return ( theWrappedObject->horizontalStretch());
}

bool  PythonQtWrapper_QSizePolicy::__ne__(QSizePolicy* theWrappedObject, const QSizePolicy&  s) const
{
  return ( (*theWrappedObject)!= s);
}

void PythonQtWrapper_QSizePolicy::writeTo(QSizePolicy* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 <<  (*theWrappedObject);
}

bool  PythonQtWrapper_QSizePolicy::__eq__(QSizePolicy* theWrappedObject, const QSizePolicy&  s) const
{
  return ( (*theWrappedObject)== s);
}

void PythonQtWrapper_QSizePolicy::readFrom(QSizePolicy* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 >>  (*theWrappedObject);
}

void PythonQtWrapper_QSizePolicy::setControlType(QSizePolicy* theWrappedObject, QSizePolicy::ControlType  type)
{
  ( theWrappedObject->setControlType(type));
}

void PythonQtWrapper_QSizePolicy::setHeightForWidth(QSizePolicy* theWrappedObject, bool  b)
{
  ( theWrappedObject->setHeightForWidth(b));
}

void PythonQtWrapper_QSizePolicy::setHorizontalPolicy(QSizePolicy* theWrappedObject, QSizePolicy::Policy  d)
{
  ( theWrappedObject->setHorizontalPolicy(d));
}

void PythonQtWrapper_QSizePolicy::setHorizontalStretch(QSizePolicy* theWrappedObject, uchar  stretchFactor)
{
  ( theWrappedObject->setHorizontalStretch(stretchFactor));
}

void PythonQtWrapper_QSizePolicy::setVerticalPolicy(QSizePolicy* theWrappedObject, QSizePolicy::Policy  d)
{
  ( theWrappedObject->setVerticalPolicy(d));
}

void PythonQtWrapper_QSizePolicy::setVerticalStretch(QSizePolicy* theWrappedObject, uchar  stretchFactor)
{
  ( theWrappedObject->setVerticalStretch(stretchFactor));
}

void PythonQtWrapper_QSizePolicy::transpose(QSizePolicy* theWrappedObject)
{
  ( theWrappedObject->transpose());
}

QSizePolicy::Policy  PythonQtWrapper_QSizePolicy::verticalPolicy(QSizePolicy* theWrappedObject) const
{
  return ( theWrappedObject->verticalPolicy());
}

int  PythonQtWrapper_QSizePolicy::verticalStretch(QSizePolicy* theWrappedObject) const
{
  return ( theWrappedObject->verticalStretch());
}



QTextFormat* PythonQtWrapper_QTextFormat::new_QTextFormat()
{ 
return new QTextFormat(); }

QTextFormat* PythonQtWrapper_QTextFormat::new_QTextFormat(const QTextFormat&  rhs)
{ 
return new QTextFormat(rhs); }

QTextFormat* PythonQtWrapper_QTextFormat::new_QTextFormat(int  type)
{ 
return new QTextFormat(type); }

QBrush  PythonQtWrapper_QTextFormat::background(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->background());
}

bool  PythonQtWrapper_QTextFormat::boolProperty(QTextFormat* theWrappedObject, int  propertyId) const
{
  return ( theWrappedObject->boolProperty(propertyId));
}

QBrush  PythonQtWrapper_QTextFormat::brushProperty(QTextFormat* theWrappedObject, int  propertyId) const
{
  return ( theWrappedObject->brushProperty(propertyId));
}

void PythonQtWrapper_QTextFormat::clearBackground(QTextFormat* theWrappedObject)
{
  ( theWrappedObject->clearBackground());
}

void PythonQtWrapper_QTextFormat::clearForeground(QTextFormat* theWrappedObject)
{
  ( theWrappedObject->clearForeground());
}

void PythonQtWrapper_QTextFormat::clearProperty(QTextFormat* theWrappedObject, int  propertyId)
{
  ( theWrappedObject->clearProperty(propertyId));
}

QColor  PythonQtWrapper_QTextFormat::colorProperty(QTextFormat* theWrappedObject, int  propertyId) const
{
  return ( theWrappedObject->colorProperty(propertyId));
}

qreal  PythonQtWrapper_QTextFormat::doubleProperty(QTextFormat* theWrappedObject, int  propertyId) const
{
  return ( theWrappedObject->doubleProperty(propertyId));
}

QBrush  PythonQtWrapper_QTextFormat::foreground(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->foreground());
}

bool  PythonQtWrapper_QTextFormat::hasProperty(QTextFormat* theWrappedObject, int  propertyId) const
{
  return ( theWrappedObject->hasProperty(propertyId));
}

int  PythonQtWrapper_QTextFormat::intProperty(QTextFormat* theWrappedObject, int  propertyId) const
{
  return ( theWrappedObject->intProperty(propertyId));
}

bool  PythonQtWrapper_QTextFormat::isBlockFormat(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->isBlockFormat());
}

bool  PythonQtWrapper_QTextFormat::isCharFormat(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->isCharFormat());
}

bool  PythonQtWrapper_QTextFormat::isFrameFormat(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->isFrameFormat());
}

bool  PythonQtWrapper_QTextFormat::isImageFormat(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->isImageFormat());
}

bool  PythonQtWrapper_QTextFormat::isListFormat(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->isListFormat());
}

bool  PythonQtWrapper_QTextFormat::isTableCellFormat(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->isTableCellFormat());
}

bool  PythonQtWrapper_QTextFormat::isTableFormat(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->isTableFormat());
}

bool  PythonQtWrapper_QTextFormat::isValid(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->isValid());
}

Qt::LayoutDirection  PythonQtWrapper_QTextFormat::layoutDirection(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->layoutDirection());
}

QTextLength  PythonQtWrapper_QTextFormat::lengthProperty(QTextFormat* theWrappedObject, int  propertyId) const
{
  return ( theWrappedObject->lengthProperty(propertyId));
}

QVector<QTextLength >  PythonQtWrapper_QTextFormat::lengthVectorProperty(QTextFormat* theWrappedObject, int  propertyId) const
{
  return ( theWrappedObject->lengthVectorProperty(propertyId));
}

void PythonQtWrapper_QTextFormat::merge(QTextFormat* theWrappedObject, const QTextFormat&  other)
{
  ( theWrappedObject->merge(other));
}

int  PythonQtWrapper_QTextFormat::objectIndex(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->objectIndex());
}

int  PythonQtWrapper_QTextFormat::objectType(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->objectType());
}

bool  PythonQtWrapper_QTextFormat::__ne__(QTextFormat* theWrappedObject, const QTextFormat&  rhs) const
{
  return ( (*theWrappedObject)!= rhs);
}

void PythonQtWrapper_QTextFormat::writeTo(QTextFormat* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 <<  (*theWrappedObject);
}

bool  PythonQtWrapper_QTextFormat::__eq__(QTextFormat* theWrappedObject, const QTextFormat&  rhs) const
{
  return ( (*theWrappedObject)== rhs);
}

void PythonQtWrapper_QTextFormat::readFrom(QTextFormat* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 >>  (*theWrappedObject);
}

QPen  PythonQtWrapper_QTextFormat::penProperty(QTextFormat* theWrappedObject, int  propertyId) const
{
  return ( theWrappedObject->penProperty(propertyId));
}

QMap<int , QVariant >  PythonQtWrapper_QTextFormat::properties(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->properties());
}

QVariant  PythonQtWrapper_QTextFormat::property(QTextFormat* theWrappedObject, int  propertyId) const
{
  return ( theWrappedObject->property(propertyId));
}

int  PythonQtWrapper_QTextFormat::propertyCount(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->propertyCount());
}

void PythonQtWrapper_QTextFormat::setBackground(QTextFormat* theWrappedObject, const QBrush&  brush)
{
  ( theWrappedObject->setBackground(brush));
}

void PythonQtWrapper_QTextFormat::setForeground(QTextFormat* theWrappedObject, const QBrush&  brush)
{
  ( theWrappedObject->setForeground(brush));
}

void PythonQtWrapper_QTextFormat::setLayoutDirection(QTextFormat* theWrappedObject, Qt::LayoutDirection  direction)
{
  ( theWrappedObject->setLayoutDirection(direction));
}

void PythonQtWrapper_QTextFormat::setObjectIndex(QTextFormat* theWrappedObject, int  object)
{
  ( theWrappedObject->setObjectIndex(object));
}

void PythonQtWrapper_QTextFormat::setObjectType(QTextFormat* theWrappedObject, int  type)
{
  ( theWrappedObject->setObjectType(type));
}

void PythonQtWrapper_QTextFormat::setProperty(QTextFormat* theWrappedObject, int  propertyId, const QVariant&  value)
{
  ( theWrappedObject->setProperty(propertyId, value));
}

void PythonQtWrapper_QTextFormat::setProperty(QTextFormat* theWrappedObject, int  propertyId, const QVector<QTextLength >&  lengths)
{
  ( theWrappedObject->setProperty(propertyId, lengths));
}

QString  PythonQtWrapper_QTextFormat::stringProperty(QTextFormat* theWrappedObject, int  propertyId) const
{
  return ( theWrappedObject->stringProperty(propertyId));
}

QTextBlockFormat  PythonQtWrapper_QTextFormat::toBlockFormat(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->toBlockFormat());
}

QTextCharFormat  PythonQtWrapper_QTextFormat::toCharFormat(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->toCharFormat());
}

QTextFrameFormat  PythonQtWrapper_QTextFormat::toFrameFormat(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->toFrameFormat());
}

QTextImageFormat  PythonQtWrapper_QTextFormat::toImageFormat(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->toImageFormat());
}

QTextListFormat  PythonQtWrapper_QTextFormat::toListFormat(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->toListFormat());
}

QTextTableCellFormat  PythonQtWrapper_QTextFormat::toTableCellFormat(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->toTableCellFormat());
}

QTextTableFormat  PythonQtWrapper_QTextFormat::toTableFormat(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->toTableFormat());
}

int  PythonQtWrapper_QTextFormat::type(QTextFormat* theWrappedObject) const
{
  return ( theWrappedObject->type());
}



QTextLength* PythonQtWrapper_QTextLength::new_QTextLength()
{ 
return new QTextLength(); }

QTextLength* PythonQtWrapper_QTextLength::new_QTextLength(QTextLength::Type  type, qreal  value)
{ 
return new QTextLength(type, value); }

bool  PythonQtWrapper_QTextLength::__ne__(QTextLength* theWrappedObject, const QTextLength&  other) const
{
  return ( (*theWrappedObject)!= other);
}

void PythonQtWrapper_QTextLength::writeTo(QTextLength* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 <<  (*theWrappedObject);
}

bool  PythonQtWrapper_QTextLength::__eq__(QTextLength* theWrappedObject, const QTextLength&  other) const
{
  return ( (*theWrappedObject)== other);
}

void PythonQtWrapper_QTextLength::readFrom(QTextLength* theWrappedObject, QDataStream&  arg__1)
{
  arg__1 >>  (*theWrappedObject);
}

qreal  PythonQtWrapper_QTextLength::rawValue(QTextLength* theWrappedObject) const
{
  return ( theWrappedObject->rawValue());
}

QTextLength::Type  PythonQtWrapper_QTextLength::type(QTextLength* theWrappedObject) const
{
  return ( theWrappedObject->type());
}

qreal  PythonQtWrapper_QTextLength::value(QTextLength* theWrappedObject, qreal  maximumLength) const
{
  return ( theWrappedObject->value(maximumLength));
}


