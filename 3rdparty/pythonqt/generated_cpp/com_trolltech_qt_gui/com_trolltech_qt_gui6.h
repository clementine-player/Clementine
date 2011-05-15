#include <PythonQt.h>
#include <QObject>
#include <QPrinterInfo>
#include <QVariant>
#include <qabstractprintdialog.h>
#include <qaction.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qbrush.h>
#include <qbuttongroup.h>
#include <qbytearray.h>
#include <qcolor.h>
#include <qcoreevent.h>
#include <qcursor.h>
#include <qdatastream.h>
#include <qdialog.h>
#include <qevent.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qgraphicseffect.h>
#include <qgraphicsproxywidget.h>
#include <qicon.h>
#include <qinputcontext.h>
#include <qkeysequence.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlayoutitem.h>
#include <qlineedit.h>
#include <qlist.h>
#include <qlocale.h>
#include <qmargins.h>
#include <qmenu.h>
#include <qobject.h>
#include <qpaintdevice.h>
#include <qpaintengine.h>
#include <qpainter.h>
#include <qpair.h>
#include <qpalette.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qprintdialog.h>
#include <qprintengine.h>
#include <qprinter.h>
#include <qprintpreviewdialog.h>
#include <qprintpreviewwidget.h>
#include <qprogressbar.h>
#include <qprogressdialog.h>
#include <qproxystyle.h>
#include <qpushbutton.h>
#include <qquaternion.h>
#include <qradiobutton.h>
#include <qrect.h>
#include <qregexp.h>
#include <qregion.h>
#include <qrubberband.h>
#include <qscrollarea.h>
#include <qscrollbar.h>
#include <qsessionmanager.h>
#include <qshortcut.h>
#include <qsize.h>
#include <qsizegrip.h>
#include <qsizepolicy.h>
#include <qslider.h>
#include <qsound.h>
#include <qspinbox.h>
#include <qsplashscreen.h>
#include <qsplitter.h>
#include <qstackedlayout.h>
#include <qstringlist.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtextstream.h>
#include <qvalidator.h>
#include <qvector.h>
#include <qvector3d.h>
#include <qvector4d.h>
#include <qwidget.h>



class PythonQtShell_QPrintDialog : public QPrintDialog
{
public:
    PythonQtShell_QPrintDialog(QPrinter*  printer, QWidget*  parent = 0):QPrintDialog(printer, parent),_wrapper(NULL) {};
    PythonQtShell_QPrintDialog(QWidget*  parent = 0):QPrintDialog(parent),_wrapper(NULL) {};

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
virtual int  exec();
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

class PythonQtPublicPromoter_QPrintDialog : public QPrintDialog
{ public:
inline void promoted_accepted() { QPrintDialog::accepted(); }
inline void promoted_done(int  result) { QPrintDialog::done(result); }
inline int  promoted_exec() { return QPrintDialog::exec(); }
};

class PythonQtWrapper_QPrintDialog : public QObject
{ Q_OBJECT
public:
public slots:
QPrintDialog* new_QPrintDialog(QPrinter*  printer, QWidget*  parent = 0);
QPrintDialog* new_QPrintDialog(QWidget*  parent = 0);
void delete_QPrintDialog(QPrintDialog* obj) { delete obj; } 
   void accepted(QPrintDialog* theWrappedObject);
   void done(QPrintDialog* theWrappedObject, int  result);
   int  exec(QPrintDialog* theWrappedObject);
   void open(QPrintDialog* theWrappedObject);
   void open(QPrintDialog* theWrappedObject, QObject*  receiver, const char*  member);
   QAbstractPrintDialog::PrintDialogOptions  options(QPrintDialog* theWrappedObject) const;
   void setOption(QPrintDialog* theWrappedObject, QAbstractPrintDialog::PrintDialogOption  option, bool  on = true);
   void setOptions(QPrintDialog* theWrappedObject, QAbstractPrintDialog::PrintDialogOptions  options);
   bool  testOption(QPrintDialog* theWrappedObject, QAbstractPrintDialog::PrintDialogOption  option) const;
};





class PythonQtShell_QPrintEngine : public QPrintEngine
{
public:
    PythonQtShell_QPrintEngine():QPrintEngine(),_wrapper(NULL) {};

virtual bool  abort();
virtual int  metric(QPaintDevice::PaintDeviceMetric  arg__1) const;
virtual bool  newPage();
virtual QPrinter::PrinterState  printerState() const;
virtual QVariant  property(QPrintEngine::PrintEnginePropertyKey  key) const;
virtual void setProperty(QPrintEngine::PrintEnginePropertyKey  key, const QVariant&  value);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QPrintEngine : public QObject
{ Q_OBJECT
public:
Q_ENUMS(PrintEnginePropertyKey )
enum PrintEnginePropertyKey{
  PPK_CollateCopies = QPrintEngine::PPK_CollateCopies,   PPK_ColorMode = QPrintEngine::PPK_ColorMode,   PPK_Creator = QPrintEngine::PPK_Creator,   PPK_DocumentName = QPrintEngine::PPK_DocumentName,   PPK_FullPage = QPrintEngine::PPK_FullPage,   PPK_NumberOfCopies = QPrintEngine::PPK_NumberOfCopies,   PPK_Orientation = QPrintEngine::PPK_Orientation,   PPK_OutputFileName = QPrintEngine::PPK_OutputFileName,   PPK_PageOrder = QPrintEngine::PPK_PageOrder,   PPK_PageRect = QPrintEngine::PPK_PageRect,   PPK_PageSize = QPrintEngine::PPK_PageSize,   PPK_PaperRect = QPrintEngine::PPK_PaperRect,   PPK_PaperSource = QPrintEngine::PPK_PaperSource,   PPK_PrinterName = QPrintEngine::PPK_PrinterName,   PPK_PrinterProgram = QPrintEngine::PPK_PrinterProgram,   PPK_Resolution = QPrintEngine::PPK_Resolution,   PPK_SelectionOption = QPrintEngine::PPK_SelectionOption,   PPK_SupportedResolutions = QPrintEngine::PPK_SupportedResolutions,   PPK_WindowsPageSize = QPrintEngine::PPK_WindowsPageSize,   PPK_FontEmbedding = QPrintEngine::PPK_FontEmbedding,   PPK_SuppressSystemPrintStatus = QPrintEngine::PPK_SuppressSystemPrintStatus,   PPK_Duplex = QPrintEngine::PPK_Duplex,   PPK_PaperSources = QPrintEngine::PPK_PaperSources,   PPK_CustomPaperSize = QPrintEngine::PPK_CustomPaperSize,   PPK_PageMargins = QPrintEngine::PPK_PageMargins,   PPK_PaperSize = QPrintEngine::PPK_PaperSize,   PPK_CustomBase = QPrintEngine::PPK_CustomBase};
public slots:
QPrintEngine* new_QPrintEngine();
void delete_QPrintEngine(QPrintEngine* obj) { delete obj; } 
};





class PythonQtShell_QPrintPreviewDialog : public QPrintPreviewDialog
{
public:
    PythonQtShell_QPrintPreviewDialog(QPrinter*  printer, QWidget*  parent = 0, Qt::WindowFlags  flags = 0):QPrintPreviewDialog(printer, parent, flags),_wrapper(NULL) {};
    PythonQtShell_QPrintPreviewDialog(QWidget*  parent = 0, Qt::WindowFlags  flags = 0):QPrintPreviewDialog(parent, flags),_wrapper(NULL) {};

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

class PythonQtPublicPromoter_QPrintPreviewDialog : public QPrintPreviewDialog
{ public:
inline void promoted_done(int  result) { QPrintPreviewDialog::done(result); }
};

class PythonQtWrapper_QPrintPreviewDialog : public QObject
{ Q_OBJECT
public:
public slots:
QPrintPreviewDialog* new_QPrintPreviewDialog(QPrinter*  printer, QWidget*  parent = 0, Qt::WindowFlags  flags = 0);
QPrintPreviewDialog* new_QPrintPreviewDialog(QWidget*  parent = 0, Qt::WindowFlags  flags = 0);
void delete_QPrintPreviewDialog(QPrintPreviewDialog* obj) { delete obj; } 
   void done(QPrintPreviewDialog* theWrappedObject, int  result);
   void open(QPrintPreviewDialog* theWrappedObject);
   void open(QPrintPreviewDialog* theWrappedObject, QObject*  receiver, const char*  member);
   QPrinter*  printer(QPrintPreviewDialog* theWrappedObject);
   void setVisible(QPrintPreviewDialog* theWrappedObject, bool  visible);
};





class PythonQtShell_QPrintPreviewWidget : public QPrintPreviewWidget
{
public:
    PythonQtShell_QPrintPreviewWidget(QPrinter*  printer, QWidget*  parent = 0, Qt::WindowFlags  flags = 0):QPrintPreviewWidget(printer, parent, flags),_wrapper(NULL) {};
    PythonQtShell_QPrintPreviewWidget(QWidget*  parent = 0, Qt::WindowFlags  flags = 0):QPrintPreviewWidget(parent, flags),_wrapper(NULL) {};

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

class PythonQtWrapper_QPrintPreviewWidget : public QObject
{ Q_OBJECT
public:
Q_ENUMS(ZoomMode ViewMode )
enum ZoomMode{
  CustomZoom = QPrintPreviewWidget::CustomZoom,   FitToWidth = QPrintPreviewWidget::FitToWidth,   FitInView = QPrintPreviewWidget::FitInView};
enum ViewMode{
  SinglePageView = QPrintPreviewWidget::SinglePageView,   FacingPagesView = QPrintPreviewWidget::FacingPagesView,   AllPagesView = QPrintPreviewWidget::AllPagesView};
public slots:
QPrintPreviewWidget* new_QPrintPreviewWidget(QPrinter*  printer, QWidget*  parent = 0, Qt::WindowFlags  flags = 0);
QPrintPreviewWidget* new_QPrintPreviewWidget(QWidget*  parent = 0, Qt::WindowFlags  flags = 0);
void delete_QPrintPreviewWidget(QPrintPreviewWidget* obj) { delete obj; } 
   int  currentPage(QPrintPreviewWidget* theWrappedObject) const;
   int  numPages(QPrintPreviewWidget* theWrappedObject) const;
   QPrinter::Orientation  orientation(QPrintPreviewWidget* theWrappedObject) const;
   int  pageCount(QPrintPreviewWidget* theWrappedObject) const;
   void setVisible(QPrintPreviewWidget* theWrappedObject, bool  visible);
   QPrintPreviewWidget::ViewMode  viewMode(QPrintPreviewWidget* theWrappedObject) const;
   qreal  zoomFactor(QPrintPreviewWidget* theWrappedObject) const;
   QPrintPreviewWidget::ZoomMode  zoomMode(QPrintPreviewWidget* theWrappedObject) const;
};





class PythonQtShell_QPrinter : public QPrinter
{
public:
    PythonQtShell_QPrinter(QPrinter::PrinterMode  mode = QPrinter::ScreenResolution):QPrinter(mode),_wrapper(NULL) {};
    PythonQtShell_QPrinter(const QPrinterInfo&  printer, QPrinter::PrinterMode  mode = QPrinter::ScreenResolution):QPrinter(printer, mode),_wrapper(NULL) {};

virtual int  devType() const;
virtual int  metric(QPaintDevice::PaintDeviceMetric  arg__1) const;
virtual QPaintEngine*  paintEngine() const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QPrinter : public QPrinter
{ public:
inline int  promoted_devType() const { return QPrinter::devType(); }
inline int  promoted_metric(QPaintDevice::PaintDeviceMetric  arg__1) const { return QPrinter::metric(arg__1); }
inline QPaintEngine*  promoted_paintEngine() const { return QPrinter::paintEngine(); }
};

class PythonQtWrapper_QPrinter : public QObject
{ Q_OBJECT
public:
Q_ENUMS(OutputFormat DuplexMode PrintRange Unit PrinterMode Orientation PrinterState ColorMode PaperSource PageOrder PageSize )
enum OutputFormat{
  NativeFormat = QPrinter::NativeFormat,   PdfFormat = QPrinter::PdfFormat,   PostScriptFormat = QPrinter::PostScriptFormat};
enum DuplexMode{
  DuplexNone = QPrinter::DuplexNone,   DuplexAuto = QPrinter::DuplexAuto,   DuplexLongSide = QPrinter::DuplexLongSide,   DuplexShortSide = QPrinter::DuplexShortSide};
enum PrintRange{
  AllPages = QPrinter::AllPages,   Selection = QPrinter::Selection,   PageRange = QPrinter::PageRange};
enum Unit{
  Millimeter = QPrinter::Millimeter,   Point = QPrinter::Point,   Inch = QPrinter::Inch,   Pica = QPrinter::Pica,   Didot = QPrinter::Didot,   Cicero = QPrinter::Cicero,   DevicePixel = QPrinter::DevicePixel};
enum PrinterMode{
  ScreenResolution = QPrinter::ScreenResolution,   PrinterResolution = QPrinter::PrinterResolution,   HighResolution = QPrinter::HighResolution};
enum Orientation{
  Portrait = QPrinter::Portrait,   Landscape = QPrinter::Landscape};
enum PrinterState{
  Idle = QPrinter::Idle,   Active = QPrinter::Active,   Aborted = QPrinter::Aborted,   Error = QPrinter::Error};
enum ColorMode{
  GrayScale = QPrinter::GrayScale,   Color = QPrinter::Color};
enum PaperSource{
  OnlyOne = QPrinter::OnlyOne,   Lower = QPrinter::Lower,   Middle = QPrinter::Middle,   Manual = QPrinter::Manual,   Envelope = QPrinter::Envelope,   EnvelopeManual = QPrinter::EnvelopeManual,   Auto = QPrinter::Auto,   Tractor = QPrinter::Tractor,   SmallFormat = QPrinter::SmallFormat,   LargeFormat = QPrinter::LargeFormat,   LargeCapacity = QPrinter::LargeCapacity,   Cassette = QPrinter::Cassette,   FormSource = QPrinter::FormSource,   MaxPageSource = QPrinter::MaxPageSource};
enum PageOrder{
  FirstPageFirst = QPrinter::FirstPageFirst,   LastPageFirst = QPrinter::LastPageFirst};
enum PageSize{
  A4 = QPrinter::A4,   B5 = QPrinter::B5,   Letter = QPrinter::Letter,   Legal = QPrinter::Legal,   Executive = QPrinter::Executive,   A0 = QPrinter::A0,   A1 = QPrinter::A1,   A2 = QPrinter::A2,   A3 = QPrinter::A3,   A5 = QPrinter::A5,   A6 = QPrinter::A6,   A7 = QPrinter::A7,   A8 = QPrinter::A8,   A9 = QPrinter::A9,   B0 = QPrinter::B0,   B1 = QPrinter::B1,   B10 = QPrinter::B10,   B2 = QPrinter::B2,   B3 = QPrinter::B3,   B4 = QPrinter::B4,   B6 = QPrinter::B6,   B7 = QPrinter::B7,   B8 = QPrinter::B8,   B9 = QPrinter::B9,   C5E = QPrinter::C5E,   Comm10E = QPrinter::Comm10E,   DLE = QPrinter::DLE,   Folio = QPrinter::Folio,   Ledger = QPrinter::Ledger,   Tabloid = QPrinter::Tabloid,   Custom = QPrinter::Custom,   NPageSize = QPrinter::NPageSize,   NPaperSize = QPrinter::NPaperSize};
public slots:
QPrinter* new_QPrinter(QPrinter::PrinterMode  mode = QPrinter::ScreenResolution);
QPrinter* new_QPrinter(const QPrinterInfo&  printer, QPrinter::PrinterMode  mode = QPrinter::ScreenResolution);
void delete_QPrinter(QPrinter* obj) { delete obj; } 
   bool  abort(QPrinter* theWrappedObject);
   int  actualNumCopies(QPrinter* theWrappedObject) const;
   bool  collateCopies(QPrinter* theWrappedObject) const;
   QPrinter::ColorMode  colorMode(QPrinter* theWrappedObject) const;
   QString  creator(QPrinter* theWrappedObject) const;
   int  devType(QPrinter* theWrappedObject) const;
   QString  docName(QPrinter* theWrappedObject) const;
   bool  doubleSidedPrinting(QPrinter* theWrappedObject) const;
   QPrinter::DuplexMode  duplex(QPrinter* theWrappedObject) const;
   bool  fontEmbeddingEnabled(QPrinter* theWrappedObject) const;
   int  fromPage(QPrinter* theWrappedObject) const;
   bool  fullPage(QPrinter* theWrappedObject) const;
   void getPageMargins(QPrinter* theWrappedObject, qreal*  left, qreal*  top, qreal*  right, qreal*  bottom, QPrinter::Unit  unit) const;
   bool  isValid(QPrinter* theWrappedObject) const;
   int  metric(QPrinter* theWrappedObject, QPaintDevice::PaintDeviceMetric  arg__1) const;
   bool  newPage(QPrinter* theWrappedObject);
   int  numCopies(QPrinter* theWrappedObject) const;
   QPrinter::Orientation  orientation(QPrinter* theWrappedObject) const;
   QString  outputFileName(QPrinter* theWrappedObject) const;
   QPrinter::OutputFormat  outputFormat(QPrinter* theWrappedObject) const;
   QPrinter::PageOrder  pageOrder(QPrinter* theWrappedObject) const;
   QRect  pageRect(QPrinter* theWrappedObject) const;
   QRectF  pageRect(QPrinter* theWrappedObject, QPrinter::Unit  arg__1) const;
   QPrinter::PageSize  pageSize(QPrinter* theWrappedObject) const;
   QPaintEngine*  paintEngine(QPrinter* theWrappedObject) const;
   QRect  paperRect(QPrinter* theWrappedObject) const;
   QRectF  paperRect(QPrinter* theWrappedObject, QPrinter::Unit  arg__1) const;
   QPrinter::PageSize  paperSize(QPrinter* theWrappedObject) const;
   QSizeF  paperSize(QPrinter* theWrappedObject, QPrinter::Unit  unit) const;
   QPrinter::PaperSource  paperSource(QPrinter* theWrappedObject) const;
   QPrintEngine*  printEngine(QPrinter* theWrappedObject) const;
   QString  printProgram(QPrinter* theWrappedObject) const;
   QPrinter::PrintRange  printRange(QPrinter* theWrappedObject) const;
   QString  printerName(QPrinter* theWrappedObject) const;
   QPrinter::PrinterState  printerState(QPrinter* theWrappedObject) const;
   int  resolution(QPrinter* theWrappedObject) const;
   void setCollateCopies(QPrinter* theWrappedObject, bool  collate);
   void setColorMode(QPrinter* theWrappedObject, QPrinter::ColorMode  arg__1);
   void setCreator(QPrinter* theWrappedObject, const QString&  arg__1);
   void setDocName(QPrinter* theWrappedObject, const QString&  arg__1);
   void setDoubleSidedPrinting(QPrinter* theWrappedObject, bool  enable);
   void setDuplex(QPrinter* theWrappedObject, QPrinter::DuplexMode  duplex);
   void setFontEmbeddingEnabled(QPrinter* theWrappedObject, bool  enable);
   void setFromTo(QPrinter* theWrappedObject, int  fromPage, int  toPage);
   void setFullPage(QPrinter* theWrappedObject, bool  arg__1);
   void setNumCopies(QPrinter* theWrappedObject, int  arg__1);
   void setOrientation(QPrinter* theWrappedObject, QPrinter::Orientation  arg__1);
   void setOutputFileName(QPrinter* theWrappedObject, const QString&  arg__1);
   void setOutputFormat(QPrinter* theWrappedObject, QPrinter::OutputFormat  format);
   void setPageMargins(QPrinter* theWrappedObject, qreal  left, qreal  top, qreal  right, qreal  bottom, QPrinter::Unit  unit);
   void setPageOrder(QPrinter* theWrappedObject, QPrinter::PageOrder  arg__1);
   void setPageSize(QPrinter* theWrappedObject, QPrinter::PageSize  arg__1);
   void setPaperSize(QPrinter* theWrappedObject, QPrinter::PageSize  arg__1);
   void setPaperSize(QPrinter* theWrappedObject, const QSizeF&  paperSize, QPrinter::Unit  unit);
   void setPaperSource(QPrinter* theWrappedObject, QPrinter::PaperSource  arg__1);
   void setPrintProgram(QPrinter* theWrappedObject, const QString&  arg__1);
   void setPrintRange(QPrinter* theWrappedObject, QPrinter::PrintRange  range);
   void setPrinterName(QPrinter* theWrappedObject, const QString&  arg__1);
   void setResolution(QPrinter* theWrappedObject, int  arg__1);
   QList<int >  supportedResolutions(QPrinter* theWrappedObject) const;
   int  toPage(QPrinter* theWrappedObject) const;
};





class PythonQtShell_QProgressBar : public QProgressBar
{
public:
    PythonQtShell_QProgressBar(QWidget*  parent = 0):QProgressBar(parent),_wrapper(NULL) {};

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
virtual QString  text() const;
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QProgressBar : public QProgressBar
{ public:
inline bool  promoted_event(QEvent*  e) { return QProgressBar::event(e); }
inline void promoted_paintEvent(QPaintEvent*  arg__1) { QProgressBar::paintEvent(arg__1); }
inline QString  promoted_text() const { return QProgressBar::text(); }
};

class PythonQtWrapper_QProgressBar : public QObject
{ Q_OBJECT
public:
public slots:
QProgressBar* new_QProgressBar(QWidget*  parent = 0);
void delete_QProgressBar(QProgressBar* obj) { delete obj; } 
   Qt::Alignment  alignment(QProgressBar* theWrappedObject) const;
   bool  event(QProgressBar* theWrappedObject, QEvent*  e);
   QString  format(QProgressBar* theWrappedObject) const;
   bool  invertedAppearance(QProgressBar* theWrappedObject);
   bool  isTextVisible(QProgressBar* theWrappedObject) const;
   int  maximum(QProgressBar* theWrappedObject) const;
   int  minimum(QProgressBar* theWrappedObject) const;
   QSize  minimumSizeHint(QProgressBar* theWrappedObject) const;
   Qt::Orientation  orientation(QProgressBar* theWrappedObject) const;
   void paintEvent(QProgressBar* theWrappedObject, QPaintEvent*  arg__1);
   void setAlignment(QProgressBar* theWrappedObject, Qt::Alignment  alignment);
   void setFormat(QProgressBar* theWrappedObject, const QString&  format);
   void setInvertedAppearance(QProgressBar* theWrappedObject, bool  invert);
   void setTextDirection(QProgressBar* theWrappedObject, QProgressBar::Direction  textDirection);
   void setTextVisible(QProgressBar* theWrappedObject, bool  visible);
   QSize  sizeHint(QProgressBar* theWrappedObject) const;
   QString  text(QProgressBar* theWrappedObject) const;
   QProgressBar::Direction  textDirection(QProgressBar* theWrappedObject);
   int  value(QProgressBar* theWrappedObject) const;
};





class PythonQtShell_QProgressDialog : public QProgressDialog
{
public:
    PythonQtShell_QProgressDialog(QWidget*  parent = 0, Qt::WindowFlags  flags = 0):QProgressDialog(parent, flags),_wrapper(NULL) {};
    PythonQtShell_QProgressDialog(const QString&  labelText, const QString&  cancelButtonText, int  minimum, int  maximum, QWidget*  parent = 0, Qt::WindowFlags  flags = 0):QProgressDialog(labelText, cancelButtonText, minimum, maximum, parent, flags),_wrapper(NULL) {};

virtual void accept();
virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  event);
virtual void childEvent(QChildEvent*  arg__1);
virtual void closeEvent(QCloseEvent*  event);
virtual void contextMenuEvent(QContextMenuEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual int  devType() const;
virtual void done(int  arg__1);
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
virtual void resizeEvent(QResizeEvent*  event);
virtual void showEvent(QShowEvent*  event);
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QProgressDialog : public QProgressDialog
{ public:
inline void promoted_changeEvent(QEvent*  event) { QProgressDialog::changeEvent(event); }
inline void promoted_closeEvent(QCloseEvent*  event) { QProgressDialog::closeEvent(event); }
inline void promoted_resizeEvent(QResizeEvent*  event) { QProgressDialog::resizeEvent(event); }
inline void promoted_showEvent(QShowEvent*  event) { QProgressDialog::showEvent(event); }
};

class PythonQtWrapper_QProgressDialog : public QObject
{ Q_OBJECT
public:
public slots:
QProgressDialog* new_QProgressDialog(QWidget*  parent = 0, Qt::WindowFlags  flags = 0);
QProgressDialog* new_QProgressDialog(const QString&  labelText, const QString&  cancelButtonText, int  minimum, int  maximum, QWidget*  parent = 0, Qt::WindowFlags  flags = 0);
void delete_QProgressDialog(QProgressDialog* obj) { delete obj; } 
   bool  autoClose(QProgressDialog* theWrappedObject) const;
   bool  autoReset(QProgressDialog* theWrappedObject) const;
   void changeEvent(QProgressDialog* theWrappedObject, QEvent*  event);
   void closeEvent(QProgressDialog* theWrappedObject, QCloseEvent*  event);
   QString  labelText(QProgressDialog* theWrappedObject) const;
   int  maximum(QProgressDialog* theWrappedObject) const;
   int  minimum(QProgressDialog* theWrappedObject) const;
   int  minimumDuration(QProgressDialog* theWrappedObject) const;
   void open(QProgressDialog* theWrappedObject);
   void open(QProgressDialog* theWrappedObject, QObject*  receiver, const char*  member);
   void resizeEvent(QProgressDialog* theWrappedObject, QResizeEvent*  event);
   void setAutoClose(QProgressDialog* theWrappedObject, bool  close);
   void setAutoReset(QProgressDialog* theWrappedObject, bool  reset);
   void setBar(QProgressDialog* theWrappedObject, QProgressBar*  bar);
   void setCancelButton(QProgressDialog* theWrappedObject, QPushButton*  button);
   void setLabel(QProgressDialog* theWrappedObject, QLabel*  label);
   void showEvent(QProgressDialog* theWrappedObject, QShowEvent*  event);
   QSize  sizeHint(QProgressDialog* theWrappedObject) const;
   int  value(QProgressDialog* theWrappedObject) const;
   bool  wasCanceled(QProgressDialog* theWrappedObject) const;
};





class PythonQtShell_QProxyStyle : public QProxyStyle
{
public:
    PythonQtShell_QProxyStyle(QStyle*  baseStyle = 0):QProxyStyle(baseStyle),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual void drawComplexControl(QStyle::ComplexControl  control, const QStyleOptionComplex*  option, QPainter*  painter, const QWidget*  widget = 0) const;
virtual void drawControl(QStyle::ControlElement  element, const QStyleOption*  option, QPainter*  painter, const QWidget*  widget = 0) const;
virtual void drawItemPixmap(QPainter*  painter, const QRect&  rect, int  alignment, const QPixmap&  pixmap) const;
virtual void drawItemText(QPainter*  painter, const QRect&  rect, int  flags, const QPalette&  pal, bool  enabled, const QString&  text, QPalette::ColorRole  textRole = QPalette::NoRole) const;
virtual void drawPrimitive(QStyle::PrimitiveElement  element, const QStyleOption*  option, QPainter*  painter, const QWidget*  widget = 0) const;
virtual bool  event(QEvent*  e);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual QPixmap  generatedIconPixmap(QIcon::Mode  iconMode, const QPixmap&  pixmap, const QStyleOption*  opt) const;
virtual QStyle::SubControl  hitTestComplexControl(QStyle::ComplexControl  control, const QStyleOptionComplex*  option, const QPoint&  pos, const QWidget*  widget = 0) const;
virtual QRect  itemPixmapRect(const QRect&  r, int  flags, const QPixmap&  pixmap) const;
virtual int  pixelMetric(QStyle::PixelMetric  metric, const QStyleOption*  option = 0, const QWidget*  widget = 0) const;
virtual void polish(QApplication*  app);
virtual void polish(QPalette&  pal);
virtual void polish(QWidget*  widget);
virtual QSize  sizeFromContents(QStyle::ContentsType  type, const QStyleOption*  option, const QSize&  size, const QWidget*  widget) const;
virtual QPalette  standardPalette() const;
virtual QPixmap  standardPixmap(QStyle::StandardPixmap  standardPixmap, const QStyleOption*  opt, const QWidget*  widget = 0) const;
virtual int  styleHint(QStyle::StyleHint  hint, const QStyleOption*  option = 0, const QWidget*  widget = 0, QStyleHintReturn*  returnData = 0) const;
virtual QRect  subControlRect(QStyle::ComplexControl  cc, const QStyleOptionComplex*  opt, QStyle::SubControl  sc, const QWidget*  widget) const;
virtual QRect  subElementRect(QStyle::SubElement  element, const QStyleOption*  option, const QWidget*  widget) const;
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void unpolish(QApplication*  app);
virtual void unpolish(QWidget*  widget);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QProxyStyle : public QProxyStyle
{ public:
inline void promoted_drawComplexControl(QStyle::ComplexControl  control, const QStyleOptionComplex*  option, QPainter*  painter, const QWidget*  widget = 0) const { QProxyStyle::drawComplexControl(control, option, painter, widget); }
inline void promoted_drawControl(QStyle::ControlElement  element, const QStyleOption*  option, QPainter*  painter, const QWidget*  widget = 0) const { QProxyStyle::drawControl(element, option, painter, widget); }
inline void promoted_drawItemPixmap(QPainter*  painter, const QRect&  rect, int  alignment, const QPixmap&  pixmap) const { QProxyStyle::drawItemPixmap(painter, rect, alignment, pixmap); }
inline void promoted_drawItemText(QPainter*  painter, const QRect&  rect, int  flags, const QPalette&  pal, bool  enabled, const QString&  text, QPalette::ColorRole  textRole = QPalette::NoRole) const { QProxyStyle::drawItemText(painter, rect, flags, pal, enabled, text, textRole); }
inline void promoted_drawPrimitive(QStyle::PrimitiveElement  element, const QStyleOption*  option, QPainter*  painter, const QWidget*  widget = 0) const { QProxyStyle::drawPrimitive(element, option, painter, widget); }
inline bool  promoted_event(QEvent*  e) { return QProxyStyle::event(e); }
inline QPixmap  promoted_generatedIconPixmap(QIcon::Mode  iconMode, const QPixmap&  pixmap, const QStyleOption*  opt) const { return QProxyStyle::generatedIconPixmap(iconMode, pixmap, opt); }
inline QStyle::SubControl  promoted_hitTestComplexControl(QStyle::ComplexControl  control, const QStyleOptionComplex*  option, const QPoint&  pos, const QWidget*  widget = 0) const { return QProxyStyle::hitTestComplexControl(control, option, pos, widget); }
inline QRect  promoted_itemPixmapRect(const QRect&  r, int  flags, const QPixmap&  pixmap) const { return QProxyStyle::itemPixmapRect(r, flags, pixmap); }
inline int  promoted_pixelMetric(QStyle::PixelMetric  metric, const QStyleOption*  option = 0, const QWidget*  widget = 0) const { return QProxyStyle::pixelMetric(metric, option, widget); }
inline void promoted_polish(QApplication*  app) { QProxyStyle::polish(app); }
inline void promoted_polish(QPalette&  pal) { QProxyStyle::polish(pal); }
inline void promoted_polish(QWidget*  widget) { QProxyStyle::polish(widget); }
inline QSize  promoted_sizeFromContents(QStyle::ContentsType  type, const QStyleOption*  option, const QSize&  size, const QWidget*  widget) const { return QProxyStyle::sizeFromContents(type, option, size, widget); }
inline QPalette  promoted_standardPalette() const { return QProxyStyle::standardPalette(); }
inline QPixmap  promoted_standardPixmap(QStyle::StandardPixmap  standardPixmap, const QStyleOption*  opt, const QWidget*  widget = 0) const { return QProxyStyle::standardPixmap(standardPixmap, opt, widget); }
inline int  promoted_styleHint(QStyle::StyleHint  hint, const QStyleOption*  option = 0, const QWidget*  widget = 0, QStyleHintReturn*  returnData = 0) const { return QProxyStyle::styleHint(hint, option, widget, returnData); }
inline QRect  promoted_subControlRect(QStyle::ComplexControl  cc, const QStyleOptionComplex*  opt, QStyle::SubControl  sc, const QWidget*  widget) const { return QProxyStyle::subControlRect(cc, opt, sc, widget); }
inline QRect  promoted_subElementRect(QStyle::SubElement  element, const QStyleOption*  option, const QWidget*  widget) const { return QProxyStyle::subElementRect(element, option, widget); }
inline void promoted_unpolish(QApplication*  app) { QProxyStyle::unpolish(app); }
inline void promoted_unpolish(QWidget*  widget) { QProxyStyle::unpolish(widget); }
};

class PythonQtWrapper_QProxyStyle : public QObject
{ Q_OBJECT
public:
public slots:
QProxyStyle* new_QProxyStyle(QStyle*  baseStyle = 0);
void delete_QProxyStyle(QProxyStyle* obj) { delete obj; } 
   QStyle*  baseStyle(QProxyStyle* theWrappedObject) const;
   void drawComplexControl(QProxyStyle* theWrappedObject, QStyle::ComplexControl  control, const QStyleOptionComplex*  option, QPainter*  painter, const QWidget*  widget = 0) const;
   void drawControl(QProxyStyle* theWrappedObject, QStyle::ControlElement  element, const QStyleOption*  option, QPainter*  painter, const QWidget*  widget = 0) const;
   void drawItemPixmap(QProxyStyle* theWrappedObject, QPainter*  painter, const QRect&  rect, int  alignment, const QPixmap&  pixmap) const;
   void drawItemText(QProxyStyle* theWrappedObject, QPainter*  painter, const QRect&  rect, int  flags, const QPalette&  pal, bool  enabled, const QString&  text, QPalette::ColorRole  textRole = QPalette::NoRole) const;
   void drawPrimitive(QProxyStyle* theWrappedObject, QStyle::PrimitiveElement  element, const QStyleOption*  option, QPainter*  painter, const QWidget*  widget = 0) const;
   bool  event(QProxyStyle* theWrappedObject, QEvent*  e);
   QPixmap  generatedIconPixmap(QProxyStyle* theWrappedObject, QIcon::Mode  iconMode, const QPixmap&  pixmap, const QStyleOption*  opt) const;
   QStyle::SubControl  hitTestComplexControl(QProxyStyle* theWrappedObject, QStyle::ComplexControl  control, const QStyleOptionComplex*  option, const QPoint&  pos, const QWidget*  widget = 0) const;
   QRect  itemPixmapRect(QProxyStyle* theWrappedObject, const QRect&  r, int  flags, const QPixmap&  pixmap) const;
   QRect  itemTextRect(QProxyStyle* theWrappedObject, const QFontMetrics&  fm, const QRect&  r, int  flags, bool  enabled, const QString&  text) const;
   int  pixelMetric(QProxyStyle* theWrappedObject, QStyle::PixelMetric  metric, const QStyleOption*  option = 0, const QWidget*  widget = 0) const;
   void polish(QProxyStyle* theWrappedObject, QApplication*  app);
   void polish(QProxyStyle* theWrappedObject, QPalette&  pal);
   void polish(QProxyStyle* theWrappedObject, QWidget*  widget);
   void setBaseStyle(QProxyStyle* theWrappedObject, QStyle*  style);
   QSize  sizeFromContents(QProxyStyle* theWrappedObject, QStyle::ContentsType  type, const QStyleOption*  option, const QSize&  size, const QWidget*  widget) const;
   QPalette  standardPalette(QProxyStyle* theWrappedObject) const;
   QPixmap  standardPixmap(QProxyStyle* theWrappedObject, QStyle::StandardPixmap  standardPixmap, const QStyleOption*  opt, const QWidget*  widget = 0) const;
   int  styleHint(QProxyStyle* theWrappedObject, QStyle::StyleHint  hint, const QStyleOption*  option = 0, const QWidget*  widget = 0, QStyleHintReturn*  returnData = 0) const;
   QRect  subControlRect(QProxyStyle* theWrappedObject, QStyle::ComplexControl  cc, const QStyleOptionComplex*  opt, QStyle::SubControl  sc, const QWidget*  widget) const;
   QRect  subElementRect(QProxyStyle* theWrappedObject, QStyle::SubElement  element, const QStyleOption*  option, const QWidget*  widget) const;
   void unpolish(QProxyStyle* theWrappedObject, QApplication*  app);
   void unpolish(QProxyStyle* theWrappedObject, QWidget*  widget);
};





class PythonQtShell_QPushButton : public QPushButton
{
public:
    PythonQtShell_QPushButton(QWidget*  parent = 0):QPushButton(parent),_wrapper(NULL) {};
    PythonQtShell_QPushButton(const QIcon&  icon, const QString&  text, QWidget*  parent = 0):QPushButton(icon, text, parent),_wrapper(NULL) {};
    PythonQtShell_QPushButton(const QString&  text, QWidget*  parent = 0):QPushButton(text, parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  e);
virtual void checkStateSet();
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
virtual bool  hitButton(const QPoint&  pos) const;
virtual void inputMethodEvent(QInputMethodEvent*  arg__1);
virtual QVariant  inputMethodQuery(Qt::InputMethodQuery  arg__1) const;
virtual void keyPressEvent(QKeyEvent*  arg__1);
virtual void keyReleaseEvent(QKeyEvent*  e);
virtual void languageChange();
virtual void leaveEvent(QEvent*  arg__1);
virtual int  metric(QPaintDevice::PaintDeviceMetric  arg__1) const;
virtual void mouseDoubleClickEvent(QMouseEvent*  arg__1);
virtual void mouseMoveEvent(QMouseEvent*  e);
virtual void mousePressEvent(QMouseEvent*  e);
virtual void mouseReleaseEvent(QMouseEvent*  e);
virtual void moveEvent(QMoveEvent*  arg__1);
virtual void nextCheckState();
virtual QPaintEngine*  paintEngine() const;
virtual void paintEvent(QPaintEvent*  arg__1);
virtual void resizeEvent(QResizeEvent*  arg__1);
virtual void showEvent(QShowEvent*  arg__1);
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  e);
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QPushButton : public QPushButton
{ public:
inline bool  promoted_event(QEvent*  e) { return QPushButton::event(e); }
inline void promoted_focusInEvent(QFocusEvent*  arg__1) { QPushButton::focusInEvent(arg__1); }
inline void promoted_focusOutEvent(QFocusEvent*  arg__1) { QPushButton::focusOutEvent(arg__1); }
inline void promoted_keyPressEvent(QKeyEvent*  arg__1) { QPushButton::keyPressEvent(arg__1); }
inline void promoted_paintEvent(QPaintEvent*  arg__1) { QPushButton::paintEvent(arg__1); }
};

class PythonQtWrapper_QPushButton : public QObject
{ Q_OBJECT
public:
public slots:
QPushButton* new_QPushButton(QWidget*  parent = 0);
QPushButton* new_QPushButton(const QIcon&  icon, const QString&  text, QWidget*  parent = 0);
QPushButton* new_QPushButton(const QString&  text, QWidget*  parent = 0);
void delete_QPushButton(QPushButton* obj) { delete obj; } 
   bool  autoDefault(QPushButton* theWrappedObject) const;
   bool  event(QPushButton* theWrappedObject, QEvent*  e);
   void focusInEvent(QPushButton* theWrappedObject, QFocusEvent*  arg__1);
   void focusOutEvent(QPushButton* theWrappedObject, QFocusEvent*  arg__1);
   bool  isDefault(QPushButton* theWrappedObject) const;
   bool  isFlat(QPushButton* theWrappedObject) const;
   void keyPressEvent(QPushButton* theWrappedObject, QKeyEvent*  arg__1);
   QMenu*  menu(QPushButton* theWrappedObject) const;
   QSize  minimumSizeHint(QPushButton* theWrappedObject) const;
   void paintEvent(QPushButton* theWrappedObject, QPaintEvent*  arg__1);
   void setAutoDefault(QPushButton* theWrappedObject, bool  arg__1);
   void setDefault(QPushButton* theWrappedObject, bool  arg__1);
   void setFlat(QPushButton* theWrappedObject, bool  arg__1);
   void setMenu(QPushButton* theWrappedObject, QMenu*  menu);
   QSize  sizeHint(QPushButton* theWrappedObject) const;
};





class PythonQtWrapper_QQuaternion : public QObject
{ Q_OBJECT
public:
public slots:
QQuaternion* new_QQuaternion();
QQuaternion* new_QQuaternion(const QVector4D&  vector);
QQuaternion* new_QQuaternion(qreal  scalar, const QVector3D&  vector);
QQuaternion* new_QQuaternion(qreal  scalar, qreal  xpos, qreal  ypos, qreal  zpos);
QQuaternion* new_QQuaternion(const QQuaternion& other) {
QQuaternion* a = new QQuaternion();
*((QQuaternion*)a) = other;
return a; }
void delete_QQuaternion(QQuaternion* obj) { delete obj; } 
   QQuaternion  conjugate(QQuaternion* theWrappedObject) const;
   QQuaternion  static_QQuaternion_fromAxisAndAngle(const QVector3D&  axis, qreal  angle);
   QQuaternion  static_QQuaternion_fromAxisAndAngle(qreal  x, qreal  y, qreal  z, qreal  angle);
   bool  isIdentity(QQuaternion* theWrappedObject) const;
   bool  isNull(QQuaternion* theWrappedObject) const;
   qreal  length(QQuaternion* theWrappedObject) const;
   qreal  lengthSquared(QQuaternion* theWrappedObject) const;
   QQuaternion  static_QQuaternion_nlerp(const QQuaternion&  q1, const QQuaternion&  q2, qreal  t);
   void normalize(QQuaternion* theWrappedObject);
   QQuaternion  normalized(QQuaternion* theWrappedObject) const;
   const QQuaternion  __mul__(QQuaternion* theWrappedObject, const QQuaternion&  q2);
   const QQuaternion  __mul__(QQuaternion* theWrappedObject, qreal  factor);
   QQuaternion*  __imul__(QQuaternion* theWrappedObject, const QQuaternion&  quaternion);
   QQuaternion*  __imul__(QQuaternion* theWrappedObject, qreal  factor);
   const QQuaternion  __add__(QQuaternion* theWrappedObject, const QQuaternion&  q2);
   QQuaternion*  __iadd__(QQuaternion* theWrappedObject, const QQuaternion&  quaternion);
   const QQuaternion  __sub__(QQuaternion* theWrappedObject, const QQuaternion&  q2);
   QQuaternion*  __isub__(QQuaternion* theWrappedObject, const QQuaternion&  quaternion);
   const QQuaternion  __div__(QQuaternion* theWrappedObject, qreal  divisor);
   QQuaternion*  __idiv__(QQuaternion* theWrappedObject, qreal  divisor);
   void writeTo(QQuaternion* theWrappedObject, QDataStream&  arg__1);
   bool  __eq__(QQuaternion* theWrappedObject, const QQuaternion&  q2);
   void readFrom(QQuaternion* theWrappedObject, QDataStream&  arg__1);
   QVector3D  rotatedVector(QQuaternion* theWrappedObject, const QVector3D&  vector) const;
   qreal  scalar(QQuaternion* theWrappedObject) const;
   void setScalar(QQuaternion* theWrappedObject, qreal  scalar);
   void setVector(QQuaternion* theWrappedObject, const QVector3D&  vector);
   void setVector(QQuaternion* theWrappedObject, qreal  x, qreal  y, qreal  z);
   void setX(QQuaternion* theWrappedObject, qreal  x);
   void setY(QQuaternion* theWrappedObject, qreal  y);
   void setZ(QQuaternion* theWrappedObject, qreal  z);
   QQuaternion  static_QQuaternion_slerp(const QQuaternion&  q1, const QQuaternion&  q2, qreal  t);
   QVector4D  toVector4D(QQuaternion* theWrappedObject) const;
   QVector3D  vector(QQuaternion* theWrappedObject) const;
   qreal  x(QQuaternion* theWrappedObject) const;
   qreal  y(QQuaternion* theWrappedObject) const;
   qreal  z(QQuaternion* theWrappedObject) const;
    QString py_toString(QQuaternion*);
    bool __nonzero__(QQuaternion* obj) { return !obj->isNull(); }
};





class PythonQtWrapper_QRadialGradient : public QObject
{ Q_OBJECT
public:
public slots:
QRadialGradient* new_QRadialGradient();
QRadialGradient* new_QRadialGradient(const QPointF&  center, qreal  radius);
QRadialGradient* new_QRadialGradient(const QPointF&  center, qreal  radius, const QPointF&  focalPoint);
QRadialGradient* new_QRadialGradient(qreal  cx, qreal  cy, qreal  radius);
QRadialGradient* new_QRadialGradient(qreal  cx, qreal  cy, qreal  radius, qreal  fx, qreal  fy);
QRadialGradient* new_QRadialGradient(const QRadialGradient& other) {
QRadialGradient* a = new QRadialGradient();
*((QRadialGradient*)a) = other;
return a; }
void delete_QRadialGradient(QRadialGradient* obj) { delete obj; } 
   QPointF  center(QRadialGradient* theWrappedObject) const;
   QPointF  focalPoint(QRadialGradient* theWrappedObject) const;
   qreal  radius(QRadialGradient* theWrappedObject) const;
   void setCenter(QRadialGradient* theWrappedObject, const QPointF&  center);
   void setCenter(QRadialGradient* theWrappedObject, qreal  x, qreal  y);
   void setFocalPoint(QRadialGradient* theWrappedObject, const QPointF&  focalPoint);
   void setFocalPoint(QRadialGradient* theWrappedObject, qreal  x, qreal  y);
   void setRadius(QRadialGradient* theWrappedObject, qreal  radius);
};





class PythonQtShell_QRadioButton : public QRadioButton
{
public:
    PythonQtShell_QRadioButton(QWidget*  parent = 0):QRadioButton(parent),_wrapper(NULL) {};
    PythonQtShell_QRadioButton(const QString&  text, QWidget*  parent = 0):QRadioButton(text, parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  e);
virtual void checkStateSet();
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
virtual void focusInEvent(QFocusEvent*  e);
virtual bool  focusNextPrevChild(bool  next);
virtual void focusOutEvent(QFocusEvent*  e);
virtual int  heightForWidth(int  arg__1) const;
virtual void hideEvent(QHideEvent*  arg__1);
virtual bool  hitButton(const QPoint&  arg__1) const;
virtual void inputMethodEvent(QInputMethodEvent*  arg__1);
virtual QVariant  inputMethodQuery(Qt::InputMethodQuery  arg__1) const;
virtual void keyPressEvent(QKeyEvent*  e);
virtual void keyReleaseEvent(QKeyEvent*  e);
virtual void languageChange();
virtual void leaveEvent(QEvent*  arg__1);
virtual int  metric(QPaintDevice::PaintDeviceMetric  arg__1) const;
virtual QSize  minimumSizeHint() const;
virtual void mouseDoubleClickEvent(QMouseEvent*  arg__1);
virtual void mouseMoveEvent(QMouseEvent*  arg__1);
virtual void mousePressEvent(QMouseEvent*  e);
virtual void mouseReleaseEvent(QMouseEvent*  e);
virtual void moveEvent(QMoveEvent*  arg__1);
virtual void nextCheckState();
virtual QPaintEngine*  paintEngine() const;
virtual void paintEvent(QPaintEvent*  arg__1);
virtual void resizeEvent(QResizeEvent*  arg__1);
virtual void showEvent(QShowEvent*  arg__1);
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  e);
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QRadioButton : public QRadioButton
{ public:
inline bool  promoted_event(QEvent*  e) { return QRadioButton::event(e); }
inline bool  promoted_hitButton(const QPoint&  arg__1) const { return QRadioButton::hitButton(arg__1); }
inline void promoted_mouseMoveEvent(QMouseEvent*  arg__1) { QRadioButton::mouseMoveEvent(arg__1); }
inline void promoted_paintEvent(QPaintEvent*  arg__1) { QRadioButton::paintEvent(arg__1); }
};

class PythonQtWrapper_QRadioButton : public QObject
{ Q_OBJECT
public:
public slots:
QRadioButton* new_QRadioButton(QWidget*  parent = 0);
QRadioButton* new_QRadioButton(const QString&  text, QWidget*  parent = 0);
void delete_QRadioButton(QRadioButton* obj) { delete obj; } 
   bool  event(QRadioButton* theWrappedObject, QEvent*  e);
   bool  hitButton(QRadioButton* theWrappedObject, const QPoint&  arg__1) const;
   void mouseMoveEvent(QRadioButton* theWrappedObject, QMouseEvent*  arg__1);
   void paintEvent(QRadioButton* theWrappedObject, QPaintEvent*  arg__1);
   QSize  sizeHint(QRadioButton* theWrappedObject) const;
};





class PythonQtShell_QRegExpValidator : public QRegExpValidator
{
public:
    PythonQtShell_QRegExpValidator(QObject*  parent):QRegExpValidator(parent),_wrapper(NULL) {};
    PythonQtShell_QRegExpValidator(const QRegExp&  rx, QObject*  parent):QRegExpValidator(rx, parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void fixup(QString&  arg__1) const;
virtual void timerEvent(QTimerEvent*  arg__1);
virtual QValidator::State  validate(QString&  input, int&  pos) const;

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QRegExpValidator : public QRegExpValidator
{ public:
inline QValidator::State  promoted_validate(QString&  input, int&  pos) const { return QRegExpValidator::validate(input, pos); }
};

class PythonQtWrapper_QRegExpValidator : public QObject
{ Q_OBJECT
public:
public slots:
QRegExpValidator* new_QRegExpValidator(QObject*  parent);
QRegExpValidator* new_QRegExpValidator(const QRegExp&  rx, QObject*  parent);
void delete_QRegExpValidator(QRegExpValidator* obj) { delete obj; } 
   const QRegExp*  regExp(QRegExpValidator* theWrappedObject) const;
   void setRegExp(QRegExpValidator* theWrappedObject, const QRegExp&  rx);
   QValidator::State  validate(QRegExpValidator* theWrappedObject, QString&  input, int&  pos) const;
};





class PythonQtShell_QResizeEvent : public QResizeEvent
{
public:
    PythonQtShell_QResizeEvent(const QSize&  size, const QSize&  oldSize):QResizeEvent(size, oldSize),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QResizeEvent : public QObject
{ Q_OBJECT
public:
public slots:
QResizeEvent* new_QResizeEvent(const QSize&  size, const QSize&  oldSize);
void delete_QResizeEvent(QResizeEvent* obj) { delete obj; } 
   const QSize*  oldSize(QResizeEvent* theWrappedObject) const;
   const QSize*  size(QResizeEvent* theWrappedObject) const;
};





class PythonQtShell_QRubberBand : public QRubberBand
{
public:
    PythonQtShell_QRubberBand(QRubberBand::Shape  arg__1, QWidget*  arg__2 = 0):QRubberBand(arg__1, arg__2),_wrapper(NULL) {};

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
virtual QSize  sizeHint() const;
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QRubberBand : public QRubberBand
{ public:
inline void promoted_changeEvent(QEvent*  arg__1) { QRubberBand::changeEvent(arg__1); }
inline bool  promoted_event(QEvent*  e) { return QRubberBand::event(e); }
inline void promoted_moveEvent(QMoveEvent*  arg__1) { QRubberBand::moveEvent(arg__1); }
inline void promoted_paintEvent(QPaintEvent*  arg__1) { QRubberBand::paintEvent(arg__1); }
inline void promoted_resizeEvent(QResizeEvent*  arg__1) { QRubberBand::resizeEvent(arg__1); }
inline void promoted_showEvent(QShowEvent*  arg__1) { QRubberBand::showEvent(arg__1); }
};

class PythonQtWrapper_QRubberBand : public QObject
{ Q_OBJECT
public:
Q_ENUMS(Shape )
enum Shape{
  Line = QRubberBand::Line,   Rectangle = QRubberBand::Rectangle};
public slots:
QRubberBand* new_QRubberBand(QRubberBand::Shape  arg__1, QWidget*  arg__2 = 0);
void delete_QRubberBand(QRubberBand* obj) { delete obj; } 
   void changeEvent(QRubberBand* theWrappedObject, QEvent*  arg__1);
   bool  event(QRubberBand* theWrappedObject, QEvent*  e);
   void move(QRubberBand* theWrappedObject, const QPoint&  p);
   void move(QRubberBand* theWrappedObject, int  x, int  y);
   void moveEvent(QRubberBand* theWrappedObject, QMoveEvent*  arg__1);
   void paintEvent(QRubberBand* theWrappedObject, QPaintEvent*  arg__1);
   void resize(QRubberBand* theWrappedObject, const QSize&  s);
   void resize(QRubberBand* theWrappedObject, int  w, int  h);
   void resizeEvent(QRubberBand* theWrappedObject, QResizeEvent*  arg__1);
   void setGeometry(QRubberBand* theWrappedObject, const QRect&  r);
   void setGeometry(QRubberBand* theWrappedObject, int  x, int  y, int  w, int  h);
   QRubberBand::Shape  shape(QRubberBand* theWrappedObject) const;
   void showEvent(QRubberBand* theWrappedObject, QShowEvent*  arg__1);
};





class PythonQtShell_QScrollArea : public QScrollArea
{
public:
    PythonQtShell_QScrollArea(QWidget*  parent = 0):QScrollArea(parent),_wrapper(NULL) {};

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
virtual void mouseDoubleClickEvent(QMouseEvent*  arg__1);
virtual void mouseMoveEvent(QMouseEvent*  arg__1);
virtual void mousePressEvent(QMouseEvent*  arg__1);
virtual void mouseReleaseEvent(QMouseEvent*  arg__1);
virtual void moveEvent(QMoveEvent*  arg__1);
virtual QPaintEngine*  paintEngine() const;
virtual void paintEvent(QPaintEvent*  arg__1);
virtual void resizeEvent(QResizeEvent*  arg__1);
virtual void scrollContentsBy(int  dx, int  dy);
virtual void showEvent(QShowEvent*  arg__1);
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual bool  viewportEvent(QEvent*  arg__1);
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QScrollArea : public QScrollArea
{ public:
inline bool  promoted_event(QEvent*  arg__1) { return QScrollArea::event(arg__1); }
inline bool  promoted_eventFilter(QObject*  arg__1, QEvent*  arg__2) { return QScrollArea::eventFilter(arg__1, arg__2); }
inline bool  promoted_focusNextPrevChild(bool  next) { return QScrollArea::focusNextPrevChild(next); }
inline void promoted_resizeEvent(QResizeEvent*  arg__1) { QScrollArea::resizeEvent(arg__1); }
inline void promoted_scrollContentsBy(int  dx, int  dy) { QScrollArea::scrollContentsBy(dx, dy); }
};

class PythonQtWrapper_QScrollArea : public QObject
{ Q_OBJECT
public:
public slots:
QScrollArea* new_QScrollArea(QWidget*  parent = 0);
void delete_QScrollArea(QScrollArea* obj) { delete obj; } 
   Qt::Alignment  alignment(QScrollArea* theWrappedObject) const;
   void ensureVisible(QScrollArea* theWrappedObject, int  x, int  y, int  xmargin = 50, int  ymargin = 50);
   void ensureWidgetVisible(QScrollArea* theWrappedObject, QWidget*  childWidget, int  xmargin = 50, int  ymargin = 50);
   bool  event(QScrollArea* theWrappedObject, QEvent*  arg__1);
   bool  eventFilter(QScrollArea* theWrappedObject, QObject*  arg__1, QEvent*  arg__2);
   bool  focusNextPrevChild(QScrollArea* theWrappedObject, bool  next);
   void resizeEvent(QScrollArea* theWrappedObject, QResizeEvent*  arg__1);
   void scrollContentsBy(QScrollArea* theWrappedObject, int  dx, int  dy);
   void setAlignment(QScrollArea* theWrappedObject, Qt::Alignment  arg__1);
   void setWidget(QScrollArea* theWrappedObject, QWidget*  widget);
   void setWidgetResizable(QScrollArea* theWrappedObject, bool  resizable);
   QSize  sizeHint(QScrollArea* theWrappedObject) const;
   QWidget*  takeWidget(QScrollArea* theWrappedObject);
   QWidget*  widget(QScrollArea* theWrappedObject) const;
   bool  widgetResizable(QScrollArea* theWrappedObject) const;
};





class PythonQtShell_QScrollBar : public QScrollBar
{
public:
    PythonQtShell_QScrollBar(QWidget*  parent = 0):QScrollBar(parent),_wrapper(NULL) {};
    PythonQtShell_QScrollBar(Qt::Orientation  arg__1, QWidget*  parent = 0):QScrollBar(arg__1, parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  e);
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
virtual bool  event(QEvent*  event);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void focusInEvent(QFocusEvent*  arg__1);
virtual bool  focusNextPrevChild(bool  next);
virtual void focusOutEvent(QFocusEvent*  arg__1);
virtual int  heightForWidth(int  arg__1) const;
virtual void hideEvent(QHideEvent*  arg__1);
virtual void inputMethodEvent(QInputMethodEvent*  arg__1);
virtual QVariant  inputMethodQuery(Qt::InputMethodQuery  arg__1) const;
virtual void keyPressEvent(QKeyEvent*  ev);
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
virtual void wheelEvent(QWheelEvent*  e);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QScrollBar : public QScrollBar
{ public:
inline void promoted_contextMenuEvent(QContextMenuEvent*  arg__1) { QScrollBar::contextMenuEvent(arg__1); }
inline bool  promoted_event(QEvent*  event) { return QScrollBar::event(event); }
inline void promoted_hideEvent(QHideEvent*  arg__1) { QScrollBar::hideEvent(arg__1); }
inline void promoted_mouseMoveEvent(QMouseEvent*  arg__1) { QScrollBar::mouseMoveEvent(arg__1); }
inline void promoted_mousePressEvent(QMouseEvent*  arg__1) { QScrollBar::mousePressEvent(arg__1); }
inline void promoted_mouseReleaseEvent(QMouseEvent*  arg__1) { QScrollBar::mouseReleaseEvent(arg__1); }
inline void promoted_paintEvent(QPaintEvent*  arg__1) { QScrollBar::paintEvent(arg__1); }
};

class PythonQtWrapper_QScrollBar : public QObject
{ Q_OBJECT
public:
public slots:
QScrollBar* new_QScrollBar(QWidget*  parent = 0);
QScrollBar* new_QScrollBar(Qt::Orientation  arg__1, QWidget*  parent = 0);
void delete_QScrollBar(QScrollBar* obj) { delete obj; } 
   void contextMenuEvent(QScrollBar* theWrappedObject, QContextMenuEvent*  arg__1);
   bool  event(QScrollBar* theWrappedObject, QEvent*  event);
   void hideEvent(QScrollBar* theWrappedObject, QHideEvent*  arg__1);
   void mouseMoveEvent(QScrollBar* theWrappedObject, QMouseEvent*  arg__1);
   void mousePressEvent(QScrollBar* theWrappedObject, QMouseEvent*  arg__1);
   void mouseReleaseEvent(QScrollBar* theWrappedObject, QMouseEvent*  arg__1);
   void paintEvent(QScrollBar* theWrappedObject, QPaintEvent*  arg__1);
   QSize  sizeHint(QScrollBar* theWrappedObject) const;
};





class PythonQtWrapper_QSessionManager : public QObject
{ Q_OBJECT
public:
Q_ENUMS(RestartHint )
enum RestartHint{
  RestartIfRunning = QSessionManager::RestartIfRunning,   RestartAnyway = QSessionManager::RestartAnyway,   RestartImmediately = QSessionManager::RestartImmediately,   RestartNever = QSessionManager::RestartNever};
public slots:
   bool  allowsErrorInteraction(QSessionManager* theWrappedObject);
   bool  allowsInteraction(QSessionManager* theWrappedObject);
   void cancel(QSessionManager* theWrappedObject);
   QStringList  discardCommand(QSessionManager* theWrappedObject) const;
   bool  isPhase2(QSessionManager* theWrappedObject) const;
   void release(QSessionManager* theWrappedObject);
   void requestPhase2(QSessionManager* theWrappedObject);
   QStringList  restartCommand(QSessionManager* theWrappedObject) const;
   QSessionManager::RestartHint  restartHint(QSessionManager* theWrappedObject) const;
   QString  sessionId(QSessionManager* theWrappedObject) const;
   QString  sessionKey(QSessionManager* theWrappedObject) const;
   void setDiscardCommand(QSessionManager* theWrappedObject, const QStringList&  arg__1);
   void setManagerProperty(QSessionManager* theWrappedObject, const QString&  name, const QString&  value);
   void setManagerProperty(QSessionManager* theWrappedObject, const QString&  name, const QStringList&  value);
   void setRestartCommand(QSessionManager* theWrappedObject, const QStringList&  arg__1);
   void setRestartHint(QSessionManager* theWrappedObject, QSessionManager::RestartHint  arg__1);
};





class PythonQtShell_QShortcut : public QShortcut
{
public:
    PythonQtShell_QShortcut(QWidget*  parent):QShortcut(parent),_wrapper(NULL) {};
    PythonQtShell_QShortcut(const QKeySequence&  key, QWidget*  parent, const char*  member = 0, const char*  ambiguousMember = 0, Qt::ShortcutContext  context = Qt::WindowShortcut):QShortcut(key, parent, member, ambiguousMember, context),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  e);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QShortcut : public QShortcut
{ public:
inline bool  promoted_event(QEvent*  e) { return QShortcut::event(e); }
};

class PythonQtWrapper_QShortcut : public QObject
{ Q_OBJECT
public:
public slots:
QShortcut* new_QShortcut(QWidget*  parent);
QShortcut* new_QShortcut(const QKeySequence&  key, QWidget*  parent, const char*  member = 0, const char*  ambiguousMember = 0, Qt::ShortcutContext  context = Qt::WindowShortcut);
void delete_QShortcut(QShortcut* obj) { delete obj; } 
   bool  autoRepeat(QShortcut* theWrappedObject) const;
   Qt::ShortcutContext  context(QShortcut* theWrappedObject);
   bool  event(QShortcut* theWrappedObject, QEvent*  e);
   int  id(QShortcut* theWrappedObject) const;
   bool  isEnabled(QShortcut* theWrappedObject) const;
   QKeySequence  key(QShortcut* theWrappedObject) const;
   QWidget*  parentWidget(QShortcut* theWrappedObject) const;
   void setAutoRepeat(QShortcut* theWrappedObject, bool  on);
   void setContext(QShortcut* theWrappedObject, Qt::ShortcutContext  context);
   void setEnabled(QShortcut* theWrappedObject, bool  enable);
   void setKey(QShortcut* theWrappedObject, const QKeySequence&  key);
   void setWhatsThis(QShortcut* theWrappedObject, const QString&  text);
   QString  whatsThis(QShortcut* theWrappedObject) const;
};





class PythonQtShell_QShortcutEvent : public QShortcutEvent
{
public:
    PythonQtShell_QShortcutEvent(const QKeySequence&  key, int  id, bool  ambiguous = false):QShortcutEvent(key, id, ambiguous),_wrapper(NULL) {};


  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QShortcutEvent : public QObject
{ Q_OBJECT
public:
public slots:
QShortcutEvent* new_QShortcutEvent(const QKeySequence&  key, int  id, bool  ambiguous = false);
void delete_QShortcutEvent(QShortcutEvent* obj) { delete obj; } 
   bool  isAmbiguous(QShortcutEvent* theWrappedObject) const;
   const QKeySequence*  key(QShortcutEvent* theWrappedObject) const;
   int  shortcutId(QShortcutEvent* theWrappedObject) const;
};





class PythonQtWrapper_QShowEvent : public QObject
{ Q_OBJECT
public:
public slots:
QShowEvent* new_QShowEvent();
void delete_QShowEvent(QShowEvent* obj) { delete obj; } 
};





class PythonQtShell_QSizeGrip : public QSizeGrip
{
public:
    PythonQtShell_QSizeGrip(QWidget*  parent):QSizeGrip(parent),_wrapper(NULL) {};

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
virtual void hideEvent(QHideEvent*  hideEvent);
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
virtual void mouseReleaseEvent(QMouseEvent*  mouseEvent);
virtual void moveEvent(QMoveEvent*  moveEvent);
virtual QPaintEngine*  paintEngine() const;
virtual void paintEvent(QPaintEvent*  arg__1);
virtual void resizeEvent(QResizeEvent*  arg__1);
virtual void showEvent(QShowEvent*  showEvent);
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QSizeGrip : public QSizeGrip
{ public:
inline bool  promoted_event(QEvent*  arg__1) { return QSizeGrip::event(arg__1); }
inline bool  promoted_eventFilter(QObject*  arg__1, QEvent*  arg__2) { return QSizeGrip::eventFilter(arg__1, arg__2); }
inline void promoted_hideEvent(QHideEvent*  hideEvent) { QSizeGrip::hideEvent(hideEvent); }
inline void promoted_mouseMoveEvent(QMouseEvent*  arg__1) { QSizeGrip::mouseMoveEvent(arg__1); }
inline void promoted_mousePressEvent(QMouseEvent*  arg__1) { QSizeGrip::mousePressEvent(arg__1); }
inline void promoted_mouseReleaseEvent(QMouseEvent*  mouseEvent) { QSizeGrip::mouseReleaseEvent(mouseEvent); }
inline void promoted_moveEvent(QMoveEvent*  moveEvent) { QSizeGrip::moveEvent(moveEvent); }
inline void promoted_paintEvent(QPaintEvent*  arg__1) { QSizeGrip::paintEvent(arg__1); }
inline void promoted_showEvent(QShowEvent*  showEvent) { QSizeGrip::showEvent(showEvent); }
};

class PythonQtWrapper_QSizeGrip : public QObject
{ Q_OBJECT
public:
public slots:
QSizeGrip* new_QSizeGrip(QWidget*  parent);
void delete_QSizeGrip(QSizeGrip* obj) { delete obj; } 
   bool  event(QSizeGrip* theWrappedObject, QEvent*  arg__1);
   bool  eventFilter(QSizeGrip* theWrappedObject, QObject*  arg__1, QEvent*  arg__2);
   void hideEvent(QSizeGrip* theWrappedObject, QHideEvent*  hideEvent);
   void mouseMoveEvent(QSizeGrip* theWrappedObject, QMouseEvent*  arg__1);
   void mousePressEvent(QSizeGrip* theWrappedObject, QMouseEvent*  arg__1);
   void mouseReleaseEvent(QSizeGrip* theWrappedObject, QMouseEvent*  mouseEvent);
   void moveEvent(QSizeGrip* theWrappedObject, QMoveEvent*  moveEvent);
   void paintEvent(QSizeGrip* theWrappedObject, QPaintEvent*  arg__1);
   void setVisible(QSizeGrip* theWrappedObject, bool  arg__1);
   void showEvent(QSizeGrip* theWrappedObject, QShowEvent*  showEvent);
   QSize  sizeHint(QSizeGrip* theWrappedObject) const;
};





class PythonQtShell_QSlider : public QSlider
{
public:
    PythonQtShell_QSlider(QWidget*  parent = 0):QSlider(parent),_wrapper(NULL) {};
    PythonQtShell_QSlider(Qt::Orientation  orientation, QWidget*  parent = 0):QSlider(orientation, parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  e);
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
virtual bool  event(QEvent*  event);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void focusInEvent(QFocusEvent*  arg__1);
virtual bool  focusNextPrevChild(bool  next);
virtual void focusOutEvent(QFocusEvent*  arg__1);
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
virtual void paintEvent(QPaintEvent*  ev);
virtual void resizeEvent(QResizeEvent*  arg__1);
virtual void showEvent(QShowEvent*  arg__1);
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void wheelEvent(QWheelEvent*  e);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QSlider : public QSlider
{ public:
inline bool  promoted_event(QEvent*  event) { return QSlider::event(event); }
inline void promoted_mouseMoveEvent(QMouseEvent*  ev) { QSlider::mouseMoveEvent(ev); }
inline void promoted_mousePressEvent(QMouseEvent*  ev) { QSlider::mousePressEvent(ev); }
inline void promoted_mouseReleaseEvent(QMouseEvent*  ev) { QSlider::mouseReleaseEvent(ev); }
inline void promoted_paintEvent(QPaintEvent*  ev) { QSlider::paintEvent(ev); }
};

class PythonQtWrapper_QSlider : public QObject
{ Q_OBJECT
public:
public slots:
QSlider* new_QSlider(QWidget*  parent = 0);
QSlider* new_QSlider(Qt::Orientation  orientation, QWidget*  parent = 0);
void delete_QSlider(QSlider* obj) { delete obj; } 
   bool  event(QSlider* theWrappedObject, QEvent*  event);
   QSize  minimumSizeHint(QSlider* theWrappedObject) const;
   void mouseMoveEvent(QSlider* theWrappedObject, QMouseEvent*  ev);
   void mousePressEvent(QSlider* theWrappedObject, QMouseEvent*  ev);
   void mouseReleaseEvent(QSlider* theWrappedObject, QMouseEvent*  ev);
   void paintEvent(QSlider* theWrappedObject, QPaintEvent*  ev);
   void setTickInterval(QSlider* theWrappedObject, int  ti);
   void setTickPosition(QSlider* theWrappedObject, QSlider::TickPosition  position);
   QSize  sizeHint(QSlider* theWrappedObject) const;
   int  tickInterval(QSlider* theWrappedObject) const;
   QSlider::TickPosition  tickPosition(QSlider* theWrappedObject) const;
};





class PythonQtShell_QSound : public QSound
{
public:
    PythonQtShell_QSound(const QString&  filename, QObject*  parent = 0):QSound(filename, parent),_wrapper(NULL) {};

virtual void childEvent(QChildEvent*  arg__1);
virtual void customEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  arg__1);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtWrapper_QSound : public QObject
{ Q_OBJECT
public:
public slots:
QSound* new_QSound(const QString&  filename, QObject*  parent = 0);
void delete_QSound(QSound* obj) { delete obj; } 
   QString  fileName(QSound* theWrappedObject) const;
   bool  static_QSound_isAvailable();
   bool  isFinished(QSound* theWrappedObject) const;
   int  loops(QSound* theWrappedObject) const;
   int  loopsRemaining(QSound* theWrappedObject) const;
   void static_QSound_play(const QString&  filename);
   void setLoops(QSound* theWrappedObject, int  arg__1);
};





class PythonQtShell_QSpacerItem : public QSpacerItem
{
public:
    PythonQtShell_QSpacerItem(int  w, int  h, QSizePolicy::Policy  hData = QSizePolicy::Minimum, QSizePolicy::Policy  vData = QSizePolicy::Minimum):QSpacerItem(w, h, hData, vData),_wrapper(NULL) {};

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

class PythonQtPublicPromoter_QSpacerItem : public QSpacerItem
{ public:
inline Qt::Orientations  promoted_expandingDirections() const { return QSpacerItem::expandingDirections(); }
inline QRect  promoted_geometry() const { return QSpacerItem::geometry(); }
inline bool  promoted_isEmpty() const { return QSpacerItem::isEmpty(); }
inline QSize  promoted_maximumSize() const { return QSpacerItem::maximumSize(); }
inline QSize  promoted_minimumSize() const { return QSpacerItem::minimumSize(); }
inline void promoted_setGeometry(const QRect&  arg__1) { QSpacerItem::setGeometry(arg__1); }
inline QSize  promoted_sizeHint() const { return QSpacerItem::sizeHint(); }
inline QSpacerItem*  promoted_spacerItem() { return QSpacerItem::spacerItem(); }
};

class PythonQtWrapper_QSpacerItem : public QObject
{ Q_OBJECT
public:
public slots:
QSpacerItem* new_QSpacerItem(int  w, int  h, QSizePolicy::Policy  hData = QSizePolicy::Minimum, QSizePolicy::Policy  vData = QSizePolicy::Minimum);
void delete_QSpacerItem(QSpacerItem* obj) { delete obj; } 
   void changeSize(QSpacerItem* theWrappedObject, int  w, int  h, QSizePolicy::Policy  hData = QSizePolicy::Minimum, QSizePolicy::Policy  vData = QSizePolicy::Minimum);
   Qt::Orientations  expandingDirections(QSpacerItem* theWrappedObject) const;
   QRect  geometry(QSpacerItem* theWrappedObject) const;
   bool  isEmpty(QSpacerItem* theWrappedObject) const;
   QSize  maximumSize(QSpacerItem* theWrappedObject) const;
   QSize  minimumSize(QSpacerItem* theWrappedObject) const;
   void setGeometry(QSpacerItem* theWrappedObject, const QRect&  arg__1);
   QSize  sizeHint(QSpacerItem* theWrappedObject) const;
   QSpacerItem*  spacerItem(QSpacerItem* theWrappedObject);
};





class PythonQtShell_QSpinBox : public QSpinBox
{
public:
    PythonQtShell_QSpinBox(QWidget*  parent = 0):QSpinBox(parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  event);
virtual void childEvent(QChildEvent*  arg__1);
virtual void clear();
virtual void closeEvent(QCloseEvent*  event);
virtual void contextMenuEvent(QContextMenuEvent*  event);
virtual void customEvent(QEvent*  arg__1);
virtual int  devType() const;
virtual void dragEnterEvent(QDragEnterEvent*  arg__1);
virtual void dragLeaveEvent(QDragLeaveEvent*  arg__1);
virtual void dragMoveEvent(QDragMoveEvent*  arg__1);
virtual void dropEvent(QDropEvent*  arg__1);
virtual void enterEvent(QEvent*  arg__1);
virtual bool  event(QEvent*  event);
virtual bool  eventFilter(QObject*  arg__1, QEvent*  arg__2);
virtual void fixup(QString&  str) const;
virtual void focusInEvent(QFocusEvent*  event);
virtual bool  focusNextPrevChild(bool  next);
virtual void focusOutEvent(QFocusEvent*  event);
virtual int  heightForWidth(int  arg__1) const;
virtual void hideEvent(QHideEvent*  event);
virtual void inputMethodEvent(QInputMethodEvent*  arg__1);
virtual QVariant  inputMethodQuery(Qt::InputMethodQuery  arg__1) const;
virtual void keyPressEvent(QKeyEvent*  event);
virtual void keyReleaseEvent(QKeyEvent*  event);
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
virtual void showEvent(QShowEvent*  event);
virtual void stepBy(int  steps);
virtual QAbstractSpinBox::StepEnabled  stepEnabled() const;
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual QString  textFromValue(int  val) const;
virtual void timerEvent(QTimerEvent*  event);
virtual QValidator::State  validate(QString&  input, int&  pos) const;
virtual int  valueFromText(const QString&  text) const;
virtual void wheelEvent(QWheelEvent*  event);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QSpinBox : public QSpinBox
{ public:
inline bool  promoted_event(QEvent*  event) { return QSpinBox::event(event); }
inline void promoted_fixup(QString&  str) const { QSpinBox::fixup(str); }
inline QString  promoted_textFromValue(int  val) const { return QSpinBox::textFromValue(val); }
inline QValidator::State  promoted_validate(QString&  input, int&  pos) const { return QSpinBox::validate(input, pos); }
inline int  promoted_valueFromText(const QString&  text) const { return QSpinBox::valueFromText(text); }
};

class PythonQtWrapper_QSpinBox : public QObject
{ Q_OBJECT
public:
public slots:
QSpinBox* new_QSpinBox(QWidget*  parent = 0);
void delete_QSpinBox(QSpinBox* obj) { delete obj; } 
   QString  cleanText(QSpinBox* theWrappedObject) const;
   bool  event(QSpinBox* theWrappedObject, QEvent*  event);
   void fixup(QSpinBox* theWrappedObject, QString&  str) const;
   int  maximum(QSpinBox* theWrappedObject) const;
   int  minimum(QSpinBox* theWrappedObject) const;
   QString  prefix(QSpinBox* theWrappedObject) const;
   void setMaximum(QSpinBox* theWrappedObject, int  max);
   void setMinimum(QSpinBox* theWrappedObject, int  min);
   void setPrefix(QSpinBox* theWrappedObject, const QString&  prefix);
   void setRange(QSpinBox* theWrappedObject, int  min, int  max);
   void setSingleStep(QSpinBox* theWrappedObject, int  val);
   void setSuffix(QSpinBox* theWrappedObject, const QString&  suffix);
   int  singleStep(QSpinBox* theWrappedObject) const;
   QString  suffix(QSpinBox* theWrappedObject) const;
   QString  textFromValue(QSpinBox* theWrappedObject, int  val) const;
   QValidator::State  validate(QSpinBox* theWrappedObject, QString&  input, int&  pos) const;
   int  value(QSpinBox* theWrappedObject) const;
   int  valueFromText(QSpinBox* theWrappedObject, const QString&  text) const;
};





class PythonQtShell_QSplashScreen : public QSplashScreen
{
public:
    PythonQtShell_QSplashScreen(QWidget*  parent, const QPixmap&  pixmap = QPixmap(), Qt::WindowFlags  f = 0):QSplashScreen(parent, pixmap, f),_wrapper(NULL) {};
    PythonQtShell_QSplashScreen(const QPixmap&  pixmap = QPixmap(), Qt::WindowFlags  f = 0):QSplashScreen(pixmap, f),_wrapper(NULL) {};

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
virtual void drawContents(QPainter*  painter);
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
virtual QSize  sizeHint() const;
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QSplashScreen : public QSplashScreen
{ public:
inline void promoted_drawContents(QPainter*  painter) { QSplashScreen::drawContents(painter); }
inline bool  promoted_event(QEvent*  e) { return QSplashScreen::event(e); }
inline void promoted_mousePressEvent(QMouseEvent*  arg__1) { QSplashScreen::mousePressEvent(arg__1); }
};

class PythonQtWrapper_QSplashScreen : public QObject
{ Q_OBJECT
public:
public slots:
QSplashScreen* new_QSplashScreen(QWidget*  parent, const QPixmap&  pixmap = QPixmap(), Qt::WindowFlags  f = 0);
QSplashScreen* new_QSplashScreen(const QPixmap&  pixmap = QPixmap(), Qt::WindowFlags  f = 0);
void delete_QSplashScreen(QSplashScreen* obj) { delete obj; } 
   void drawContents(QSplashScreen* theWrappedObject, QPainter*  painter);
   bool  event(QSplashScreen* theWrappedObject, QEvent*  e);
   void finish(QSplashScreen* theWrappedObject, QWidget*  w);
   void mousePressEvent(QSplashScreen* theWrappedObject, QMouseEvent*  arg__1);
   const QPixmap  pixmap(QSplashScreen* theWrappedObject) const;
   void setPixmap(QSplashScreen* theWrappedObject, const QPixmap&  pixmap);
};





class PythonQtShell_QSplitter : public QSplitter
{
public:
    PythonQtShell_QSplitter(QWidget*  parent = 0):QSplitter(parent),_wrapper(NULL) {};
    PythonQtShell_QSplitter(Qt::Orientation  arg__1, QWidget*  parent = 0):QSplitter(arg__1, parent),_wrapper(NULL) {};

virtual void actionEvent(QActionEvent*  arg__1);
virtual void changeEvent(QEvent*  arg__1);
virtual void childEvent(QChildEvent*  arg__1);
virtual void closeEvent(QCloseEvent*  arg__1);
virtual void contextMenuEvent(QContextMenuEvent*  arg__1);
virtual QSplitterHandle*  createHandle();
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

class PythonQtPublicPromoter_QSplitter : public QSplitter
{ public:
inline void promoted_changeEvent(QEvent*  arg__1) { QSplitter::changeEvent(arg__1); }
inline void promoted_childEvent(QChildEvent*  arg__1) { QSplitter::childEvent(arg__1); }
inline QSplitterHandle*  promoted_createHandle() { return QSplitter::createHandle(); }
inline bool  promoted_event(QEvent*  arg__1) { return QSplitter::event(arg__1); }
inline void promoted_resizeEvent(QResizeEvent*  arg__1) { QSplitter::resizeEvent(arg__1); }
};

class PythonQtWrapper_QSplitter : public QObject
{ Q_OBJECT
public:
public slots:
QSplitter* new_QSplitter(QWidget*  parent = 0);
QSplitter* new_QSplitter(Qt::Orientation  arg__1, QWidget*  parent = 0);
void delete_QSplitter(QSplitter* obj) { delete obj; } 
   void addWidget(QSplitter* theWrappedObject, QWidget*  widget);
   void changeEvent(QSplitter* theWrappedObject, QEvent*  arg__1);
   void childEvent(QSplitter* theWrappedObject, QChildEvent*  arg__1);
   bool  childrenCollapsible(QSplitter* theWrappedObject) const;
   int  count(QSplitter* theWrappedObject) const;
   QSplitterHandle*  createHandle(QSplitter* theWrappedObject);
   bool  event(QSplitter* theWrappedObject, QEvent*  arg__1);
   void getRange(QSplitter* theWrappedObject, int  index, int*  arg__2, int*  arg__3) const;
   QSplitterHandle*  handle(QSplitter* theWrappedObject, int  index) const;
   int  handleWidth(QSplitter* theWrappedObject) const;
   int  indexOf(QSplitter* theWrappedObject, QWidget*  w) const;
   void insertWidget(QSplitter* theWrappedObject, int  index, QWidget*  widget);
   bool  isCollapsible(QSplitter* theWrappedObject, int  index) const;
   QSize  minimumSizeHint(QSplitter* theWrappedObject) const;
   bool  opaqueResize(QSplitter* theWrappedObject) const;
   void writeTo(QSplitter* theWrappedObject, QTextStream&  arg__1);
   void readFrom(QSplitter* theWrappedObject, QTextStream&  arg__1);
   Qt::Orientation  orientation(QSplitter* theWrappedObject) const;
   void refresh(QSplitter* theWrappedObject);
   void resizeEvent(QSplitter* theWrappedObject, QResizeEvent*  arg__1);
   bool  restoreState(QSplitter* theWrappedObject, const QByteArray&  state);
   QByteArray  saveState(QSplitter* theWrappedObject) const;
   void setChildrenCollapsible(QSplitter* theWrappedObject, bool  arg__1);
   void setCollapsible(QSplitter* theWrappedObject, int  index, bool  arg__2);
   void setHandleWidth(QSplitter* theWrappedObject, int  arg__1);
   void setOpaqueResize(QSplitter* theWrappedObject, bool  opaque = true);
   void setOrientation(QSplitter* theWrappedObject, Qt::Orientation  arg__1);
   void setSizes(QSplitter* theWrappedObject, const QList<int >&  list);
   void setStretchFactor(QSplitter* theWrappedObject, int  index, int  stretch);
   QSize  sizeHint(QSplitter* theWrappedObject) const;
   QList<int >  sizes(QSplitter* theWrappedObject) const;
   QWidget*  widget(QSplitter* theWrappedObject, int  index) const;
};





class PythonQtShell_QSplitterHandle : public QSplitterHandle
{
public:
    PythonQtShell_QSplitterHandle(Qt::Orientation  o, QSplitter*  parent):QSplitterHandle(o, parent),_wrapper(NULL) {};

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
virtual void tabletEvent(QTabletEvent*  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);
virtual void wheelEvent(QWheelEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QSplitterHandle : public QSplitterHandle
{ public:
inline bool  promoted_event(QEvent*  arg__1) { return QSplitterHandle::event(arg__1); }
inline void promoted_mouseMoveEvent(QMouseEvent*  arg__1) { QSplitterHandle::mouseMoveEvent(arg__1); }
inline void promoted_mousePressEvent(QMouseEvent*  arg__1) { QSplitterHandle::mousePressEvent(arg__1); }
inline void promoted_mouseReleaseEvent(QMouseEvent*  arg__1) { QSplitterHandle::mouseReleaseEvent(arg__1); }
inline void promoted_paintEvent(QPaintEvent*  arg__1) { QSplitterHandle::paintEvent(arg__1); }
};

class PythonQtWrapper_QSplitterHandle : public QObject
{ Q_OBJECT
public:
public slots:
QSplitterHandle* new_QSplitterHandle(Qt::Orientation  o, QSplitter*  parent);
void delete_QSplitterHandle(QSplitterHandle* obj) { delete obj; } 
   bool  event(QSplitterHandle* theWrappedObject, QEvent*  arg__1);
   void mouseMoveEvent(QSplitterHandle* theWrappedObject, QMouseEvent*  arg__1);
   void mousePressEvent(QSplitterHandle* theWrappedObject, QMouseEvent*  arg__1);
   void mouseReleaseEvent(QSplitterHandle* theWrappedObject, QMouseEvent*  arg__1);
   bool  opaqueResize(QSplitterHandle* theWrappedObject) const;
   Qt::Orientation  orientation(QSplitterHandle* theWrappedObject) const;
   void paintEvent(QSplitterHandle* theWrappedObject, QPaintEvent*  arg__1);
   void setOrientation(QSplitterHandle* theWrappedObject, Qt::Orientation  o);
   QSize  sizeHint(QSplitterHandle* theWrappedObject) const;
   QSplitter*  splitter(QSplitterHandle* theWrappedObject) const;
};





class PythonQtShell_QStackedLayout : public QStackedLayout
{
public:
    PythonQtShell_QStackedLayout():QStackedLayout(),_wrapper(NULL) {};
    PythonQtShell_QStackedLayout(QLayout*  parentLayout):QStackedLayout(parentLayout),_wrapper(NULL) {};
    PythonQtShell_QStackedLayout(QWidget*  parent):QStackedLayout(parent),_wrapper(NULL) {};

virtual void addItem(QLayoutItem*  item);
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
virtual void setGeometry(const QRect&  rect);
virtual QLayoutItem*  takeAt(int  arg__1);
virtual void timerEvent(QTimerEvent*  arg__1);

  PythonQtInstanceWrapper* _wrapper; 
};

class PythonQtPublicPromoter_QStackedLayout : public QStackedLayout
{ public:
inline void promoted_addItem(QLayoutItem*  item) { QStackedLayout::addItem(item); }
inline int  promoted_count() const { return QStackedLayout::count(); }
inline QLayoutItem*  promoted_itemAt(int  arg__1) const { return QStackedLayout::itemAt(arg__1); }
inline QSize  promoted_minimumSize() const { return QStackedLayout::minimumSize(); }
inline void promoted_setGeometry(const QRect&  rect) { QStackedLayout::setGeometry(rect); }
inline QLayoutItem*  promoted_takeAt(int  arg__1) { return QStackedLayout::takeAt(arg__1); }
};

class PythonQtWrapper_QStackedLayout : public QObject
{ Q_OBJECT
public:
public slots:
QStackedLayout* new_QStackedLayout();
QStackedLayout* new_QStackedLayout(QLayout*  parentLayout);
QStackedLayout* new_QStackedLayout(QWidget*  parent);
void delete_QStackedLayout(QStackedLayout* obj) { delete obj; } 
   void addItem(QStackedLayout* theWrappedObject, QLayoutItem*  item);
   int  addWidget(QStackedLayout* theWrappedObject, QWidget*  w);
   int  count(QStackedLayout* theWrappedObject) const;
   int  currentIndex(QStackedLayout* theWrappedObject) const;
   QWidget*  currentWidget(QStackedLayout* theWrappedObject) const;
   int  insertWidget(QStackedLayout* theWrappedObject, int  index, QWidget*  w);
   QLayoutItem*  itemAt(QStackedLayout* theWrappedObject, int  arg__1) const;
   QSize  minimumSize(QStackedLayout* theWrappedObject) const;
   void setGeometry(QStackedLayout* theWrappedObject, const QRect&  rect);
   void setStackingMode(QStackedLayout* theWrappedObject, QStackedLayout::StackingMode  stackingMode);
   QSize  sizeHint(QStackedLayout* theWrappedObject) const;
   QStackedLayout::StackingMode  stackingMode(QStackedLayout* theWrappedObject) const;
   QLayoutItem*  takeAt(QStackedLayout* theWrappedObject, int  arg__1);
   QWidget*  widget(QStackedLayout* theWrappedObject);
   QWidget*  widget(QStackedLayout* theWrappedObject, int  arg__1) const;
};


