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

#include "qsearchfield.h"

#include "qocoa_mac.h"

#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSNotification.h"
#import "AppKit/NSSearchField.h"

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>

class QSearchFieldPrivate : public QObject
{
public:
    QSearchFieldPrivate(QSearchField *qSearchField, NSSearchField *nsSearchField)
        : QObject(qSearchField), qSearchField(qSearchField), nsSearchField(nsSearchField) {}

    void textDidChange(const QString &text)
    {
        if (qSearchField)
            emit qSearchField->textChanged(text);
    }

    void textDidEndEditing()
    {
        if (qSearchField)
            emit qSearchField->editingFinished();
    }

    void returnPressed()
    {
        if (qSearchField) {
            emit qSearchField->returnPressed();
            QKeyEvent* event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
            QApplication::postEvent(qSearchField, event);
        }
    }

    QPointer<QSearchField> qSearchField;
    NSSearchField *nsSearchField;
};

@interface QSearchFieldDelegate : NSObject<NSTextFieldDelegate>
{
@public
    QPointer<QSearchFieldPrivate> pimpl;
}
-(void)controlTextDidChange:(NSNotification*)notification;
-(void)controlTextDidEndEditing:(NSNotification*)notification;
@end

@implementation QSearchFieldDelegate
-(void)controlTextDidChange:(NSNotification*)notification {
    Q_ASSERT(pimpl);
    if (pimpl)
        pimpl->textDidChange(toQString([[notification object] stringValue]));
}

-(void)controlTextDidEndEditing:(NSNotification*)notification {
    // No Q_ASSERT here as it is called on destruction.
    if (!pimpl) return;

    pimpl->textDidEndEditing();

    if ([[[notification userInfo] objectForKey:@"NSTextMovement"] intValue] == NSReturnTextMovement)
        pimpl->returnPressed();
}

@end

@interface QocoaSearchField : NSSearchField
-(BOOL)performKeyEquivalent:(NSEvent*)event;
@end

@implementation QocoaSearchField
-(BOOL)performKeyEquivalent:(NSEvent*)event {

    // First, check if we have the focus.
    // If no, it probably means this event isn't for us.
    NSResponder* firstResponder = [[NSApp keyWindow] firstResponder];
    if ([firstResponder isKindOfClass:[NSText class]] &&
            [(NSText*)firstResponder delegate] == self) {

        if ([event type] == NSKeyDown && [event modifierFlags] & NSCommandKeyMask)
        {
            QString keyString = toQString([event characters]);
            if (keyString == "a")  // Cmd+a
            {
                [self performSelector:@selector(selectText:)];
                return YES;
            }
            else if (keyString == "c")  // Cmd+c
            {
                QClipboard* clipboard = QApplication::clipboard();
                clipboard->setText(toQString([self stringValue]));
                return YES;
            }
            else if (keyString == "v")  // Cmd+v
            {
                QClipboard* clipboard = QApplication::clipboard();
                [self setStringValue:fromQString(clipboard->text())];
                return YES;
            }
            else if (keyString == "x")  // Cmd+x
            {
                QClipboard* clipboard = QApplication::clipboard();
                clipboard->setText(toQString([self stringValue]));
                [self setStringValue:@""];
                return YES;
            }
        }
    }

    return NO;
}
@end

QSearchField::QSearchField(QWidget *parent) : QWidget(parent)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSSearchField *search = [[QocoaSearchField alloc] init];

    QSearchFieldDelegate *delegate = [[QSearchFieldDelegate alloc] init];
    pimpl = delegate->pimpl = new QSearchFieldPrivate(this, search);
    [search setDelegate:delegate];

    setupLayout(search, this);

    setFixedHeight(24);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    [search release];

    [pool drain];
}

void QSearchField::setText(const QString &text)
{
    Q_ASSERT(pimpl);
    if (!pimpl)
        return;

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [pimpl->nsSearchField setStringValue:fromQString(text)];
    [pimpl->nsSearchField selectText:pimpl->nsSearchField];
    [[pimpl->nsSearchField currentEditor] setSelectedRange:NSMakeRange([[pimpl->nsSearchField stringValue] length], 0)];
    [pool drain];
}

void QSearchField::setPlaceholderText(const QString &text)
{
    Q_ASSERT(pimpl);
    if (!pimpl)
        return;

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [[pimpl->nsSearchField cell] setPlaceholderString:fromQString(text)];
    [pool drain];
}

QString QSearchField::placeholderText() const {
    Q_ASSERT(pimpl);
    NSString* placeholder = [[pimpl->nsSearchField cell] placeholderString];
    return toQString(placeholder);
}

void QSearchField::setFocus(Qt::FocusReason reason)
{
/* Do nothing: we were previously using makeFirstResponder on search field, but
 * that resulted in having the text being selected (and I didn't find any way to
 * deselect it) which would result in the user erasing the first letter he just
 * typed, after using setText (e.g. if the user typed a letter while having
 * focus on the playlist, which means we call setText and give focus to the
 * search bar).
 * Instead now the focus will take place when calling selectText in setText.
 * This obviously breaks the purpose of this function, but we never call only
 * setFocus on a search box in Clementine (i.e. without a call to setText
 * shortly after).
 */

//    Q_ASSERT(pimpl);
//    if (!pimpl)
//        return;

//    if ([pimpl->nsSearchField acceptsFirstResponder]) {
//        [[pimpl->nsSearchField window] makeFirstResponder: pimpl->nsSearchField];
//    }
}

void QSearchField::setFocus()
{
    setFocus(Qt::OtherFocusReason);
}

void QSearchField::clear()
{
    Q_ASSERT(pimpl);
    if (!pimpl)
        return;

    [pimpl->nsSearchField setStringValue:@""];
    emit textChanged(QString());
}

void QSearchField::selectAll()
{
    Q_ASSERT(pimpl);
    if (!pimpl)
        return;

    [pimpl->nsSearchField performSelector:@selector(selectText:)];
}

QString QSearchField::text() const
{
    Q_ASSERT(pimpl);
    if (!pimpl)
        return QString();

    return toQString([pimpl->nsSearchField stringValue]);
}

void QSearchField::resizeEvent(QResizeEvent *resizeEvent)
{
    QWidget::resizeEvent(resizeEvent);
}

bool QSearchField::eventFilter(QObject *o, QEvent *e)
{
    return QWidget::eventFilter(o, e);
}
