#ifndef QSEARCHFIELD_H
#define QSEARCHFIELD_H

#include <QWidget>
#include <QPointer>
#include <QtGlobal>

#ifndef Q_OS_MAC

// Currently the old LineEdit is better than the current qsearchfield_nonmac
// IMHO: add this (ugly) hack to force non Mac systems to use the old LineEdit.
// TODO: Fix this in a better way (improve qsearchfield_nonmac definitely?)

class LineEdit;
typedef LineEdit QSearchField;

#else // Q_OS_MAC

class QSearchFieldPrivate;
class QSearchField : public QWidget
{
    Q_OBJECT
public:
    explicit QSearchField(QWidget *parent);

    QString text() const;
    QString placeholderText() const;

public slots:
    void setText(const QString &text);
    void setPlaceholderText(const QString &text);
    void clear();
    void selectAll();

signals:
    void textChanged(const QString &text);
    void editingFinished();
    void returnPressed();

protected:
    void resizeEvent(QResizeEvent*);
    bool eventFilter(QObject*, QEvent*);

private:
    friend class QSearchFieldPrivate;
    QPointer <QSearchFieldPrivate> pimpl;

    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText);
};

#endif // Q_OS_MAC

#endif // QSEARCHFIELD_H
