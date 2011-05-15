#include <PythonQt.h>
#include <QColor>
#include <QImage>
#include <QObject>
#include <QRect>
#include <QSize>
#include <QStringList>
#include <QTextFormat>
#include <QVariant>
#include <qabstractanimation.h>
#include <qabstractitemdelegate.h>
#include <qabstractitemmodel.h>
#include <qabstractitemview.h>
#include <qabstractstate.h>
#include <qaction.h>
#include <qbitmap.h>
#include <qbrush.h>
#include <qbytearray.h>
#include <qcolor.h>
#include <qcompleter.h>
#include <qcoreevent.h>
#include <qcursor.h>
#include <qdatastream.h>
#include <qdockwidget.h>
#include <qevent.h>
#include <qfont.h>
#include <qgraphicseffect.h>
#include <qgraphicsproxywidget.h>
#include <qicon.h>
#include <qiconengineplugin.h>
#include <qimage.h>
#include <qimageiohandler.h>
#include <qimagereader.h>
#include <qimagewriter.h>
#include <qinputcontext.h>
#include <qinputcontextfactory.h>
#include <qinputcontextplugin.h>
#include <qinputdialog.h>
#include <qiodevice.h>
#include <qitemdelegate.h>
#include <qitemeditorfactory.h>
#include <qitemselectionmodel.h>
#include <qkeyeventtransition.h>
#include <qkeysequence.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlayoutitem.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qlist.h>
#include <qlistview.h>
#include <qlistwidget.h>
#include <qlocale.h>
#include <qmainwindow.h>
#include <qmargins.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qmimedata.h>
#include <qmovie.h>
#include <qobject.h>
#include <qpaintdevice.h>
#include <qpaintengine.h>
#include <qpainter.h>
#include <qpair.h>
#include <qpalette.h>
#include <qpicture.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qrect.h>
#include <qregion.h>
#include <qscrollbar.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qstate.h>
#include <qstatemachine.h>
#include <qstatusbar.h>
#include <qstringlist.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtextformat.h>
#include <qtoolbar.h>
#include <qvalidator.h>
#include <qvector.h>
#include <qwidget.h>



class PythonQtShell_QIconEnginePluginV2 : public QIconEnginePluginV2
{
public:
    PythonQtShell_QIconEnginePluginV2(QObject*  parent = 0):QIconEnginePluginV2(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual QIconEngineV2*  create(const QString&  filename = QString());
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual QStringList  keys() const;
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QIconEnginePluginV2 : public QObject
{ Q_OBJECT
public:
public slots:
QIconEnginePluginV2* new_QIconEnginePluginV2(QObject*  parent = 0);
void delete_QIconEnginePluginV2(QIconEnginePluginV2* obj) { delete obj; } 
};





class PythonQtShell_QImageIOHandler : public QImageIOHandler
{
public:
    PythonQtShell_QImageIOHandler():QImageIOHandler(),_wrapper(NULL) {};

virtual bool  canRead() const;
virtual int  currentImageNumber() const;
virtual QRect  currentImageRect() const;
virtual int  imageCount() const;
virtual bool  jumpToImage(int  imageNumber);
virtual bool  jumpToNextImage();
virtual int  loopCount() const;
virtual QByteArray  name() const;
virtual int  nextImageDelay() const;
virtual QVariant  option(QImageIOHandler::ImageOption  option) const;
virtual bool  read(QImage*  image);
virtual void setOption(QImageIOHandler::ImageOption  option, const QVariant&  value);
virtual bool  supportsOption(QImageIOHandler::ImageOption  option) const;
virtual bool  write(const QImage&  image);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QImageIOHandler : public QImageIOHandler
{ public:
inline int  promoted_currentImageNumber() const { return QImageIOHandler::currentImageNumber(); }
inline QRect  promoted_currentImageRect() const { return QImageIOHandler::currentImageRect(); }
inline int  promoted_imageCount() const { return QImageIOHandler::imageCount(); }
inline bool  promoted_jumpToImage(int  imageNumber) { return QImageIOHandler::jumpToImage(imageNumber); }
inline bool  promoted_jumpToNextImage() { return QImageIOHandler::jumpToNextImage(); }
inline int  promoted_loopCount() const { return QImageIOHandler::loopCount(); }
inline int  promoted_nextImageDelay() const { return QImageIOHandler::nextImageDelay(); }
inline QVariant  promoted_option(QImageIOHandler::ImageOption  option) const { return QImageIOHandler::option(option); }
inline void promoted_setOption(QImageIOHandler::ImageOption  option, const QVariant&  value) { QImageIOHandler::setOption(option, value); }
inline bool  promoted_supportsOption(QImageIOHandler::ImageOption  option) const { return QImageIOHandler::supportsOption(option); }
inline bool  promoted_write(const QImage&  image) { return QImageIOHandler::write(image); }
};

class PythonQtWrapper_QImageIOHandler : public QObject
{ Q_OBJECT
public:
Q_ENUMS(ImageOption )
enum ImageOption{
  Size = QImageIOHandler::Size,   ClipRect = QImageIOHandler::ClipRect,   Description = QImageIOHandler::Description,   ScaledClipRect = QImageIOHandler::ScaledClipRect,   ScaledSize = QImageIOHandler::ScaledSize,   CompressionRatio = QImageIOHandler::CompressionRatio,   Gamma = QImageIOHandler::Gamma,   Quality = QImageIOHandler::Quality,   Name = QImageIOHandler::Name,   SubType = QImageIOHandler::SubType,   IncrementalReading = QImageIOHandler::IncrementalReading,   Endianness = QImageIOHandler::Endianness,   Animation = QImageIOHandler::Animation,   BackgroundColor = QImageIOHandler::BackgroundColor,   ImageFormat = QImageIOHandler::ImageFormat};
public slots:
QImageIOHandler* new_QImageIOHandler();
void delete_QImageIOHandler(QImageIOHandler* obj) { delete obj; } 
   int  currentImageNumber(QImageIOHandler* theWrappedObject) const;
   QRect  currentImageRect(QImageIOHandler* theWrappedObject) const;
   QIODevice*  device(QImageIOHandler* theWrappedObject) const;
   QByteArray  format(QImageIOHandler* theWrappedObject) const;
   int  imageCount(QImageIOHandler* theWrappedObject) const;
   bool  jumpToImage(QImageIOHandler* theWrappedObject, int  imageNumber);
   bool  jumpToNextImage(QImageIOHandler* theWrappedObject);
   int  loopCount(QImageIOHandler* theWrappedObject) const;
   int  nextImageDelay(QImageIOHandler* theWrappedObject) const;
   QVariant  option(QImageIOHandler* theWrappedObject, QImageIOHandler::ImageOption  option) const;
   void setDevice(QImageIOHandler* theWrappedObject, QIODevice*  device);
   void setFormat(QImageIOHandler* theWrappedObject, const QByteArray&  format);
   void setOption(QImageIOHandler* theWrappedObject, QImageIOHandler::ImageOption  option, const QVariant&  value);
   bool  supportsOption(QImageIOHandler* theWrappedObject, QImageIOHandler::ImageOption  option) const;
   bool  write(QImageIOHandler* theWrappedObject, const QImage&  image);
};





class PythonQtShell_QImageIOPlugin : public QImageIOPlugin
{
public:
    PythonQtShell_QImageIOPlugin(QObject*  parent = 0):QImageIOPlugin(parent),_wrapper(NULL) {};

virtual QImageIOPlugin::Capabilities  capabilities(QIODevice*  device, const QByteArray&  format) const;
virtual void childEvent(QChildEvent*  arg__1);
virtual QImageIOHandler*  create(QIODevice*  device, const QByteArray&  format = QByteArray()) const;
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual QStringList  keys() const;
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QImageIOPlugin : public QObject
{ Q_OBJECT
public:
Q_ENUMS(Capability )
Q_FLAGS(Capabilities )
enum Capability{
  CanRead = QImageIOPlugin::CanRead,   CanWrite = QImageIOPlugin::CanWrite,   CanReadIncremental = QImageIOPlugin::CanReadIncremental};
Q_DECLARE_FLAGS(Capabilities, Capability)
public slots:
QImageIOPlugin* new_QImageIOPlugin(QObject*  parent = 0);
void delete_QImageIOPlugin(QImageIOPlugin* obj) { delete obj; } 
};





class PythonQtWrapper_QImageReader : public QObject
{ Q_OBJECT
public:
Q_ENUMS(ImageReaderError )
enum ImageReaderError{
  UnknownError = QImageReader::UnknownError,   FileNotFoundError = QImageReader::FileNotFoundError,   DeviceError = QImageReader::DeviceError,   UnsupportedFormatError = QImageReader::UnsupportedFormatError,   InvalidDataError = QImageReader::InvalidDataError};
public slots:
QImageReader* new_QImageReader();
QImageReader* new_QImageReader(QIODevice*  device, const QByteArray&  format = QByteArray());
QImageReader* new_QImageReader(const QString&  fileName, const QByteArray&  format = QByteArray());
void delete_QImageReader(QImageReader* obj) { delete obj; } 
   bool  autoDetectImageFormat(QImageReader* theWrappedObject) const;
   QColor  backgroundColor(QImageReader* theWrappedObject) const;
   bool  canRead(QImageReader* theWrappedObject) const;
   QRect  clipRect(QImageReader* theWrappedObject) const;
   int  currentImageNumber(QImageReader* theWrappedObject) const;
   QRect  currentImageRect(QImageReader* theWrappedObject) const;
   bool  decideFormatFromContent(QImageReader* theWrappedObject) const;
   QIODevice*  device(QImageReader* theWrappedObject) const;
   QImageReader::ImageReaderError  error(QImageReader* theWrappedObject) const;
   QString  errorString(QImageReader* theWrappedObject) const;
   QString  fileName(QImageReader* theWrappedObject) const;
   QByteArray  format(QImageReader* theWrappedObject) const;
   int  imageCount(QImageReader* theWrappedObject) const;
   QImage::Format  imageFormat(QImageReader* theWrappedObject) const;
   QByteArray  static_QImageReader_imageFormat(QIODevice*  device);
   QByteArray  static_QImageReader_imageFormat(const QString&  fileName);
   bool  jumpToImage(QImageReader* theWrappedObject, int  imageNumber);
   bool  jumpToNextImage(QImageReader* theWrappedObject);
   int  loopCount(QImageReader* theWrappedObject) const;
   int  nextImageDelay(QImageReader* theWrappedObject) const;
   int  quality(QImageReader* theWrappedObject) const;
   QImage  read(QImageReader* theWrappedObject);
   QRect  scaledClipRect(QImageReader* theWrappedObject) const;
   QSize  scaledSize(QImageReader* theWrappedObject) const;
   void setAutoDetectImageFormat(QImageReader* theWrappedObject, bool  enabled);
   void setBackgroundColor(QImageReader* theWrappedObject, const QColor&  color);
   void setClipRect(QImageReader* theWrappedObject, const QRect&  rect);
   void setDecideFormatFromContent(QImageReader* theWrappedObject, bool  ignored);
   void setDevice(QImageReader* theWrappedObject, QIODevice*  device);
   void setFileName(QImageReader* theWrappedObject, const QString&  fileName);
   void setFormat(QImageReader* theWrappedObject, const QByteArray&  format);
   void setQuality(QImageReader* theWrappedObject, int  quality);
   void setScaledClipRect(QImageReader* theWrappedObject, const QRect&  rect);
   void setScaledSize(QImageReader* theWrappedObject, const QSize&  size);
   QSize  size(QImageReader* theWrappedObject) const;
   QList<QByteArray >  static_QImageReader_supportedImageFormats();
   bool  supportsAnimation(QImageReader* theWrappedObject) const;
   bool  supportsOption(QImageReader* theWrappedObject, QImageIOHandler::ImageOption  option) const;
   QString  text(QImageReader* theWrappedObject, const QString&  key) const;
   QStringList  textKeys(QImageReader* theWrappedObject) const;
};





class PythonQtWrapper_QImageWriter : public QObject
{ Q_OBJECT
public:
Q_ENUMS(ImageWriterError )
enum ImageWriterError{
  UnknownError = QImageWriter::UnknownError,   DeviceError = QImageWriter::DeviceError,   UnsupportedFormatError = QImageWriter::UnsupportedFormatError};
public slots:
QImageWriter* new_QImageWriter();
QImageWriter* new_QImageWriter(QIODevice*  device, const QByteArray&  format);
QImageWriter* new_QImageWriter(const QString&  fileName, const QByteArray&  format = QByteArray());
void delete_QImageWriter(QImageWriter* obj) { delete obj; } 
   bool  canWrite(QImageWriter* theWrappedObject) const;
   int  compression(QImageWriter* theWrappedObject) const;
   QIODevice*  device(QImageWriter* theWrappedObject) const;
   QImageWriter::ImageWriterError  error(QImageWriter* theWrappedObject) const;
   QString  errorString(QImageWriter* theWrappedObject) const;
   QString  fileName(QImageWriter* theWrappedObject) const;
   QByteArray  format(QImageWriter* theWrappedObject) const;
   float  gamma(QImageWriter* theWrappedObject) const;
   int  quality(QImageWriter* theWrappedObject) const;
   void setCompression(QImageWriter* theWrappedObject, int  compression);
   void setDevice(QImageWriter* theWrappedObject, QIODevice*  device);
   void setFileName(QImageWriter* theWrappedObject, const QString&  fileName);
   void setFormat(QImageWriter* theWrappedObject, const QByteArray&  format);
   void setGamma(QImageWriter* theWrappedObject, float  gamma);
   void setQuality(QImageWriter* theWrappedObject, int  quality);
   void setText(QImageWriter* theWrappedObject, const QString&  key, const QString&  text);
   QList<QByteArray >  static_QImageWriter_supportedImageFormats();
   bool  supportsOption(QImageWriter* theWrappedObject, QImageIOHandler::ImageOption  option) const;
   bool  write(QImageWriter* theWrappedObject, const QImage&  image);
};





class PythonQtShell_QInputContext : public QInputContext
{
public:
    PythonQtShell_QInputContext(QObject*  parent = 0):QInputContext(parent),_wrapper(NULL) {};

virtual QList<QAction* >  actions();
virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual bool  filterEvent(const QEvent*  event);
virtual QFont  font() const;
virtual QString  identifierName();
virtual bool  isComposing() const;
virtual QString  language();
virtual void mouseHandler(int  x, QMouseEvent*  event);
virtual void reset();
virtual void setFocusWidget(QWidget*  w);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void update();
virtual void widgetDestroyed(QWidget*  w);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QInputContext : public QInputContext
{ public:
inline QList<QAction* >  promoted_actions() { return QInputContext::actions(); }
inline bool  promoted_filterEvent(const QEvent*  event) { return QInputContext::filterEvent(event); }
inline QFont  promoted_font() const { return QInputContext::font(); }
inline void promoted_mouseHandler(int  x, QMouseEvent*  event) { QInputContext::mouseHandler(x, event); }
inline void promoted_update() { QInputContext::update(); }
inline void promoted_widgetDestroyed(QWidget*  w) { QInputContext::widgetDestroyed(w); }
};

class PythonQtWrapper_QInputContext : public QObject
{ Q_OBJECT
public:
Q_ENUMS(StandardFormat )
enum StandardFormat{
  PreeditFormat = QInputContext::PreeditFormat,   SelectionFormat = QInputContext::SelectionFormat};
public slots:
QInputContext* new_QInputContext(QObject*  parent = 0);
void delete_QInputContext(QInputContext* obj) { delete obj; } 
   QList<QAction* >  actions(QInputContext* theWrappedObject);
   bool  filterEvent(QInputContext* theWrappedObject, const QEvent*  event);
   QWidget*  focusWidget(QInputContext* theWrappedObject) const;
   QFont  font(QInputContext* theWrappedObject) const;
   void mouseHandler(QInputContext* theWrappedObject, int  x, QMouseEvent*  event);
   void sendEvent(QInputContext* theWrappedObject, const QInputMethodEvent&  event);
   QTextFormat  standardFormat(QInputContext* theWrappedObject, QInputContext::StandardFormat  s) const;
   void update(QInputContext* theWrappedObject);
   void widgetDestroyed(QInputContext* theWrappedObject, QWidget*  w);
};





class PythonQtShell_QInputContextFactory : public QInputContextFactory
{
public:
    PythonQtShell_QInputContextFactory():QInputContextFactory(),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QInputContextFactory : public QObject
{ Q_OBJECT
public:
public slots:
QInputContextFactory* new_QInputContextFactory();
void delete_QInputContextFactory(QInputContextFactory* obj) { delete obj; } 
   QInputContext*  static_QInputContextFactory_create(const QString&  key, QObject*  parent);
   QString  static_QInputContextFactory_description(const QString&  key);
   QString  static_QInputContextFactory_displayName(const QString&  key);
   QStringList  static_QInputContextFactory_keys();
   QStringList  static_QInputContextFactory_languages(const QString&  key);
};





class PythonQtShell_QInputContextPlugin : public QInputContextPlugin
{
public:
    PythonQtShell_QInputContextPlugin(QObject*  parent = 0):QInputContextPlugin(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual QInputContext*  create(const QString&  key);
virtual void customEvent(QEvent*  arg__1);
virtual QString  description(const QString&  key);
virtual QString  displayName(const QString&  key);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual QStringList  keys() const;
virtual QStringList  languages(const QString&  key);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QInputContextPlugin : public QObject
{ Q_OBJECT
public:
public slots:
QInputContextPlugin* new_QInputContextPlugin(QObject*  parent = 0);
void delete_QInputContextPlugin(QInputContextPlugin* obj) { delete obj; } 
};





class PythonQtShell_QInputDialog : public QInputDialog
{
public:
    PythonQtShell_QInputDialog(QWidget*  parent = 0, Qt::WindowFlags  flags = 0):QInputDialog(parent, flags),_wrapper(NULL) {};

virtual void accept();
virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  arg__1);
virtual void childEvent(QChildEvent*  arg__1);
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
virtual void mouseDoubleClickEvent(QMouseEvent*  arg__1);
virtual void mouseMoveEvent(QMouseEvent*  arg__1);
virtual void mousePressEvent(QMouseEvent*  arg__1);
virtual void mouseReleaseEvent(QMouseEvent*  arg__1);
virtual void moveEvent(QMoveEvent*  arg__1);
virtual QPaintEngine*  paintEngine() const;
virtual void paintEvent(QPaintEvent*  arg__1);
virtual void reject();
virtual void resizeEvent(QResizeEvent*  arg__1);
virtual void showEvent(QShowEvent*  arg__1);
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QInputDialog : public QInputDialog
{ public:
inline void promoted_done(int  result) { QInputDialog::done(result); }
};

class PythonQtWrapper_QInputDialog : public QObject
{ Q_OBJECT
public:
Q_ENUMS(InputMode InputDialogOption )
Q_FLAGS(InputDialogOptions )
enum InputMode{
  TextInput = QInputDialog::TextInput,   IntInput = QInputDialog::IntInput,   DoubleInput = QInputDialog::DoubleInput};
enum InputDialogOption{
  NoButtons = QInputDialog::NoButtons,   UseListViewForComboBoxItems = QInputDialog::UseListViewForComboBoxItems};
Q_DECLARE_FLAGS(InputDialogOptions, InputDialogOption)
public slots:
QInputDialog* new_QInputDialog(QWidget*  parent = 0, Qt::WindowFlags  flags = 0);
void delete_QInputDialog(QInputDialog* obj) { delete obj; } 
   QString  cancelButtonText(QInputDialog* theWrappedObject) const;
   QStringList  comboBoxItems(QInputDialog* theWrappedObject) const;
   void done(QInputDialog* theWrappedObject, int  result);
   int  doubleDecimals(QInputDialog* theWrappedObject) const;
   double  doubleMaximum(QInputDialog* theWrappedObject) const;
   double  doubleMinimum(QInputDialog* theWrappedObject) const;
   double  doubleValue(QInputDialog* theWrappedObject) const;
   double  static_QInputDialog_getDouble(QWidget*  parent, const QString&  title, const QString&  label, double  value = 0, double  minValue = -2147483647, double  maxValue = 2147483647, int  decimals = 1, bool*  ok = 0, Qt::WindowFlags  flags = 0);
   int  static_QInputDialog_getInt(QWidget*  parent, const QString&  title, const QString&  label, int  value = 0, int  minValue = -2147483647, int  maxValue = 2147483647, int  step = 1, bool*  ok = 0, Qt::WindowFlags  flags = 0);
   int  static_QInputDialog_getInteger(QWidget*  parent, const QString&  title, const QString&  label, int  value = 0, int  minValue = -2147483647, int  maxValue = 2147483647, int  step = 1, bool*  ok = 0, Qt::WindowFlags  flags = 0);
   QString  static_QInputDialog_getItem(QWidget*  parent, const QString&  title, const QString&  label, const QStringList&  items, int  current = 0, bool  editable = true, bool*  ok = 0, Qt::WindowFlags  flags = 0);
   QString  static_QInputDialog_getText(QWidget*  parent, const QString&  title, const QString&  label, QLineEdit::EchoMode  echo = QLineEdit::Normal, const QString&  text = QString(), bool*  ok = 0, Qt::WindowFlags  flags = 0);
   QInputDialog::InputMode  inputMode(QInputDialog* theWrappedObject) const;
   int  intMaximum(QInputDialog* theWrappedObject) const;
   int  intMinimum(QInputDialog* theWrappedObject) const;
   int  intStep(QInputDialog* theWrappedObject) const;
   int  intValue(QInputDialog* theWrappedObject) const;
   bool  isComboBoxEditable(QInputDialog* theWrappedObject) const;
   QString  labelText(QInputDialog* theWrappedObject) const;
   QSize  minimumSizeHint(QInputDialog* theWrappedObject) const;
   QString  okButtonText(QInputDialog* theWrappedObject) const;
   void open(QInputDialog* theWrappedObject);
   void open(QInputDialog* theWrappedObject, QObject*  receiver, const char*  member);
   QInputDialog::InputDialogOptions  options(QInputDialog* theWrappedObject) const;
   void setCancelButtonText(QInputDialog* theWrappedObject, const QString&  text);
   void setComboBoxEditable(QInputDialog* theWrappedObject, bool  editable);
   void setComboBoxItems(QInputDialog* theWrappedObject, const QStringList&  items);
   void setDoubleDecimals(QInputDialog* theWrappedObject, int  decimals);
   void setDoubleMaximum(QInputDialog* theWrappedObject, double  max);
   void setDoubleMinimum(QInputDialog* theWrappedObject, double  min);
   void setDoubleRange(QInputDialog* theWrappedObject, double  min, double  max);
   void setDoubleValue(QInputDialog* theWrappedObject, double  value);
   void setInputMode(QInputDialog* theWrappedObject, QInputDialog::InputMode  mode);
   void setIntMaximum(QInputDialog* theWrappedObject, int  max);
   void setIntMinimum(QInputDialog* theWrappedObject, int  min);
   void setIntRange(QInputDialog* theWrappedObject, int  min, int  max);
   void setIntStep(QInputDialog* theWrappedObject, int  step);
   void setIntValue(QInputDialog* theWrappedObject, int  value);
   void setLabelText(QInputDialog* theWrappedObject, const QString&  text);
   void setOkButtonText(QInputDialog* theWrappedObject, const QString&  text);
   void setOption(QInputDialog* theWrappedObject, QInputDialog::InputDialogOption  option, bool  on = true);
   void setOptions(QInputDialog* theWrappedObject, QInputDialog::InputDialogOptions  options);
   void setTextEchoMode(QInputDialog* theWrappedObject, QLineEdit::EchoMode  mode);
   void setTextValue(QInputDialog* theWrappedObject, const QString&  text);
   void setVisible(QInputDialog* theWrappedObject, bool  visible);
   QSize  sizeHint(QInputDialog* theWrappedObject) const;
   bool  testOption(QInputDialog* theWrappedObject, QInputDialog::InputDialogOption  option) const;
   QLineEdit::EchoMode  textEchoMode(QInputDialog* theWrappedObject) const;
   QString  textValue(QInputDialog* theWrappedObject) const;
};





class PythonQtShell_QInputEvent : public QInputEvent
{
public:
    PythonQtShell_QInputEvent(QEvent::Type  type, Qt::KeyboardModifiers  modifiers = Qt::NoModifier):QInputEvent(type, modifiers),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QInputEvent : public QObject
{ Q_OBJECT
public:
public slots:
QInputEvent* new_QInputEvent(QEvent::Type  type, Qt::KeyboardModifiers  modifiers = Qt::NoModifier);
void delete_QInputEvent(QInputEvent* obj) { delete obj; } 
   Qt::KeyboardModifiers  modifiers(QInputEvent* theWrappedObject) const;
   void setModifiers(QInputEvent* theWrappedObject, Qt::KeyboardModifiers  amodifiers);
};





class PythonQtShell_QIntValidator : public QIntValidator
{
public:
    PythonQtShell_QIntValidator(QObject*  parent = 0):QIntValidator(parent),_wrapper(NULL) {};
    PythonQtShell_QIntValidator(int  bottom, int  top, QObject*  parent):QIntValidator(bottom, top, parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void fixup(QString&  arg__1) const;
virtual void setRange(int  bottom, int  top);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual QValidator::State  validate(QString&  arg__1, int&  arg__2) const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QIntValidator : public QIntValidator
{ public:
inline void promoted_setRange(int  bottom, int  top) { QIntValidator::setRange(bottom, top); }
inline QValidator::State  promoted_validate(QString&  arg__1, int&  arg__2) const { return QIntValidator::validate(arg__1, arg__2); }
};

class PythonQtWrapper_QIntValidator : public QObject
{ Q_OBJECT
public:
public slots:
QIntValidator* new_QIntValidator(QObject*  parent = 0);
QIntValidator* new_QIntValidator(int  bottom, int  top, QObject*  parent);
void delete_QIntValidator(QIntValidator* obj) { delete obj; } 
   int  bottom(QIntValidator* theWrappedObject) const;
   void setBottom(QIntValidator* theWrappedObject, int  arg__1);
   void setRange(QIntValidator* theWrappedObject, int  bottom, int  top);
   void setTop(QIntValidator* theWrappedObject, int  arg__1);
   int  top(QIntValidator* theWrappedObject) const;
   QValidator::State  validate(QIntValidator* theWrappedObject, QString&  arg__1, int&  arg__2) const;
};





class PythonQtShell_QItemDelegate : public QItemDelegate
{
public:
    PythonQtShell_QItemDelegate(QObject*  parent = 0):QItemDelegate(parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual QWidget*  createEditor(QWidget*  parent, const QStyleOptionViewItem&  option, const QModelIndex&  index) const;
virtual void customEvent(QEvent*  arg__1);
virtual void drawCheck(QPainter*  painter, const QStyleOptionViewItem&  option, const QRect&  rect, Qt::CheckState  state) const;
virtual void drawDecoration(QPainter*  painter, const QStyleOptionViewItem&  option, const QRect&  rect, const QPixmap&  pixmap) const;
virtual void drawDisplay(QPainter*  painter, const QStyleOptionViewItem&  option, const QRect&  rect, const QString&  text) const;
virtual void drawFocus(QPainter*  painter, const QStyleOptionViewItem&  option, const QRect&  rect) const;
virtual bool  editorEvent(QEvent*  event, QAbstractItemModel*  model, const QStyleOptionViewItem&  option, const QModelIndex&  index);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  object, QEvent*  event);
virtual void paint(QPainter*  painter, const QStyleOptionViewItem&  option, const QModelIndex&  index) const;
virtual void setEditorData(QWidget*  editor, const QModelIndex&  index) const;
virtual void setModelData(QWidget*  editor, QAbstractItemModel*  model, const QModelIndex&  index) const;
virtual QSize  sizeHint(const QStyleOptionViewItem&  option, const QModelIndex&  index) const;
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void updateEditorGeometry(QWidget*  editor, const QStyleOptionViewItem&  option, const QModelIndex&  index) const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QItemDelegate : public QItemDelegate
{ public:
inline QWidget*  promoted_createEditor(QWidget*  parent, const QStyleOptionViewItem&  option, const QModelIndex&  index) const { return QItemDelegate::createEditor(parent, option, index); }
inline void promoted_drawCheck(QPainter*  painter, const QStyleOptionViewItem&  option, const QRect&  rect, Qt::CheckState  state) const { QItemDelegate::drawCheck(painter, option, rect, state); }
inline void promoted_drawDecoration(QPainter*  painter, const QStyleOptionViewItem&  option, const QRect&  rect, const QPixmap&  pixmap) const { QItemDelegate::drawDecoration(painter, option, rect, pixmap); }
inline void promoted_drawDisplay(QPainter*  painter, const QStyleOptionViewItem&  option, const QRect&  rect, const QString&  text) const { QItemDelegate::drawDisplay(painter, option, rect, text); }
inline void promoted_drawFocus(QPainter*  painter, const QStyleOptionViewItem&  option, const QRect&  rect) const { QItemDelegate::drawFocus(painter, option, rect); }
inline bool  promoted_editorEvent(QEvent*  event, QAbstractItemModel*  model, const QStyleOptionViewItem&  option, const QModelIndex&  index) { return QItemDelegate::editorEvent(event, model, option, index); }
inline bool  promoted_eventFilter(QObject*  object, QEvent*  event) { return QItemDelegate::eventFilter(object, event); }
inline void promoted_paint(QPainter*  painter, const QStyleOptionViewItem&  option, const QModelIndex&  index) const { QItemDelegate::paint(painter, option, index); }
inline void promoted_setEditorData(QWidget*  editor, const QModelIndex&  index) const { QItemDelegate::setEditorData(editor, index); }
inline void promoted_setModelData(QWidget*  editor, QAbstractItemModel*  model, const QModelIndex&  index) const { QItemDelegate::setModelData(editor, model, index); }
inline QSize  promoted_sizeHint(const QStyleOptionViewItem&  option, const QModelIndex&  index) const { return QItemDelegate::sizeHint(option, index); }
inline void promoted_updateEditorGeometry(QWidget*  editor, const QStyleOptionViewItem&  option, const QModelIndex&  index) const { QItemDelegate::updateEditorGeometry(editor, option, index); }
};

class PythonQtWrapper_QItemDelegate : public QObject
{ Q_OBJECT
public:
public slots:
QItemDelegate* new_QItemDelegate(QObject*  parent = 0);
void delete_QItemDelegate(QItemDelegate* obj) { delete obj; } 
   QWidget*  createEditor(QItemDelegate* theWrappedObject, QWidget*  parent, const QStyleOptionViewItem&  option, const QModelIndex&  index) const;
   void drawCheck(QItemDelegate* theWrappedObject, QPainter*  painter, const QStyleOptionViewItem&  option, const QRect&  rect, Qt::CheckState  state) const;
   void drawDecoration(QItemDelegate* theWrappedObject, QPainter*  painter, const QStyleOptionViewItem&  option, const QRect&  rect, const QPixmap&  pixmap) const;
   void drawDisplay(QItemDelegate* theWrappedObject, QPainter*  painter, const QStyleOptionViewItem&  option, const QRect&  rect, const QString&  text) const;
   void drawFocus(QItemDelegate* theWrappedObject, QPainter*  painter, const QStyleOptionViewItem&  option, const QRect&  rect) const;
   bool  editorEvent(QItemDelegate* theWrappedObject, QEvent*  event, QAbstractItemModel*  model, const QStyleOptionViewItem&  option, const QModelIndex&  index);
   bool  eventFilter(QItemDelegate* theWrappedObject, QObject*  object, QEvent*  event);
   bool  hasClipping(QItemDelegate* theWrappedObject) const;
   QItemEditorFactory*  itemEditorFactory(QItemDelegate* theWrappedObject) const;
   void paint(QItemDelegate* theWrappedObject, QPainter*  painter, const QStyleOptionViewItem&  option, const QModelIndex&  index) const;
   void setClipping(QItemDelegate* theWrappedObject, bool  clip);
   void setEditorData(QItemDelegate* theWrappedObject, QWidget*  editor, const QModelIndex&  index) const;
   void setItemEditorFactory(QItemDelegate* theWrappedObject, QItemEditorFactory*  factory);
   void setModelData(QItemDelegate* theWrappedObject, QWidget*  editor, QAbstractItemModel*  model, const QModelIndex&  index) const;
   QSize  sizeHint(QItemDelegate* theWrappedObject, const QStyleOptionViewItem&  option, const QModelIndex&  index) const;
   void updateEditorGeometry(QItemDelegate* theWrappedObject, QWidget*  editor, const QStyleOptionViewItem&  option, const QModelIndex&  index) const;
};





class PythonQtShell_QItemEditorCreatorBase : public QItemEditorCreatorBase
{
public:
    PythonQtShell_QItemEditorCreatorBase():QItemEditorCreatorBase(),_wrapper(NULL) {};

virtual QWidget*  createWidget(QWidget*  parent) const;
virtual QByteArray  valuePropertyName() const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QItemEditorCreatorBase : public QObject
{ Q_OBJECT
public:
public slots:
QItemEditorCreatorBase* new_QItemEditorCreatorBase();
void delete_QItemEditorCreatorBase(QItemEditorCreatorBase* obj) { delete obj; } 
};





class PythonQtShell_QItemEditorFactory : public QItemEditorFactory
{
public:
    PythonQtShell_QItemEditorFactory():QItemEditorFactory(),_wrapper(NULL) {};

virtual QWidget*  createEditor(QVariant::Type  type, QWidget*  parent) const;
virtual QByteArray  valuePropertyName(QVariant::Type  type) const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QItemEditorFactory : public QItemEditorFactory
{ public:
inline QWidget*  promoted_createEditor(QVariant::Type  type, QWidget*  parent) const { return QItemEditorFactory::createEditor(type, parent); }
inline QByteArray  promoted_valuePropertyName(QVariant::Type  type) const { return QItemEditorFactory::valuePropertyName(type); }
};

class PythonQtWrapper_QItemEditorFactory : public QObject
{ Q_OBJECT
public:
public slots:
QItemEditorFactory* new_QItemEditorFactory();
void delete_QItemEditorFactory(QItemEditorFactory* obj) { delete obj; } 
   QWidget*  createEditor(QItemEditorFactory* theWrappedObject, QVariant::Type  type, QWidget*  parent) const;
   const QItemEditorFactory*  static_QItemEditorFactory_defaultFactory();
   void registerEditor(QItemEditorFactory* theWrappedObject, QVariant::Type  type, QItemEditorCreatorBase*  creator);
   void static_QItemEditorFactory_setDefaultFactory(QItemEditorFactory*  factory);
   QByteArray  valuePropertyName(QItemEditorFactory* theWrappedObject, QVariant::Type  type) const;
};





class PythonQtWrapper_QItemSelection : public QObject
{ Q_OBJECT
public:
public slots:
QItemSelection* new_QItemSelection();
QItemSelection* new_QItemSelection(const QModelIndex&  topLeft, const QModelIndex&  bottomRight);
QItemSelection* new_QItemSelection(const QItemSelection& other) {
QItemSelection* a = new QItemSelection();
*((QItemSelection*)a) = other;
return a; }
void delete_QItemSelection(QItemSelection* obj) { delete obj; } 
   void append(QItemSelection* theWrappedObject, const QItemSelectionRange&  t);
   void append(QItemSelection* theWrappedObject, const QList<QItemSelectionRange >&  t);
   const QItemSelectionRange*  at(QItemSelection* theWrappedObject, int  i) const;
   const QItemSelectionRange*  back(QItemSelection* theWrappedObject) const;
   void clear(QItemSelection* theWrappedObject);
   bool  contains(QItemSelection* theWrappedObject, const QModelIndex&  index) const;
   int  count(QItemSelection* theWrappedObject) const;
   int  count(QItemSelection* theWrappedObject, const QItemSelectionRange&  t) const;
   void detachShared(QItemSelection* theWrappedObject);
   bool  empty(QItemSelection* theWrappedObject) const;
   bool  endsWith(QItemSelection* theWrappedObject, const QItemSelectionRange&  t) const;
   const QItemSelectionRange*  first(QItemSelection* theWrappedObject) const;
   QList<QItemSelectionRange >  static_QItemSelection_fromVector(const QVector<QItemSelectionRange >&  vector);
   const QItemSelectionRange*  front(QItemSelection* theWrappedObject) const;
   int  indexOf(QItemSelection* theWrappedObject, const QItemSelectionRange&  t, int  from) const;
   QList<QModelIndex >  indexes(QItemSelection* theWrappedObject) const;
   bool  isEmpty(QItemSelection* theWrappedObject) const;
   const QItemSelectionRange*  last(QItemSelection* theWrappedObject) const;
   int  lastIndexOf(QItemSelection* theWrappedObject, const QItemSelectionRange&  t, int  from) const;
   int  length(QItemSelection* theWrappedObject) const;
   void merge(QItemSelection* theWrappedObject, const QItemSelection&  other, QItemSelectionModel::SelectionFlags  command);
   QList<QItemSelectionRange >  mid(QItemSelection* theWrappedObject, int  pos, int  length) const;
   void move(QItemSelection* theWrappedObject, int  from, int  to);
   bool  __ne__(QItemSelection* theWrappedObject, const QList<QItemSelectionRange >&  l) const;
   bool  __eq__(QItemSelection* theWrappedObject, const QList<QItemSelectionRange >&  l) const;
   void pop_back(QItemSelection* theWrappedObject);
   void pop_front(QItemSelection* theWrappedObject);
   void prepend(QItemSelection* theWrappedObject, const QItemSelectionRange&  t);
   void push_back(QItemSelection* theWrappedObject, const QItemSelectionRange&  t);
   void push_front(QItemSelection* theWrappedObject, const QItemSelectionRange&  t);
   int  removeAll(QItemSelection* theWrappedObject, const QItemSelectionRange&  t);
   void removeAt(QItemSelection* theWrappedObject, int  i);
   void removeFirst(QItemSelection* theWrappedObject);
   void removeLast(QItemSelection* theWrappedObject);
   bool  removeOne(QItemSelection* theWrappedObject, const QItemSelectionRange&  t);
   void replace(QItemSelection* theWrappedObject, int  i, const QItemSelectionRange&  t);
   void select(QItemSelection* theWrappedObject, const QModelIndex&  topLeft, const QModelIndex&  bottomRight);
   void setSharable(QItemSelection* theWrappedObject, bool  sharable);
   int  size(QItemSelection* theWrappedObject) const;
   void static_QItemSelection_split(const QItemSelectionRange&  range, const QItemSelectionRange&  other, QItemSelection*  result);
   bool  startsWith(QItemSelection* theWrappedObject, const QItemSelectionRange&  t) const;
   void swap(QItemSelection* theWrappedObject, int  i, int  j);
   QItemSelectionRange  takeAt(QItemSelection* theWrappedObject, int  i);
   QItemSelectionRange  takeFirst(QItemSelection* theWrappedObject);
   QItemSelectionRange  takeLast(QItemSelection* theWrappedObject);
   QVector<QItemSelectionRange >  toVector(QItemSelection* theWrappedObject) const;
   QItemSelectionRange  value(QItemSelection* theWrappedObject, int  i) const;
   QItemSelectionRange  value(QItemSelection* theWrappedObject, int  i, const QItemSelectionRange&  defaultValue) const;
};





class PythonQtShell_QItemSelectionModel : public QItemSelectionModel
{
public:
    PythonQtShell_QItemSelectionModel(QAbstractItemModel*  model):QItemSelectionModel(model),_wrapper(NULL) {};
    PythonQtShell_QItemSelectionModel(QAbstractItemModel*  model, QObject*  parent):QItemSelectionModel(model, parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void clear();
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void reset();
virtual void select(const QItemSelection&  selection, QItemSelectionModel::SelectionFlags  command);
virtual void select(const QModelIndex&  index, QItemSelectionModel::SelectionFlags  command);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QItemSelectionModel : public QItemSelectionModel
{ public:
inline void promoted_clear() { QItemSelectionModel::clear(); }
inline void promoted_reset() { QItemSelectionModel::reset(); }
inline void promoted_select(const QItemSelection&  selection, QItemSelectionModel::SelectionFlags  command) { QItemSelectionModel::select(selection, command); }
inline void promoted_select(const QModelIndex&  index, QItemSelectionModel::SelectionFlags  command) { QItemSelectionModel::select(index, command); }
};

class PythonQtWrapper_QItemSelectionModel : public QObject
{ Q_OBJECT
public:
Q_ENUMS(SelectionFlag )
Q_FLAGS(SelectionFlags )
enum SelectionFlag{
  NoUpdate = QItemSelectionModel::NoUpdate,   Clear = QItemSelectionModel::Clear,   Select = QItemSelectionModel::Select,   Deselect = QItemSelectionModel::Deselect,   Toggle = QItemSelectionModel::Toggle,   Current = QItemSelectionModel::Current,   Rows = QItemSelectionModel::Rows,   Columns = QItemSelectionModel::Columns,   SelectCurrent = QItemSelectionModel::SelectCurrent,   ToggleCurrent = QItemSelectionModel::ToggleCurrent,   ClearAndSelect = QItemSelectionModel::ClearAndSelect};
Q_DECLARE_FLAGS(SelectionFlags, SelectionFlag)
public slots:
QItemSelectionModel* new_QItemSelectionModel(QAbstractItemModel*  model);
QItemSelectionModel* new_QItemSelectionModel(QAbstractItemModel*  model, QObject*  parent);
void delete_QItemSelectionModel(QItemSelectionModel* obj) { delete obj; } 
   bool  columnIntersectsSelection(QItemSelectionModel* theWrappedObject, int  column, const QModelIndex&  parent) const;
   QModelIndex  currentIndex(QItemSelectionModel* theWrappedObject) const;
   bool  hasSelection(QItemSelectionModel* theWrappedObject) const;
   bool  isColumnSelected(QItemSelectionModel* theWrappedObject, int  column, const QModelIndex&  parent) const;
   bool  isRowSelected(QItemSelectionModel* theWrappedObject, int  row, const QModelIndex&  parent) const;
   bool  isSelected(QItemSelectionModel* theWrappedObject, const QModelIndex&  index) const;
   const QAbstractItemModel*  model(QItemSelectionModel* theWrappedObject) const;
   bool  rowIntersectsSelection(QItemSelectionModel* theWrappedObject, int  row, const QModelIndex&  parent) const;
   QList<QModelIndex >  selectedColumns(QItemSelectionModel* theWrappedObject, int  row = 0) const;
   QList<QModelIndex >  selectedIndexes(QItemSelectionModel* theWrappedObject) const;
   QList<QModelIndex >  selectedRows(QItemSelectionModel* theWrappedObject, int  column = 0) const;
   const QItemSelection  selection(QItemSelectionModel* theWrappedObject) const;
};





class PythonQtWrapper_QItemSelectionRange : public QObject
{ Q_OBJECT
public:
public slots:
QItemSelectionRange* new_QItemSelectionRange();
QItemSelectionRange* new_QItemSelectionRange(const QItemSelectionRange&  other);
QItemSelectionRange* new_QItemSelectionRange(const QModelIndex&  index);
QItemSelectionRange* new_QItemSelectionRange(const QModelIndex&  topLeft, const QModelIndex&  bottomRight);
void delete_QItemSelectionRange(QItemSelectionRange* obj) { delete obj; } 
   int  bottom(QItemSelectionRange* theWrappedObject) const;
   QModelIndex  bottomRight(QItemSelectionRange* theWrappedObject) const;
   bool  contains(QItemSelectionRange* theWrappedObject, const QModelIndex&  index) const;
   bool  contains(QItemSelectionRange* theWrappedObject, int  row, int  column, const QModelIndex&  parentIndex) const;
   int  height(QItemSelectionRange* theWrappedObject) const;
   QList<QModelIndex >  indexes(QItemSelectionRange* theWrappedObject) const;
   QItemSelectionRange  intersected(QItemSelectionRange* theWrappedObject, const QItemSelectionRange&  other) const;
   bool  intersects(QItemSelectionRange* theWrappedObject, const QItemSelectionRange&  other) const;
   bool  isValid(QItemSelectionRange* theWrappedObject) const;
   int  left(QItemSelectionRange* theWrappedObject) const;
   const QAbstractItemModel*  model(QItemSelectionRange* theWrappedObject) const;
   bool  __ne__(QItemSelectionRange* theWrappedObject, const QItemSelectionRange&  other) const;
   bool  __eq__(QItemSelectionRange* theWrappedObject, const QItemSelectionRange&  other) const;
   QModelIndex  parent(QItemSelectionRange* theWrappedObject) const;
   int  right(QItemSelectionRange* theWrappedObject) const;
   int  top(QItemSelectionRange* theWrappedObject) const;
   QModelIndex  topLeft(QItemSelectionRange* theWrappedObject) const;
   int  width(QItemSelectionRange* theWrappedObject) const;
    QString py_toString(QItemSelectionRange*);
};





class PythonQtShell_QKeyEvent : public QKeyEvent
{
public:
    PythonQtShell_QKeyEvent(QEvent::Type  type, int  key, Qt::KeyboardModifiers  modifiers, const QString&  text = QString(), bool  autorep = false, ushort  count = 1):QKeyEvent(type, key, modifiers, text, autorep, count),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QKeyEvent : public QObject
{ Q_OBJECT
public:
public slots:
QKeyEvent* new_QKeyEvent(QEvent::Type  type, int  key, Qt::KeyboardModifiers  modifiers, const QString&  text = QString(), bool  autorep = false, ushort  count = 1);
void delete_QKeyEvent(QKeyEvent* obj) { delete obj; } 
   int  count(QKeyEvent* theWrappedObject) const;
   QKeyEvent*  static_QKeyEvent_createExtendedKeyEvent(QEvent::Type  type, int  key, Qt::KeyboardModifiers  modifiers, unsigned int  nativeScanCode, unsigned int  nativeVirtualKey, unsigned int  nativeModifiers, const QString&  text = QString(), bool  autorep = false, ushort  count = 1);
   bool  hasExtendedInfo(QKeyEvent* theWrappedObject) const;
   bool  isAutoRepeat(QKeyEvent* theWrappedObject) const;
   int  key(QKeyEvent* theWrappedObject) const;
   bool  matches(QKeyEvent* theWrappedObject, QKeySequence::StandardKey  key) const;
   Qt::KeyboardModifiers  modifiers(QKeyEvent* theWrappedObject) const;
   unsigned int  nativeModifiers(QKeyEvent* theWrappedObject) const;
   unsigned int  nativeScanCode(QKeyEvent* theWrappedObject) const;
   unsigned int  nativeVirtualKey(QKeyEvent* theWrappedObject) const;
   QString  text(QKeyEvent* theWrappedObject) const;
};





class PythonQtShell_QKeyEventTransition : public QKeyEventTransition
{
public:
    PythonQtShell_QKeyEventTransition(QObject*  object, QEvent::Type  type, int  key, QState*  sourceState = 0):QKeyEventTransition(object, type, key, sourceState),_wrapper(NULL) {};
    PythonQtShell_QKeyEventTransition(QState*  sourceState = 0):QKeyEventTransition(sourceState),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  e);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual bool  eventTest(QEvent*  event);
virtual void onTransition(QEvent*  event);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QKeyEventTransition : public QKeyEventTransition
{ public:
inline bool  promoted_eventTest(QEvent*  event) { return QKeyEventTransition::eventTest(event); }
inline void promoted_onTransition(QEvent*  event) { QKeyEventTransition::onTransition(event); }
};

class PythonQtWrapper_QKeyEventTransition : public QObject
{ Q_OBJECT
public:
public slots:
QKeyEventTransition* new_QKeyEventTransition(QObject*  object, QEvent::Type  type, int  key, QState*  sourceState = 0);
QKeyEventTransition* new_QKeyEventTransition(QState*  sourceState = 0);
void delete_QKeyEventTransition(QKeyEventTransition* obj) { delete obj; } 
   bool  eventTest(QKeyEventTransition* theWrappedObject, QEvent*  event);
   int  key(QKeyEventTransition* theWrappedObject) const;
   Qt::KeyboardModifiers  modifierMask(QKeyEventTransition* theWrappedObject) const;
   void onTransition(QKeyEventTransition* theWrappedObject, QEvent*  event);
   void setKey(QKeyEventTransition* theWrappedObject, int  key);
   void setModifierMask(QKeyEventTransition* theWrappedObject, Qt::KeyboardModifiers  modifiers);
};





class PythonQtShell_QLCDNumber : public QLCDNumber
{
public:
    PythonQtShell_QLCDNumber(QWidget*  parent = 0):QLCDNumber(parent),_wrapper(NULL) {};
    PythonQtShell_QLCDNumber(uint  numDigits, QWidget*  parent = 0):QLCDNumber(numDigits, parent),_wrapper(NULL) {};

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

class PythonQtPublicPromoter_QLCDNumber : public QLCDNumber
{ public:
inline bool  promoted_event(QEvent*  e) { return QLCDNumber::event(e); }
inline void promoted_paintEvent(QPaintEvent*  arg__1) { QLCDNumber::paintEvent(arg__1); }
};

class PythonQtWrapper_QLCDNumber : public QObject
{ Q_OBJECT
public:
public slots:
QLCDNumber* new_QLCDNumber(QWidget*  parent = 0);
QLCDNumber* new_QLCDNumber(uint  numDigits, QWidget*  parent = 0);
void delete_QLCDNumber(QLCDNumber* obj) { delete obj; } 
   bool  checkOverflow(QLCDNumber* theWrappedObject, double  num) const;
   bool  checkOverflow(QLCDNumber* theWrappedObject, int  num) const;
   int  digitCount(QLCDNumber* theWrappedObject) const;
   bool  event(QLCDNumber* theWrappedObject, QEvent*  e);
   int  intValue(QLCDNumber* theWrappedObject) const;
   QLCDNumber::Mode  mode(QLCDNumber* theWrappedObject) const;
   int  numDigits(QLCDNumber* theWrappedObject) const;
   void paintEvent(QLCDNumber* theWrappedObject, QPaintEvent*  arg__1);
   QLCDNumber::SegmentStyle  segmentStyle(QLCDNumber* theWrappedObject) const;
   void setDigitCount(QLCDNumber* theWrappedObject, int  nDigits);
   void setMode(QLCDNumber* theWrappedObject, QLCDNumber::Mode  arg__1);
   void setNumDigits(QLCDNumber* theWrappedObject, int  nDigits);
   void setSegmentStyle(QLCDNumber* theWrappedObject, QLCDNumber::SegmentStyle  arg__1);
   QSize  sizeHint(QLCDNumber* theWrappedObject) const;
   bool  smallDecimalPoint(QLCDNumber* theWrappedObject) const;
   double  value(QLCDNumber* theWrappedObject) const;
};





class PythonQtShell_QLabel : public QLabel
{
public:
    PythonQtShell_QLabel(QWidget*  parent = 0, Qt::WindowFlags  f = 0):QLabel(parent, f),_wrapper(NULL) {};
    PythonQtShell_QLabel(const QString&  text, QWidget*  parent = 0, Qt::WindowFlags  f = 0):QLabel(text, parent, f),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  arg__1);
virtual void childEvent(QChildEvent*  arg__1);
virtual void closeEvent(QCloseEvent*  arg__1);
virtual void contextMenuEvent(QContextMenuEvent*  ev);
virtual void customEvent(QEvent*  arg__1);
virtual int  devType() const;
virtual void dragEnterEvent(QDragEnterEvent*  arg__1);
virtual void dragLeaveEvent(QDragLeaveEvent*  arg__1);
virtual void dragMoveEvent(QDragMoveEvent*  arg__1);
virtual void dropEvent(QDropEvent*  arg__1);
virtual void enterEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  e);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void focusInEvent(QFocusEvent*  ev);
virtual bool  focusNextPrevChild(bool  next);
virtual void focusOutEvent(QFocusEvent*  ev);
virtual int  heightForWidth(int  arg__1) const;
virtual void hideEvent(QHideEvent*  arg__1);
virtual void inputMethodEvent(QInputMethodEvent*  arg__1);
virtual QVariant  inputMethodQuery(Qt::InputMethodQuery  arg__1) const;
virtual void keyPressEvent(QKeyEvent*  ev);
virtual void keyReleaseEvent(QKeyEvent*  arg__1);
virtual void languageChange();
virtual void leaveEvent(QEvent*  arg__1);
virtual int  metric(QPaintDevice::PaintDeviceMetric  arg__1) const;
virtual void mouseDoubleClickEvent(QMouseEvent*  arg__1);
virtual void mouseMoveEvent(QMouseEvent*  ev);
virtual void mousePressEvent(QMouseEvent*  ev);
virtual void mouseReleaseEvent(QMouseEvent*  ev);
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

class PythonQtPublicPromoter_QLabel : public QLabel
{ public:
inline void promoted_changeEvent(QEvent*  arg__1) { QLabel::changeEvent(arg__1); }
inline void promoted_contextMenuEvent(QContextMenuEvent*  ev) { QLabel::contextMenuEvent(ev); }
inline bool  promoted_event(QEvent*  e) { return QLabel::event(e); }
inline void promoted_focusInEvent(QFocusEvent*  ev) { QLabel::focusInEvent(ev); }
inline bool  promoted_focusNextPrevChild(bool  next) { return QLabel::focusNextPrevChild(next); }
inline void promoted_focusOutEvent(QFocusEvent*  ev) { QLabel::focusOutEvent(ev); }
inline int  promoted_heightForWidth(int  arg__1) const { return QLabel::heightForWidth(arg__1); }
inline void promoted_keyPressEvent(QKeyEvent*  ev) { QLabel::keyPressEvent(ev); }
inline void promoted_mouseMoveEvent(QMouseEvent*  ev) { QLabel::mouseMoveEvent(ev); }
inline void promoted_mousePressEvent(QMouseEvent*  ev) { QLabel::mousePressEvent(ev); }
inline void promoted_mouseReleaseEvent(QMouseEvent*  ev) { QLabel::mouseReleaseEvent(ev); }
inline void promoted_paintEvent(QPaintEvent*  arg__1) { QLabel::paintEvent(arg__1); }
};

class PythonQtWrapper_QLabel : public QObject
{ Q_OBJECT
public:
public slots:
QLabel* new_QLabel(QWidget*  parent = 0, Qt::WindowFlags  f = 0);
QLabel* new_QLabel(const QString&  text, QWidget*  parent = 0, Qt::WindowFlags  f = 0);
void delete_QLabel(QLabel* obj) { delete obj; } 
   Qt::Alignment  alignment(QLabel* theWrappedObject) const;
   QWidget*  buddy(QLabel* theWrappedObject) const;
   void changeEvent(QLabel* theWrappedObject, QEvent*  arg__1);
   void contextMenuEvent(QLabel* theWrappedObject, QContextMenuEvent*  ev);
   bool  event(QLabel* theWrappedObject, QEvent*  e);
   void focusInEvent(QLabel* theWrappedObject, QFocusEvent*  ev);
   bool  focusNextPrevChild(QLabel* theWrappedObject, bool  next);
   void focusOutEvent(QLabel* theWrappedObject, QFocusEvent*  ev);
   bool  hasScaledContents(QLabel* theWrappedObject) const;
   int  heightForWidth(QLabel* theWrappedObject, int  arg__1) const;
   int  indent(QLabel* theWrappedObject) const;
   void keyPressEvent(QLabel* theWrappedObject, QKeyEvent*  ev);
   int  margin(QLabel* theWrappedObject) const;
   QSize  minimumSizeHint(QLabel* theWrappedObject) const;
   void mouseMoveEvent(QLabel* theWrappedObject, QMouseEvent*  ev);
   void mousePressEvent(QLabel* theWrappedObject, QMouseEvent*  ev);
   void mouseReleaseEvent(QLabel* theWrappedObject, QMouseEvent*  ev);
   QMovie*  movie(QLabel* theWrappedObject) const;
   bool  openExternalLinks(QLabel* theWrappedObject) const;
   void paintEvent(QLabel* theWrappedObject, QPaintEvent*  arg__1);
   const QPicture*  picture(QLabel* theWrappedObject) const;
   const QPixmap*  pixmap(QLabel* theWrappedObject) const;
   void setAlignment(QLabel* theWrappedObject, Qt::Alignment  arg__1);
   void setBuddy(QLabel* theWrappedObject, QWidget*  arg__1);
   void setIndent(QLabel* theWrappedObject, int  arg__1);
   void setMargin(QLabel* theWrappedObject, int  arg__1);
   void setOpenExternalLinks(QLabel* theWrappedObject, bool  open);
   void setScaledContents(QLabel* theWrappedObject, bool  arg__1);
   void setTextFormat(QLabel* theWrappedObject, Qt::TextFormat  arg__1);
   void setTextInteractionFlags(QLabel* theWrappedObject, Qt::TextInteractionFlags  flags);
   void setWordWrap(QLabel* theWrappedObject, bool  on);
   QSize  sizeHint(QLabel* theWrappedObject) const;
   QString  text(QLabel* theWrappedObject) const;
   Qt::TextFormat  textFormat(QLabel* theWrappedObject) const;
   Qt::TextInteractionFlags  textInteractionFlags(QLabel* theWrappedObject) const;
   bool  wordWrap(QLabel* theWrappedObject) const;
};





class PythonQtShell_QLayout : public QLayout
{
public:
    PythonQtShell_QLayout():QLayout(),_wrapper(NULL) {};
    PythonQtShell_QLayout(QWidget*  parent):QLayout(parent),_wrapper(NULL) {};

virtual void addItem(QLayoutItem*  arg__1);
virtual void childEvent(QChildEvent*  e);
virtual int  count() const;
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual Qt::Orientations  expandingDirections() const;
virtual QRect  geometry() const;
virtual bool  hasHeightForWidth() const;
virtual int  heightForWidth(int  arg__1) const;
virtual int  indexOf(QWidget*  arg__1) const;
virtual void invalidate();
virtual bool  isEmpty() const;
virtual QLayoutItem*  itemAt(int  index) const;
virtual QLayout*  layout();
virtual QSize  maximumSize() const;
virtual int  minimumHeightForWidth(int  arg__1) const;
virtual QSize  minimumSize() const;
virtual void setGeometry(const QRect&  arg__1);
virtual QSize  sizeHint() const;
virtual QSpacerItem*  spacerItem();
virtual QLayoutItem*  takeAt(int  index);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual QWidget*  widget();

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QLayout : public QLayout
{ public:
inline void promoted_childEvent(QChildEvent*  e) { QLayout::childEvent(e); }
inline Qt::Orientations  promoted_expandingDirections() const { return QLayout::expandingDirections(); }
inline QRect  promoted_geometry() const { return QLayout::geometry(); }
inline int  promoted_indexOf(QWidget*  arg__1) const { return QLayout::indexOf(arg__1); }
inline void promoted_invalidate() { QLayout::invalidate(); }
inline bool  promoted_isEmpty() const { return QLayout::isEmpty(); }
inline QLayout*  promoted_layout() { return QLayout::layout(); }
inline QSize  promoted_maximumSize() const { return QLayout::maximumSize(); }
inline QSize  promoted_minimumSize() const { return QLayout::minimumSize(); }
inline void promoted_setGeometry(const QRect&  arg__1) { QLayout::setGeometry(arg__1); }
};

class PythonQtWrapper_QLayout : public QObject
{ Q_OBJECT
public:
public slots:
QLayout* new_QLayout();
QLayout* new_QLayout(QWidget*  parent);
void delete_QLayout(QLayout* obj) { delete obj; } 
   bool  activate(QLayout* theWrappedObject);
   void addWidget(QLayout* theWrappedObject, QWidget*  w);
   void childEvent(QLayout* theWrappedObject, QChildEvent*  e);
   QSize  static_QLayout_closestAcceptableSize(const QWidget*  w, const QSize&  s);
   QMargins  contentsMargins(QLayout* theWrappedObject) const;
   QRect  contentsRect(QLayout* theWrappedObject) const;
   Qt::Orientations  expandingDirections(QLayout* theWrappedObject) const;
   QRect  geometry(QLayout* theWrappedObject) const;
   void getContentsMargins(QLayout* theWrappedObject, int*  left, int*  top, int*  right, int*  bottom) const;
   int  indexOf(QLayout* theWrappedObject, QWidget*  arg__1) const;
   void invalidate(QLayout* theWrappedObject);
   bool  isEmpty(QLayout* theWrappedObject) const;
   bool  isEnabled(QLayout* theWrappedObject) const;
   QLayout*  layout(QLayout* theWrappedObject);
   QSize  maximumSize(QLayout* theWrappedObject) const;
   QWidget*  menuBar(QLayout* theWrappedObject) const;
   QSize  minimumSize(QLayout* theWrappedObject) const;
   QWidget*  parentWidget(QLayout* theWrappedObject) const;
   void removeItem(QLayout* theWrappedObject, QLayoutItem*  arg__1);
   void removeWidget(QLayout* theWrappedObject, QWidget*  w);
   void setAlignment(QLayout* theWrappedObject, Qt::Alignment  alignment);
   bool  setAlignment(QLayout* theWrappedObject, QLayout*  l, Qt::Alignment  alignment);
   bool  setAlignment(QLayout* theWrappedObject, QWidget*  w, Qt::Alignment  alignment);
   void setContentsMargins(QLayout* theWrappedObject, const QMargins&  margins);
   void setContentsMargins(QLayout* theWrappedObject, int  left, int  top, int  right, int  bottom);
   void setEnabled(QLayout* theWrappedObject, bool  arg__1);
   void setGeometry(QLayout* theWrappedObject, const QRect&  arg__1);
   void setMargin(QLayout* theWrappedObject, int  arg__1);
   void setMenuBar(QLayout* theWrappedObject, QWidget*  w);
   void setSizeConstraint(QLayout* theWrappedObject, QLayout::SizeConstraint  arg__1);
   void setSpacing(QLayout* theWrappedObject, int  arg__1);
   QLayout::SizeConstraint  sizeConstraint(QLayout* theWrappedObject) const;
   int  spacing(QLayout* theWrappedObject) const;
   int  totalHeightForWidth(QLayout* theWrappedObject, int  w) const;
   QSize  totalMaximumSize(QLayout* theWrappedObject) const;
   QSize  totalMinimumSize(QLayout* theWrappedObject) const;
   QSize  totalSizeHint(QLayout* theWrappedObject) const;
   void update(QLayout* theWrappedObject);
};





class PythonQtShell_QLayoutItem : public QLayoutItem
{
public:
    PythonQtShell_QLayoutItem(Qt::Alignment  alignment = 0):QLayoutItem(alignment),_wrapper(NULL) {};

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

class PythonQtPublicPromoter_QLayoutItem : public QLayoutItem
{ public:
inline bool  promoted_hasHeightForWidth() const { return QLayoutItem::hasHeightForWidth(); }
inline int  promoted_heightForWidth(int  arg__1) const { return QLayoutItem::heightForWidth(arg__1); }
inline void promoted_invalidate() { QLayoutItem::invalidate(); }
inline QLayout*  promoted_layout() { return QLayoutItem::layout(); }
inline int  promoted_minimumHeightForWidth(int  arg__1) const { return QLayoutItem::minimumHeightForWidth(arg__1); }
inline QSpacerItem*  promoted_spacerItem() { return QLayoutItem::spacerItem(); }
inline QWidget*  promoted_widget() { return QLayoutItem::widget(); }
};

class PythonQtWrapper_QLayoutItem : public QObject
{ Q_OBJECT
public:
public slots:
QLayoutItem* new_QLayoutItem(Qt::Alignment  alignment = 0);
void delete_QLayoutItem(QLayoutItem* obj) { delete obj; } 
   Qt::Alignment  alignment(QLayoutItem* theWrappedObject) const;
   QSizePolicy::ControlTypes  controlTypes(QLayoutItem* theWrappedObject) const;
   bool  hasHeightForWidth(QLayoutItem* theWrappedObject) const;
   int  heightForWidth(QLayoutItem* theWrappedObject, int  arg__1) const;
   void invalidate(QLayoutItem* theWrappedObject);
   QLayout*  layout(QLayoutItem* theWrappedObject);
   int  minimumHeightForWidth(QLayoutItem* theWrappedObject, int  arg__1) const;
   void setAlignment(QLayoutItem* theWrappedObject, Qt::Alignment  a);
   QSpacerItem*  spacerItem(QLayoutItem* theWrappedObject);
   QWidget*  widget(QLayoutItem* theWrappedObject);
};





class PythonQtShell_QLineEdit : public QLineEdit
{
public:
    PythonQtShell_QLineEdit(QWidget*  parent = 0):QLineEdit(parent),_wrapper(NULL) {};
    PythonQtShell_QLineEdit(const QString&  arg__1, QWidget*  parent = 0):QLineEdit(arg__1, parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  arg__1);
virtual void childEvent(QChildEvent*  arg__1);
virtual void closeEvent(QCloseEvent*  arg__1);
virtual void contextMenuEvent(QContextMenuEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual int  devType() const;
virtual void dragEnterEvent(QDragEnterEvent*  arg__1);
virtual void dragLeaveEvent(QDragLeaveEvent*  e);
virtual void dragMoveEvent(QDragMoveEvent*  e);
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

class PythonQtPublicPromoter_QLineEdit : public QLineEdit
{ public:
inline void promoted_changeEvent(QEvent*  arg__1) { QLineEdit::changeEvent(arg__1); }
inline void promoted_contextMenuEvent(QContextMenuEvent*  arg__1) { QLineEdit::contextMenuEvent(arg__1); }
inline void promoted_dragEnterEvent(QDragEnterEvent*  arg__1) { QLineEdit::dragEnterEvent(arg__1); }
inline void promoted_dragLeaveEvent(QDragLeaveEvent*  e) { QLineEdit::dragLeaveEvent(e); }
inline void promoted_dragMoveEvent(QDragMoveEvent*  e) { QLineEdit::dragMoveEvent(e); }
inline void promoted_dropEvent(QDropEvent*  arg__1) { QLineEdit::dropEvent(arg__1); }
inline bool  promoted_event(QEvent*  arg__1) { return QLineEdit::event(arg__1); }
inline void promoted_focusInEvent(QFocusEvent*  arg__1) { QLineEdit::focusInEvent(arg__1); }
inline void promoted_focusOutEvent(QFocusEvent*  arg__1) { QLineEdit::focusOutEvent(arg__1); }
inline void promoted_inputMethodEvent(QInputMethodEvent*  arg__1) { QLineEdit::inputMethodEvent(arg__1); }
inline QVariant  promoted_inputMethodQuery(Qt::InputMethodQuery  arg__1) const { return QLineEdit::inputMethodQuery(arg__1); }
inline void promoted_keyPressEvent(QKeyEvent*  arg__1) { QLineEdit::keyPressEvent(arg__1); }
inline void promoted_mouseDoubleClickEvent(QMouseEvent*  arg__1) { QLineEdit::mouseDoubleClickEvent(arg__1); }
inline void promoted_mouseMoveEvent(QMouseEvent*  arg__1) { QLineEdit::mouseMoveEvent(arg__1); }
inline void promoted_mousePressEvent(QMouseEvent*  arg__1) { QLineEdit::mousePressEvent(arg__1); }
inline void promoted_mouseReleaseEvent(QMouseEvent*  arg__1) { QLineEdit::mouseReleaseEvent(arg__1); }
inline void promoted_paintEvent(QPaintEvent*  arg__1) { QLineEdit::paintEvent(arg__1); }
};

class PythonQtWrapper_QLineEdit : public QObject
{ Q_OBJECT
public:
public slots:
QLineEdit* new_QLineEdit(QWidget*  parent = 0);
QLineEdit* new_QLineEdit(const QString&  arg__1, QWidget*  parent = 0);
void delete_QLineEdit(QLineEdit* obj) { delete obj; } 
   Qt::Alignment  alignment(QLineEdit* theWrappedObject) const;
   void backspace(QLineEdit* theWrappedObject);
   void changeEvent(QLineEdit* theWrappedObject, QEvent*  arg__1);
   QCompleter*  completer(QLineEdit* theWrappedObject) const;
   void contextMenuEvent(QLineEdit* theWrappedObject, QContextMenuEvent*  arg__1);
   QMenu*  createStandardContextMenu(QLineEdit* theWrappedObject);
   void cursorBackward(QLineEdit* theWrappedObject, bool  mark, int  steps = 1);
   void cursorForward(QLineEdit* theWrappedObject, bool  mark, int  steps = 1);
   int  cursorPosition(QLineEdit* theWrappedObject) const;
   int  cursorPositionAt(QLineEdit* theWrappedObject, const QPoint&  pos);
   void cursorWordBackward(QLineEdit* theWrappedObject, bool  mark);
   void cursorWordForward(QLineEdit* theWrappedObject, bool  mark);
   void del(QLineEdit* theWrappedObject);
   void deselect(QLineEdit* theWrappedObject);
   QString  displayText(QLineEdit* theWrappedObject) const;
   bool  dragEnabled(QLineEdit* theWrappedObject) const;
   void dragEnterEvent(QLineEdit* theWrappedObject, QDragEnterEvent*  arg__1);
   void dragLeaveEvent(QLineEdit* theWrappedObject, QDragLeaveEvent*  e);
   void dragMoveEvent(QLineEdit* theWrappedObject, QDragMoveEvent*  e);
   void dropEvent(QLineEdit* theWrappedObject, QDropEvent*  arg__1);
   QLineEdit::EchoMode  echoMode(QLineEdit* theWrappedObject) const;
   void end(QLineEdit* theWrappedObject, bool  mark);
   bool  event(QLineEdit* theWrappedObject, QEvent*  arg__1);
   void focusInEvent(QLineEdit* theWrappedObject, QFocusEvent*  arg__1);
   void focusOutEvent(QLineEdit* theWrappedObject, QFocusEvent*  arg__1);
   void getTextMargins(QLineEdit* theWrappedObject, int*  left, int*  top, int*  right, int*  bottom) const;
   bool  hasAcceptableInput(QLineEdit* theWrappedObject) const;
   bool  hasFrame(QLineEdit* theWrappedObject) const;
   bool  hasSelectedText(QLineEdit* theWrappedObject) const;
   void home(QLineEdit* theWrappedObject, bool  mark);
   QString  inputMask(QLineEdit* theWrappedObject) const;
   void inputMethodEvent(QLineEdit* theWrappedObject, QInputMethodEvent*  arg__1);
   QVariant  inputMethodQuery(QLineEdit* theWrappedObject, Qt::InputMethodQuery  arg__1) const;
   void insert(QLineEdit* theWrappedObject, const QString&  arg__1);
   bool  isModified(QLineEdit* theWrappedObject) const;
   bool  isReadOnly(QLineEdit* theWrappedObject) const;
   bool  isRedoAvailable(QLineEdit* theWrappedObject) const;
   bool  isUndoAvailable(QLineEdit* theWrappedObject) const;
   void keyPressEvent(QLineEdit* theWrappedObject, QKeyEvent*  arg__1);
   int  maxLength(QLineEdit* theWrappedObject) const;
   QSize  minimumSizeHint(QLineEdit* theWrappedObject) const;
   void mouseDoubleClickEvent(QLineEdit* theWrappedObject, QMouseEvent*  arg__1);
   void mouseMoveEvent(QLineEdit* theWrappedObject, QMouseEvent*  arg__1);
   void mousePressEvent(QLineEdit* theWrappedObject, QMouseEvent*  arg__1);
   void mouseReleaseEvent(QLineEdit* theWrappedObject, QMouseEvent*  arg__1);
   void paintEvent(QLineEdit* theWrappedObject, QPaintEvent*  arg__1);
   QString  selectedText(QLineEdit* theWrappedObject) const;
   int  selectionStart(QLineEdit* theWrappedObject) const;
   void setAlignment(QLineEdit* theWrappedObject, Qt::Alignment  flag);
   void setCompleter(QLineEdit* theWrappedObject, QCompleter*  completer);
   void setCursorPosition(QLineEdit* theWrappedObject, int  arg__1);
   void setDragEnabled(QLineEdit* theWrappedObject, bool  b);
   void setEchoMode(QLineEdit* theWrappedObject, QLineEdit::EchoMode  arg__1);
   void setFrame(QLineEdit* theWrappedObject, bool  arg__1);
   void setInputMask(QLineEdit* theWrappedObject, const QString&  inputMask);
   void setMaxLength(QLineEdit* theWrappedObject, int  arg__1);
   void setModified(QLineEdit* theWrappedObject, bool  arg__1);
   void setReadOnly(QLineEdit* theWrappedObject, bool  arg__1);
   void setSelection(QLineEdit* theWrappedObject, int  arg__1, int  arg__2);
   void setTextMargins(QLineEdit* theWrappedObject, const QMargins&  margins);
   void setTextMargins(QLineEdit* theWrappedObject, int  left, int  top, int  right, int  bottom);
   void setValidator(QLineEdit* theWrappedObject, const QValidator*  arg__1);
   QSize  sizeHint(QLineEdit* theWrappedObject) const;
   QString  text(QLineEdit* theWrappedObject) const;
   QMargins  textMargins(QLineEdit* theWrappedObject) const;
   const QValidator*  validator(QLineEdit* theWrappedObject) const;
};





class PythonQtWrapper_QLinearGradient : public QObject
{ Q_OBJECT
public:
public slots:
QLinearGradient* new_QLinearGradient();
QLinearGradient* new_QLinearGradient(const QPointF&  start, const QPointF&  finalStop);
QLinearGradient* new_QLinearGradient(qreal  xStart, qreal  yStart, qreal  xFinalStop, qreal  yFinalStop);
QLinearGradient* new_QLinearGradient(const QLinearGradient& other) {
QLinearGradient* a = new QLinearGradient();
*((QLinearGradient*)a) = other;
return a; }
void delete_QLinearGradient(QLinearGradient* obj) { delete obj; } 
   QPointF  finalStop(QLinearGradient* theWrappedObject) const;
   void setFinalStop(QLinearGradient* theWrappedObject, const QPointF&  stop);
   void setFinalStop(QLinearGradient* theWrappedObject, qreal  x, qreal  y);
   void setStart(QLinearGradient* theWrappedObject, const QPointF&  start);
   void setStart(QLinearGradient* theWrappedObject, qreal  x, qreal  y);
   QPointF  start(QLinearGradient* theWrappedObject) const;
};





class PythonQtShell_QListView : public QListView
{
public:
    PythonQtShell_QListView(QWidget*  parent = 0):QListView(parent),_wrapper(NULL) {};

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

class PythonQtPublicPromoter_QListView : public QListView
{ public:
inline void promoted_currentChanged(const QModelIndex&  current, const QModelIndex&  previous) { QListView::currentChanged(current, previous); }
inline void promoted_dataChanged(const QModelIndex&  topLeft, const QModelIndex&  bottomRight) { QListView::dataChanged(topLeft, bottomRight); }
inline void promoted_doItemsLayout() { QListView::doItemsLayout(); }
inline void promoted_dragLeaveEvent(QDragLeaveEvent*  e) { QListView::dragLeaveEvent(e); }
inline void promoted_dragMoveEvent(QDragMoveEvent*  e) { QListView::dragMoveEvent(e); }
inline void promoted_dropEvent(QDropEvent*  e) { QListView::dropEvent(e); }
inline bool  promoted_event(QEvent*  e) { return QListView::event(e); }
inline int  promoted_horizontalOffset() const { return QListView::horizontalOffset(); }
inline QModelIndex  promoted_indexAt(const QPoint&  p) const { return QListView::indexAt(p); }
inline bool  promoted_isIndexHidden(const QModelIndex&  index) const { return QListView::isIndexHidden(index); }
inline void promoted_mouseMoveEvent(QMouseEvent*  e) { QListView::mouseMoveEvent(e); }
inline void promoted_mouseReleaseEvent(QMouseEvent*  e) { QListView::mouseReleaseEvent(e); }
inline void promoted_paintEvent(QPaintEvent*  e) { QListView::paintEvent(e); }
inline void promoted_reset() { QListView::reset(); }
inline void promoted_resizeEvent(QResizeEvent*  e) { QListView::resizeEvent(e); }
inline void promoted_rowsAboutToBeRemoved(const QModelIndex&  parent, int  start, int  end) { QListView::rowsAboutToBeRemoved(parent, start, end); }
inline void promoted_rowsInserted(const QModelIndex&  parent, int  start, int  end) { QListView::rowsInserted(parent, start, end); }
inline void promoted_scrollContentsBy(int  dx, int  dy) { QListView::scrollContentsBy(dx, dy); }
inline void promoted_scrollTo(const QModelIndex&  index, QAbstractItemView::ScrollHint  hint = QAbstractItemView::EnsureVisible) { QListView::scrollTo(index, hint); }
inline QList<QModelIndex >  promoted_selectedIndexes() const { return QListView::selectedIndexes(); }
inline void promoted_selectionChanged(const QItemSelection&  selected, const QItemSelection&  deselected) { QListView::selectionChanged(selected, deselected); }
inline void promoted_setRootIndex(const QModelIndex&  index) { QListView::setRootIndex(index); }
inline void promoted_setSelection(const QRect&  rect, QItemSelectionModel::SelectionFlags  command) { QListView::setSelection(rect, command); }
inline void promoted_startDrag(Qt::DropActions  supportedActions) { QListView::startDrag(supportedActions); }
inline void promoted_timerEvent(QTimerEvent*  e) { QListView::timerEvent(e); }
inline void promoted_updateGeometries() { QListView::updateGeometries(); }
inline int  promoted_verticalOffset() const { return QListView::verticalOffset(); }
inline QStyleOptionViewItem  promoted_viewOptions() const { return QListView::viewOptions(); }
inline QRect  promoted_visualRect(const QModelIndex&  index) const { return QListView::visualRect(index); }
inline QRegion  promoted_visualRegionForSelection(const QItemSelection&  selection) const { return QListView::visualRegionForSelection(selection); }
};

class PythonQtWrapper_QListView : public QObject
{ Q_OBJECT
public:
public slots:
QListView* new_QListView(QWidget*  parent = 0);
void delete_QListView(QListView* obj) { delete obj; } 
   int  batchSize(QListView* theWrappedObject) const;
   void clearPropertyFlags(QListView* theWrappedObject);
   void currentChanged(QListView* theWrappedObject, const QModelIndex&  current, const QModelIndex&  previous);
   void dataChanged(QListView* theWrappedObject, const QModelIndex&  topLeft, const QModelIndex&  bottomRight);
   void doItemsLayout(QListView* theWrappedObject);
   void dragLeaveEvent(QListView* theWrappedObject, QDragLeaveEvent*  e);
   void dragMoveEvent(QListView* theWrappedObject, QDragMoveEvent*  e);
   void dropEvent(QListView* theWrappedObject, QDropEvent*  e);
   bool  event(QListView* theWrappedObject, QEvent*  e);
   QListView::Flow  flow(QListView* theWrappedObject) const;
   QSize  gridSize(QListView* theWrappedObject) const;
   int  horizontalOffset(QListView* theWrappedObject) const;
   QModelIndex  indexAt(QListView* theWrappedObject, const QPoint&  p) const;
   bool  isIndexHidden(QListView* theWrappedObject, const QModelIndex&  index) const;
   bool  isRowHidden(QListView* theWrappedObject, int  row) const;
   bool  isSelectionRectVisible(QListView* theWrappedObject) const;
   bool  isWrapping(QListView* theWrappedObject) const;
   QListView::LayoutMode  layoutMode(QListView* theWrappedObject) const;
   int  modelColumn(QListView* theWrappedObject) const;
   void mouseMoveEvent(QListView* theWrappedObject, QMouseEvent*  e);
   void mouseReleaseEvent(QListView* theWrappedObject, QMouseEvent*  e);
   QListView::Movement  movement(QListView* theWrappedObject) const;
   void paintEvent(QListView* theWrappedObject, QPaintEvent*  e);
   void reset(QListView* theWrappedObject);
   void resizeEvent(QListView* theWrappedObject, QResizeEvent*  e);
   QListView::ResizeMode  resizeMode(QListView* theWrappedObject) const;
   void rowsAboutToBeRemoved(QListView* theWrappedObject, const QModelIndex&  parent, int  start, int  end);
   void rowsInserted(QListView* theWrappedObject, const QModelIndex&  parent, int  start, int  end);
   void scrollContentsBy(QListView* theWrappedObject, int  dx, int  dy);
   void scrollTo(QListView* theWrappedObject, const QModelIndex&  index, QAbstractItemView::ScrollHint  hint = QAbstractItemView::EnsureVisible);
   QList<QModelIndex >  selectedIndexes(QListView* theWrappedObject) const;
   void selectionChanged(QListView* theWrappedObject, const QItemSelection&  selected, const QItemSelection&  deselected);
   void setBatchSize(QListView* theWrappedObject, int  batchSize);
   void setFlow(QListView* theWrappedObject, QListView::Flow  flow);
   void setGridSize(QListView* theWrappedObject, const QSize&  size);
   void setLayoutMode(QListView* theWrappedObject, QListView::LayoutMode  mode);
   void setModelColumn(QListView* theWrappedObject, int  column);
   void setMovement(QListView* theWrappedObject, QListView::Movement  movement);
   void setResizeMode(QListView* theWrappedObject, QListView::ResizeMode  mode);
   void setRootIndex(QListView* theWrappedObject, const QModelIndex&  index);
   void setRowHidden(QListView* theWrappedObject, int  row, bool  hide);
   void setSelection(QListView* theWrappedObject, const QRect&  rect, QItemSelectionModel::SelectionFlags  command);
   void setSelectionRectVisible(QListView* theWrappedObject, bool  show);
   void setSpacing(QListView* theWrappedObject, int  space);
   void setUniformItemSizes(QListView* theWrappedObject, bool  enable);
   void setViewMode(QListView* theWrappedObject, QListView::ViewMode  mode);
   void setWordWrap(QListView* theWrappedObject, bool  on);
   void setWrapping(QListView* theWrappedObject, bool  enable);
   int  spacing(QListView* theWrappedObject) const;
   void startDrag(QListView* theWrappedObject, Qt::DropActions  supportedActions);
   void timerEvent(QListView* theWrappedObject, QTimerEvent*  e);
   bool  uniformItemSizes(QListView* theWrappedObject) const;
   void updateGeometries(QListView* theWrappedObject);
   int  verticalOffset(QListView* theWrappedObject) const;
   QListView::ViewMode  viewMode(QListView* theWrappedObject) const;
   QStyleOptionViewItem  viewOptions(QListView* theWrappedObject) const;
   QRect  visualRect(QListView* theWrappedObject, const QModelIndex&  index) const;
   QRegion  visualRegionForSelection(QListView* theWrappedObject, const QItemSelection&  selection) const;
   bool  wordWrap(QListView* theWrappedObject) const;
};





class PythonQtShell_QListWidget : public QListWidget
{
public:
    PythonQtShell_QListWidget(QWidget*  parent = 0):QListWidget(parent),_wrapper(NULL) {};

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
virtual void dropEvent(QDropEvent*  event);
virtual bool  dropMimeData(int  index, const QMimeData*  data, Qt::DropAction  action);
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
virtual QMimeData*  mimeData(const QList<QListWidgetItem* >  items) const;
virtual QStringList  mimeTypes() const;
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
virtual void setRootIndex(const QModelIndex&  index);
virtual void setSelection(const QRect&  rect, QItemSelectionModel::SelectionFlags  command);
virtual void setSelectionModel(QItemSelectionModel*  selectionModel);
virtual void showEvent(QShowEvent*  arg__1);
virtual int  sizeHintForColumn(int  column) const;
virtual int  sizeHintForRow(int  row) const;
virtual void startDrag(Qt::DropActions  supportedActions);
virtual Qt::DropActions  supportedDropActions() const;
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

class PythonQtPublicPromoter_QListWidget : public QListWidget
{ public:
inline void promoted_dropEvent(QDropEvent*  event) { QListWidget::dropEvent(event); }
inline bool  promoted_dropMimeData(int  index, const QMimeData*  data, Qt::DropAction  action) { return QListWidget::dropMimeData(index, data, action); }
inline bool  promoted_event(QEvent*  e) { return QListWidget::event(e); }
inline QStringList  promoted_mimeTypes() const { return QListWidget::mimeTypes(); }
inline Qt::DropActions  promoted_supportedDropActions() const { return QListWidget::supportedDropActions(); }
};

class PythonQtWrapper_QListWidget : public QObject
{ Q_OBJECT
public:
public slots:
QListWidget* new_QListWidget(QWidget*  parent = 0);
void delete_QListWidget(QListWidget* obj) { delete obj; } 
   void addItem(QListWidget* theWrappedObject, QListWidgetItem*  item);
   void addItem(QListWidget* theWrappedObject, const QString&  label);
   void addItems(QListWidget* theWrappedObject, const QStringList&  labels);
   void closePersistentEditor(QListWidget* theWrappedObject, QListWidgetItem*  item);
   int  count(QListWidget* theWrappedObject) const;
   QListWidgetItem*  currentItem(QListWidget* theWrappedObject) const;
   int  currentRow(QListWidget* theWrappedObject) const;
   void dropEvent(QListWidget* theWrappedObject, QDropEvent*  event);
   bool  dropMimeData(QListWidget* theWrappedObject, int  index, const QMimeData*  data, Qt::DropAction  action);
   void editItem(QListWidget* theWrappedObject, QListWidgetItem*  item);
   bool  event(QListWidget* theWrappedObject, QEvent*  e);
   QList<QListWidgetItem* >  findItems(QListWidget* theWrappedObject, const QString&  text, Qt::MatchFlags  flags) const;
   void insertItem(QListWidget* theWrappedObject, int  row, QListWidgetItem*  item);
   void insertItem(QListWidget* theWrappedObject, int  row, const QString&  label);
   void insertItems(QListWidget* theWrappedObject, int  row, const QStringList&  labels);
   bool  isSortingEnabled(QListWidget* theWrappedObject) const;
   QListWidgetItem*  item(QListWidget* theWrappedObject, int  row) const;
   QListWidgetItem*  itemAt(QListWidget* theWrappedObject, const QPoint&  p) const;
   QListWidgetItem*  itemAt(QListWidget* theWrappedObject, int  x, int  y) const;
   QWidget*  itemWidget(QListWidget* theWrappedObject, QListWidgetItem*  item) const;
   QStringList  mimeTypes(QListWidget* theWrappedObject) const;
   void openPersistentEditor(QListWidget* theWrappedObject, QListWidgetItem*  item);
   void removeItemWidget(QListWidget* theWrappedObject, QListWidgetItem*  item);
   int  row(QListWidget* theWrappedObject, const QListWidgetItem*  item) const;
   QList<QListWidgetItem* >  selectedItems(QListWidget* theWrappedObject) const;
   void setCurrentItem(QListWidget* theWrappedObject, QListWidgetItem*  item);
   void setCurrentItem(QListWidget* theWrappedObject, QListWidgetItem*  item, QItemSelectionModel::SelectionFlags  command);
   void setCurrentRow(QListWidget* theWrappedObject, int  row);
   void setCurrentRow(QListWidget* theWrappedObject, int  row, QItemSelectionModel::SelectionFlags  command);
   void setItemWidget(QListWidget* theWrappedObject, QListWidgetItem*  item, QWidget*  widget);
   void setSortingEnabled(QListWidget* theWrappedObject, bool  enable);
   void sortItems(QListWidget* theWrappedObject, Qt::SortOrder  order = Qt::AscendingOrder);
   Qt::DropActions  supportedDropActions(QListWidget* theWrappedObject) const;
   QListWidgetItem*  takeItem(QListWidget* theWrappedObject, int  row);
   QRect  visualItemRect(QListWidget* theWrappedObject, const QListWidgetItem*  item) const;
};





class PythonQtShell_QListWidgetItem : public QListWidgetItem
{
public:
    PythonQtShell_QListWidgetItem(QListWidget*  view = 0, int  type = Type):QListWidgetItem(view, type),_wrapper(NULL) {};
    PythonQtShell_QListWidgetItem(const QIcon&  icon, const QString&  text, QListWidget*  view = 0, int  type = Type):QListWidgetItem(icon, text, view, type),_wrapper(NULL) {};
    PythonQtShell_QListWidgetItem(const QString&  text, QListWidget*  view = 0, int  type = Type):QListWidgetItem(text, view, type),_wrapper(NULL) {};

virtual QListWidgetItem*  clone() const;
virtual QVariant  data(int  role) const;
virtual bool  __lt__(const QListWidgetItem&  other) const;
virtual void read(QDataStream&  in);
virtual void setBackgroundColor(const QColor&  color);
virtual void setData(int  role, const QVariant&  value);
virtual void write(QDataStream&  out) const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QListWidgetItem : public QListWidgetItem
{ public:
inline QListWidgetItem*  promoted_clone() const { return QListWidgetItem::clone(); }
inline QVariant  promoted_data(int  role) const { return QListWidgetItem::data(role); }
inline void promoted_setData(int  role, const QVariant&  value) { QListWidgetItem::setData(role, value); }
};

class PythonQtWrapper_QListWidgetItem : public QObject
{ Q_OBJECT
public:
Q_ENUMS(ItemType )
enum ItemType{
  Type = QListWidgetItem::Type,   UserType = QListWidgetItem::UserType};
public slots:
QListWidgetItem* new_QListWidgetItem(QListWidget*  view = 0, int  type = Type);
QListWidgetItem* new_QListWidgetItem(const QIcon&  icon, const QString&  text, QListWidget*  view = 0, int  type = Type);
QListWidgetItem* new_QListWidgetItem(const QString&  text, QListWidget*  view = 0, int  type = Type);
void delete_QListWidgetItem(QListWidgetItem* obj) { delete obj; } 
   QBrush  background(QListWidgetItem* theWrappedObject) const;
   Qt::CheckState  checkState(QListWidgetItem* theWrappedObject) const;
   QListWidgetItem*  clone(QListWidgetItem* theWrappedObject) const;
   QVariant  data(QListWidgetItem* theWrappedObject, int  role) const;
   Qt::ItemFlags  flags(QListWidgetItem* theWrappedObject) const;
   QFont  font(QListWidgetItem* theWrappedObject) const;
   QBrush  foreground(QListWidgetItem* theWrappedObject) const;
   QIcon  icon(QListWidgetItem* theWrappedObject) const;
   bool  isHidden(QListWidgetItem* theWrappedObject) const;
   bool  isSelected(QListWidgetItem* theWrappedObject) const;
   QListWidget*  listWidget(QListWidgetItem* theWrappedObject) const;
   void writeTo(QListWidgetItem* theWrappedObject, QDataStream&  out);
   void readFrom(QListWidgetItem* theWrappedObject, QDataStream&  in);
   void setBackground(QListWidgetItem* theWrappedObject, const QBrush&  brush);
   void setCheckState(QListWidgetItem* theWrappedObject, Qt::CheckState  state);
   void setData(QListWidgetItem* theWrappedObject, int  role, const QVariant&  value);
   void setFlags(QListWidgetItem* theWrappedObject, Qt::ItemFlags  flags);
   void setFont(QListWidgetItem* theWrappedObject, const QFont&  font);
   void setForeground(QListWidgetItem* theWrappedObject, const QBrush&  brush);
   void setHidden(QListWidgetItem* theWrappedObject, bool  hide);
   void setIcon(QListWidgetItem* theWrappedObject, const QIcon&  icon);
   void setSelected(QListWidgetItem* theWrappedObject, bool  select);
   void setSizeHint(QListWidgetItem* theWrappedObject, const QSize&  size);
   void setStatusTip(QListWidgetItem* theWrappedObject, const QString&  statusTip);
   void setText(QListWidgetItem* theWrappedObject, const QString&  text);
   void setTextAlignment(QListWidgetItem* theWrappedObject, int  alignment);
   void setToolTip(QListWidgetItem* theWrappedObject, const QString&  toolTip);
   void setWhatsThis(QListWidgetItem* theWrappedObject, const QString&  whatsThis);
   QSize  sizeHint(QListWidgetItem* theWrappedObject) const;
   QString  statusTip(QListWidgetItem* theWrappedObject) const;
   QString  text(QListWidgetItem* theWrappedObject) const;
   int  textAlignment(QListWidgetItem* theWrappedObject) const;
   QString  toolTip(QListWidgetItem* theWrappedObject) const;
   int  type(QListWidgetItem* theWrappedObject) const;
   QString  whatsThis(QListWidgetItem* theWrappedObject) const;
};





class PythonQtShell_QMainWindow : public QMainWindow
{
public:
    PythonQtShell_QMainWindow(QWidget*  parent = 0, Qt::WindowFlags  flags = 0):QMainWindow(parent, flags),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  arg__1);
virtual void childEvent(QChildEvent*  arg__1);
virtual void closeEvent(QCloseEvent*  arg__1);
virtual void contextMenuEvent(QContextMenuEvent*  event);
virtual QMenu*  createPopupMenu();
virtual void customEvent(QEvent*  arg__1);
virtual int  devType() const;
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

class PythonQtPublicPromoter_QMainWindow : public QMainWindow
{ public:
inline void promoted_contextMenuEvent(QContextMenuEvent*  event) { QMainWindow::contextMenuEvent(event); }
inline QMenu*  promoted_createPopupMenu() { return QMainWindow::createPopupMenu(); }
inline bool  promoted_event(QEvent*  event) { return QMainWindow::event(event); }
};

class PythonQtWrapper_QMainWindow : public QObject
{ Q_OBJECT
public:
public slots:
QMainWindow* new_QMainWindow(QWidget*  parent = 0, Qt::WindowFlags  flags = 0);
void delete_QMainWindow(QMainWindow* obj) { delete obj; } 
   void addDockWidget(QMainWindow* theWrappedObject, Qt::DockWidgetArea  area, QDockWidget*  dockwidget);
   void addDockWidget(QMainWindow* theWrappedObject, Qt::DockWidgetArea  area, QDockWidget*  dockwidget, Qt::Orientation  orientation);
   void addToolBar(QMainWindow* theWrappedObject, QToolBar*  toolbar);
   void addToolBar(QMainWindow* theWrappedObject, Qt::ToolBarArea  area, QToolBar*  toolbar);
   QToolBar*  addToolBar(QMainWindow* theWrappedObject, const QString&  title);
   void addToolBarBreak(QMainWindow* theWrappedObject, Qt::ToolBarArea  area = Qt::TopToolBarArea);
   QWidget*  centralWidget(QMainWindow* theWrappedObject) const;
   void contextMenuEvent(QMainWindow* theWrappedObject, QContextMenuEvent*  event);
   Qt::DockWidgetArea  corner(QMainWindow* theWrappedObject, Qt::Corner  corner) const;
   QMenu*  createPopupMenu(QMainWindow* theWrappedObject);
   QMainWindow::DockOptions  dockOptions(QMainWindow* theWrappedObject) const;
   Qt::DockWidgetArea  dockWidgetArea(QMainWindow* theWrappedObject, QDockWidget*  dockwidget) const;
   bool  documentMode(QMainWindow* theWrappedObject) const;
   bool  event(QMainWindow* theWrappedObject, QEvent*  event);
   QSize  iconSize(QMainWindow* theWrappedObject) const;
   void insertToolBar(QMainWindow* theWrappedObject, QToolBar*  before, QToolBar*  toolbar);
   void insertToolBarBreak(QMainWindow* theWrappedObject, QToolBar*  before);
   bool  isAnimated(QMainWindow* theWrappedObject) const;
   bool  isDockNestingEnabled(QMainWindow* theWrappedObject) const;
   bool  isSeparator(QMainWindow* theWrappedObject, const QPoint&  pos) const;
   QMenuBar*  menuBar(QMainWindow* theWrappedObject) const;
   QWidget*  menuWidget(QMainWindow* theWrappedObject) const;
   void removeDockWidget(QMainWindow* theWrappedObject, QDockWidget*  dockwidget);
   void removeToolBar(QMainWindow* theWrappedObject, QToolBar*  toolbar);
   void removeToolBarBreak(QMainWindow* theWrappedObject, QToolBar*  before);
   bool  restoreDockWidget(QMainWindow* theWrappedObject, QDockWidget*  dockwidget);
   bool  restoreState(QMainWindow* theWrappedObject, const QByteArray&  state, int  version = 0);
   QByteArray  saveState(QMainWindow* theWrappedObject, int  version = 0) const;
   void setCentralWidget(QMainWindow* theWrappedObject, QWidget*  widget);
   void setCorner(QMainWindow* theWrappedObject, Qt::Corner  corner, Qt::DockWidgetArea  area);
   void setDockOptions(QMainWindow* theWrappedObject, QMainWindow::DockOptions  options);
   void setDocumentMode(QMainWindow* theWrappedObject, bool  enabled);
   void setIconSize(QMainWindow* theWrappedObject, const QSize&  iconSize);
   void setMenuBar(QMainWindow* theWrappedObject, QMenuBar*  menubar);
   void setMenuWidget(QMainWindow* theWrappedObject, QWidget*  menubar);
   void setStatusBar(QMainWindow* theWrappedObject, QStatusBar*  statusbar);
   void setTabPosition(QMainWindow* theWrappedObject, Qt::DockWidgetAreas  areas, QTabWidget::TabPosition  tabPosition);
   void setTabShape(QMainWindow* theWrappedObject, QTabWidget::TabShape  tabShape);
   void setToolButtonStyle(QMainWindow* theWrappedObject, Qt::ToolButtonStyle  toolButtonStyle);
   void setUnifiedTitleAndToolBarOnMac(QMainWindow* theWrappedObject, bool  set);
   void splitDockWidget(QMainWindow* theWrappedObject, QDockWidget*  after, QDockWidget*  dockwidget, Qt::Orientation  orientation);
   QStatusBar*  statusBar(QMainWindow* theWrappedObject) const;
   QTabWidget::TabPosition  tabPosition(QMainWindow* theWrappedObject, Qt::DockWidgetArea  area) const;
   QTabWidget::TabShape  tabShape(QMainWindow* theWrappedObject) const;
   QList<QDockWidget* >  tabifiedDockWidgets(QMainWindow* theWrappedObject, QDockWidget*  dockwidget) const;
   void tabifyDockWidget(QMainWindow* theWrappedObject, QDockWidget*  first, QDockWidget*  second);
   Qt::ToolBarArea  toolBarArea(QMainWindow* theWrappedObject, QToolBar*  toolbar) const;
   bool  toolBarBreak(QMainWindow* theWrappedObject, QToolBar*  toolbar) const;
   Qt::ToolButtonStyle  toolButtonStyle(QMainWindow* theWrappedObject) const;
   bool  unifiedTitleAndToolBarOnMac(QMainWindow* theWrappedObject) const;
};





class PythonQtWrapper_QMargins : public QObject
{ Q_OBJECT
public:
public slots:
QMargins* new_QMargins();
QMargins* new_QMargins(int  left, int  top, int  right, int  bottom);
QMargins* new_QMargins(const QMargins& other) {
QMargins* a = new QMargins();
*((QMargins*)a) = other;
return a; }
void delete_QMargins(QMargins* obj) { delete obj; } 
   int  bottom(QMargins* theWrappedObject) const;
   bool  isNull(QMargins* theWrappedObject) const;
   int  left(QMargins* theWrappedObject) const;
   bool  __eq__(QMargins* theWrappedObject, const QMargins&  m2);
   int  right(QMargins* theWrappedObject) const;
   void setBottom(QMargins* theWrappedObject, int  bottom);
   void setLeft(QMargins* theWrappedObject, int  left);
   void setRight(QMargins* theWrappedObject, int  right);
   void setTop(QMargins* theWrappedObject, int  top);
   int  top(QMargins* theWrappedObject) const;
    QString py_toString(QMargins*);
    bool __nonzero__(QMargins* obj) { return !obj->isNull(); }
};


