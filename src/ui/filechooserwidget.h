#ifndef FILECHOOSERWIDGET_H
#define FILECHOOSERWIDGET_H

#include <QWidget>
class QLineEdit;
class QHBoxLayout;

class FileChooserWidget : public QWidget
{
    Q_OBJECT
public:
    enum class Mode { File, Directory };

private:
    QHBoxLayout *layout_;
    QLineEdit   *pathEdit_;
    const Mode   mode_;
    QString      filter_;
    QString      openDirPath_;


public:
    FileChooserWidget(QWidget *parent);
    FileChooserWidget(Mode mode, const QString &initialPath = "", QWidget *parent = nullptr);
    FileChooserWidget(Mode mode, const QString &label, const QString &initialPath = "", QWidget *parent = nullptr);
    ~FileChooserWidget() = default;

    void setFileFilter(const QString &filter);

    void setPath(const QString &path);

    QString getPath() const;

public slots:
    void chooseFile();

private:
    void _init(const QString &initialPath);

};

#endif // FILECHOOSERWIDGET_H
