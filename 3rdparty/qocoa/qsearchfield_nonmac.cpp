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
#include "../../src/ui/iconloader.h"

#include <QApplication>
#include <QEvent>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QToolButton>
#include <QStyle>

#include <QDir>
#include <QDebug>

class QSearchFieldPrivate : public QObject
{
public:
    QSearchFieldPrivate(QSearchField *searchField, QLineEdit *lineEdit, QToolButton *clearButton)
        : QObject(searchField), lineEdit(lineEdit), clearButton(clearButton) {}
    int lineEditFrameWidth() const {
        return lineEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    }
    int clearButtonPaddedWidth() const {
        return clearButton->width() + lineEditFrameWidth() * 2;
    }
    int clearButtonPaddedHeight() const {
        return clearButton->height() + lineEditFrameWidth() * 2;
    }
    QPointer<QLineEdit> lineEdit;
    QPointer<QToolButton> clearButton;
};

QSearchField::QSearchField(QWidget *parent) : QWidget(parent)
{
    QLineEdit *lineEdit = new QLineEdit(this);
    connect(lineEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(textChanged(QString)));
    connect(lineEdit, SIGNAL(editingFinished()),
            this, SIGNAL(editingFinished()));
    connect(lineEdit, SIGNAL(returnPressed()),
            this, SIGNAL(returnPressed()));
    connect(lineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(setText(QString)));

    QIcon clearIcon(IconLoader::Load("edit-clear-locationbar-ltr", IconLoader::Base));

    QToolButton *clearButton = new QToolButton(this);
    clearButton->setIcon(clearIcon);
    clearButton->setIconSize(QSize(16, 16));
    clearButton->setStyleSheet("border: none; padding: 0px;");
    clearButton->resize(clearButton->sizeHint());
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));

    pimpl = new QSearchFieldPrivate(this, lineEdit, clearButton);

    const int frame_width = lineEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    lineEdit->setStyleSheet(QString("QLineEdit { padding-left: %1px; } ").arg(clearButton->width()));
    const int width = frame_width + qMax(lineEdit->minimumSizeHint().width(), pimpl->clearButtonPaddedWidth());
    const int height = frame_width + qMax(lineEdit->minimumSizeHint().height(), pimpl->clearButtonPaddedHeight());
    lineEdit->setMinimumSize(width, height);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(lineEdit);

    lineEdit->installEventFilter(this);
}

void QSearchField::setText(const QString &text)
{
    Q_ASSERT(pimpl && pimpl->clearButton && pimpl->lineEdit);
    if (!(pimpl && pimpl->clearButton && pimpl->lineEdit))
        return;

    if (text != this->text())
        pimpl->lineEdit->setText(text);
}

void QSearchField::setPlaceholderText(const QString &text)
{
    Q_ASSERT(pimpl && pimpl->lineEdit);
    if (!(pimpl && pimpl->lineEdit))
        return;

#if QT_VERSION >= 0x040700
    pimpl->lineEdit->setPlaceholderText(text);
#endif
}

QString QSearchField::placeholderText() const {
#if QT_VERSION >= 0x040700
  return pimpl->lineEdit->placeholderText();
#else
  return QString();
#endif
}

void QSearchField::setFocus(Qt::FocusReason reason)
{
    Q_ASSERT(pimpl && pimpl->lineEdit);
    if (pimpl && pimpl->lineEdit)
        pimpl->lineEdit->setFocus(reason);
}

void QSearchField::setFocus()
{
    setFocus(Qt::OtherFocusReason);
}

void QSearchField::clear()
{
    Q_ASSERT(pimpl && pimpl->lineEdit);
    if (!(pimpl && pimpl->lineEdit))
        return;

    pimpl->lineEdit->clear();
}

void QSearchField::selectAll()
{
    Q_ASSERT(pimpl && pimpl->lineEdit);
    if (!(pimpl && pimpl->lineEdit))
        return;

    pimpl->lineEdit->selectAll();
}

QString QSearchField::text() const
{
    Q_ASSERT(pimpl && pimpl->lineEdit);
    if (!(pimpl && pimpl->lineEdit))
        return QString();

    return pimpl->lineEdit->text();
}

void QSearchField::resizeEvent(QResizeEvent *resizeEvent)
{
    Q_ASSERT(pimpl && pimpl->clearButton && pimpl->lineEdit);
    if (!(pimpl && pimpl->clearButton && pimpl->lineEdit))
        return;

    QWidget::resizeEvent(resizeEvent);
    const int x = pimpl->lineEditFrameWidth();
    const int y = (height() - pimpl->clearButton->height())/2;
    pimpl->clearButton->move(x, y);
}

bool QSearchField::eventFilter(QObject *o, QEvent *e)
{
    if (pimpl && pimpl->lineEdit && o == pimpl->lineEdit) {
        // Forward some lineEdit events to QSearchField (only those we need for
        // now, but some might be added later if needed)
        switch (e->type()) {
            case QEvent::FocusIn:
            case QEvent::FocusOut:
                QApplication::sendEvent(this, e);
            break;
        }
    }
    return QWidget::eventFilter(o, e);
}
