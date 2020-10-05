#include "filechooserwidget.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileInfo>
#include <QFileDialog>
FileChooserWidget::FileChooserWidget(QWidget *parent):
    QWidget (parent),
    layout_(new QHBoxLayout(this)),
    pathEdit_(new QLineEdit(this)),
    mode_(Mode::Directory),
    filter_(""),
    openDirPath_("")
{
    _init("");
}

FileChooserWidget::FileChooserWidget(Mode mode, const QString &initialPath, QWidget *parent):
    QWidget (parent),
    layout_(new QHBoxLayout(this)),
    pathEdit_(new QLineEdit(this)),
    mode_(mode),
    filter_(""),
    openDirPath_("")
{
    _init(initialPath);
}

FileChooserWidget::FileChooserWidget(Mode mode, const QString &label, const QString &initialPath, QWidget *parent) :
    QWidget (parent),
    layout_(new QHBoxLayout(this)),
    pathEdit_(new QLineEdit(this)),
    mode_(mode),
    filter_(""),
    openDirPath_("")
{
    QLabel *lbl = new QLabel(label, this);
    layout_->addWidget(lbl);
    _init(initialPath);
}

void FileChooserWidget::setFileFilter(const QString &filter)
{
    filter_ = filter;
}

void FileChooserWidget::setPath(const QString &path)
{
    QFileInfo fileInfo(path);
    if (fileInfo.exists())
    {
        pathEdit_->setText(path);
        openDirPath_ = fileInfo.absolutePath();
    }
}

QString FileChooserWidget::getPath() const
{
    QString path(pathEdit_->text());
    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
        return "";
    if (mode_ == Mode::File)
    {
        if (!fileInfo.isFile())
            return "";
    }
    else
    {
        if (!fileInfo.isDir())
            return "";
    }
    return path;
}

void FileChooserWidget::_init(const QString &initialPath)
{
    QFileInfo fileInfo(initialPath);
    if (fileInfo.exists())
    {
        pathEdit_->setText(initialPath);
        openDirPath_ = fileInfo.absolutePath();
    }
    layout_->addWidget(pathEdit_);

    QPushButton *changePath = new QPushButton(QLatin1String("..."), this);
    connect(changePath, &QAbstractButton::clicked, this, &FileChooserWidget::chooseFile);
    changePath->setFixedWidth(2*changePath->fontMetrics().width(QLatin1String(" ... ")));
    //    changePath->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout_->addWidget(changePath);

    layout_->setContentsMargins(2, 0, 2, 0);

    setFocusProxy(pathEdit_);
}

void FileChooserWidget::chooseFile()
{
    QString newPath;

    if ( mode_ == Mode::File )
        newPath = QFileDialog::getOpenFileName( this, tr( "Select a file" ), openDirPath_, filter_ );
    else
        newPath = QFileDialog::getExistingDirectory( this, tr( "Select a directory" ), openDirPath_ );

    if ( !newPath.isEmpty() )
    {
        QFileInfo fileInfo(newPath);
        openDirPath_ = fileInfo.absolutePath();
        if (mode_ == Mode::File)
            pathEdit_->setText(fileInfo.absoluteFilePath());
        else
            pathEdit_->setText(fileInfo.absoluteFilePath() + "/");
    }
}
