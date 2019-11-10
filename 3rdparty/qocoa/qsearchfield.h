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
    QString placeholderText() const;
    void setFocus(Qt::FocusReason reason);

public slots:
    void setText(const QString &text);
    void setPlaceholderText(const QString &text);
    void clear();
    void selectAll();
    void setFocus();

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

    Q_PROPERTY(
        QString placeholderText READ placeholderText WRITE setPlaceholderText)
};

#endif // QSEARCHFIELD_H
