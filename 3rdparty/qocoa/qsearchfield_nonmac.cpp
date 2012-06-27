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
    connect(lineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(setText(QString)));

    QToolButton *clearButton = new QToolButton(this);
    QPixmap clearIcon(QString(":/Qocoa/qsearchfield_nonmac.png"));
    clearButton->setIcon(QIcon(clearIcon));
    clearButton->setIconSize(clearIcon.size());
    clearButton->setFixedSize(clearIcon.size());
    clearButton->setStyleSheet("border: none;");
    clearButton->hide();
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));

    pimpl = new QSearchFieldPrivate(this, lineEdit, clearButton);

    lineEdit->setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(pimpl->clearButtonPaddedWidth()));
    const int width = qMax(lineEdit->minimumSizeHint().width(), pimpl->clearButtonPaddedWidth());
    const int height = qMax(lineEdit->minimumSizeHint().height(), pimpl->clearButtonPaddedHeight());
    lineEdit->setMinimumSize(width, height);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(lineEdit);
}

void QSearchField::setText(const QString &text)
{
    Q_ASSERT(pimpl && pimpl->clearButton && pimpl->lineEdit);
    if (!(pimpl && pimpl->clearButton && pimpl->lineEdit))
        return;

    pimpl->clearButton->setVisible(!text.isEmpty());

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

void QSearchField::clear()
{
    Q_ASSERT(pimpl && pimpl->lineEdit);
    if (!(pimpl && pimpl->lineEdit))
        return;

    pimpl->lineEdit->clear();
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
    const int x = width() - pimpl->clearButtonPaddedWidth();
    const int y = (height() - pimpl->clearButton->height())/2;
    pimpl->clearButton->move(x, y);
}
