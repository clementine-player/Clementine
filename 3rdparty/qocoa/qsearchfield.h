#ifndef QSEARCHFIELD_H
#define QSEARCHFIELD_H

#include <QWidget>
#include <QPointer>

class QSearchFieldPrivate;
class QSearchField : public QWidget
{
    Q_OBJECT
public:
    explicit QSearchField(QWidget *parent);

    QString text() const;

public slots:
    void setText(const QString &text);
    void setPlaceholderText(const QString &text);

    void clear();
signals:
    void textChanged(const QString &text);
    void editingFinished();

protected:
    void resizeEvent(QResizeEvent*);

private:
    friend class QSearchFieldPrivate;
    QPointer <QSearchFieldPrivate> pimpl;
};

#endif // QSEARCHFIELD_H
