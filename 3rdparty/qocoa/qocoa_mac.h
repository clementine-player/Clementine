/*
Copyright (C) 2011 by Mike McQuaid

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <AppKit/NSImage.h>
#include <AppKit/NSView.h>
#include <Foundation/NSString.h>
#include <QEvent>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

// Qt5's QtMacExtras module (and QMacCocoaViewContainer with it) has no Qt6
// equivalent, so this is a small local replacement: force the QWidget to get
// a native NSView (via WA_NativeWindow + winId()), then add the wrapped
// Cocoa view as a subview and let AppKit's own autoresizing mask keep it
// sized to match, rather than reimplementing QMacCocoaViewContainer's
// internal resize-event plumbing.
//
// The container keeps its own reference to the wrapped view, as Qt5's did:
// the widgets that use it release theirs once it's set up. Qt destroys and
// recreates a widget's native view when, for example, the widget is
// reparented, and the old view's subviews go with it; without that
// reference, the wrapped view would be freed while the widget still points
// at it. When the native view changes, the wrapped view moves to the new one.
class QMacCocoaViewContainer : public QWidget {
public:
  explicit QMacCocoaViewContainer(void* cocoaViewToWrap, QWidget* parent = nullptr)
      : QWidget(parent), cocoaView_(static_cast<NSView*>(cocoaViewToWrap)) {
    [cocoaView_ retain];
    setAttribute(Qt::WA_NativeWindow);
    attach(winId());
  }

  ~QMacCocoaViewContainer() override {
    [cocoaView_ removeFromSuperview];
    [cocoaView_ release];
  }

  QSize sizeHint() const override {
    const NSSize fitting = [cocoaView_ fittingSize];
    return QSize(fitting.width, fitting.height);
  }

protected:
  bool event(QEvent* e) override {
    // Also sent when the native view goes, when there's none to move to.
    if (e->type() == QEvent::WinIdChange && internalWinId()) attach(internalWinId());
    return QWidget::event(e);
  }

private:
  // Puts the wrapped view in this widget's native view, [id].
  void attach(WId id) {
    NSView* qtView = reinterpret_cast<NSView*>(id);
    if ([cocoaView_ superview] == qtView) return;
    [cocoaView_ removeFromSuperview];
    [qtView addSubview:cocoaView_];
    [cocoaView_ setFrame:[qtView bounds]];
    [cocoaView_ setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
  }

  NSView* cocoaView_;
};

static inline NSString* fromQString(const QString &string)
{
    const QByteArray utf8 = string.toUtf8();
    const char* cString = utf8.constData();
    return [[NSString alloc] initWithUTF8String:cString];
}

static inline QString toQString(NSString *string)
{
    if (!string)
        return QString();
    return QString::fromUtf8([string UTF8String]);
}

static inline NSImage* fromQPixmap(const QPixmap &pixmap)
{
  CGImageRef cgImage = pixmap.toImage().toCGImage();
  return [[NSImage alloc] initWithCGImage:cgImage size:NSZeroSize];
}

static inline void setupLayout(NSView* cocoaView, QWidget* parent) {
  parent->setAttribute(Qt::WA_NativeWindow);
  QVBoxLayout* layout = new QVBoxLayout(parent);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(new QMacCocoaViewContainer(cocoaView, parent));
}
