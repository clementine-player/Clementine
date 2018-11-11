/* This file is part of Clementine.
   Copyright 2018, Vikram Ambrose <ambroseworks@gmail.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "fancytabwidget.h"
#include "stylehelper.h"
#include "core/logging.h"

#include <QDebug>

#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QSignalMapper>
#include <QTabBar>
#include <QStylePainter>
#include <QTimer>
#include <QVBoxLayout>
#include <QSettings>

const QSize FancyTabWidget::IconSize_LargeSidebar = QSize(24,24);
const QSize FancyTabWidget::IconSize_SmallSidebar = QSize(22,22);

const QSize FancyTabWidget::TabSize_LargeSidebar = QSize(70,47);

class FancyTabBar: public QTabBar {

private:
    int mouseHoverTabIndex = -1;
    QMap<QWidget*,QString> labelCache;

public:
    explicit FancyTabBar(QWidget* parent=0) : QTabBar(parent) {
        setMouseTracking(true);
    }

    QSize sizeHint() const {
        QSize size(QTabBar::sizeHint());

        FancyTabWidget *tabWidget = (FancyTabWidget*) parentWidget();
        if(tabWidget->mode() == FancyTabWidget::Mode_Tabs || 
           tabWidget->mode() == FancyTabWidget::Mode_IconOnlyTabs)
            return size;

        QSize tabSize(tabSizeHint(0));
        size.setWidth(tabSize.width());
        int guessHeight = tabSize.height()*count();
        if(guessHeight > size.height())
            size.setHeight(guessHeight);
        return size;
    }

    int width() {
        return tabSizeHint(0).width();
    }

protected:
    QSize tabSizeHint(int index) const {
        FancyTabWidget *tabWidget = (FancyTabWidget*) parentWidget();
        QSize size = FancyTabWidget::TabSize_LargeSidebar;

        if(tabWidget->mode() != FancyTabWidget::Mode_LargeSidebar) {
            size = QTabBar::tabSizeHint(index);
        }
        
        return size;
    }
    
    void leaveEvent(QEvent * event) {
        mouseHoverTabIndex = -1;
        update();
    }

    void mouseMoveEvent(QMouseEvent * event) {
        QPoint pos = event->pos();

        mouseHoverTabIndex = tabAt(pos);
        if(mouseHoverTabIndex > -1)
            update();
        QTabBar::mouseMoveEvent(event);
    }

    void paintEvent(QPaintEvent *pe) {
        FancyTabWidget *tabWidget = (FancyTabWidget*) parentWidget();

        bool verticalTextTabs = false;

        if(tabWidget->mode() == FancyTabWidget::Mode_SmallSidebar)
            verticalTextTabs = true;

        // Restore any label text that was hidden/cached for the IconOnlyTabs mode
        if(labelCache.count() > 0 && tabWidget->mode() != FancyTabWidget::Mode_IconOnlyTabs) {
                for(int i =0; i < count(); i++) {
                    setTabText(i,labelCache[tabWidget->widget(i)]);
                }
                labelCache.clear();
        }
        if(tabWidget->mode() != FancyTabWidget::Mode_LargeSidebar &&
           tabWidget->mode() != FancyTabWidget::Mode_SmallSidebar) {
            // Cache and hide label text for IconOnlyTabs mode
            if(tabWidget->mode() == FancyTabWidget::Mode_IconOnlyTabs && labelCache.count() == 0) {
                for(int i =0; i < count(); i++) {
                    labelCache[tabWidget->widget(i)] = tabText(i);
                    setTabText(i,"");
                }
            }
            QTabBar::paintEvent(pe);
            return;
        }

        QStylePainter p(this);

 
        for (int index = 0; index < count(); index++) {
            const bool selected = tabWidget->currentIndex() == index;;

            QRect tabrect = tabRect(index);

            QRect selectionRect = tabrect;

            if(selected) {
                // Selection highlight
                p.save();
                QLinearGradient grad(selectionRect.topLeft(), selectionRect.topRight());
                grad.setColorAt(0, QColor(255, 255, 255, 140));
                grad.setColorAt(1, QColor(255, 255, 255, 210));
                p.fillRect(selectionRect.adjusted(0,0,0,-1), grad);
                p.restore();

                // shadow lines
                p.setPen(QColor(0, 0, 0, 110));
                p.drawLine(selectionRect.topLeft()    + QPoint(1, -1), selectionRect.topRight()    - QPoint(0, 1));
                p.drawLine(selectionRect.bottomLeft(), selectionRect.bottomRight());
                p.setPen(QColor(0, 0, 0, 40));
                p.drawLine(selectionRect.topLeft(),    selectionRect.bottomLeft());

                // highlights
                p.setPen(QColor(255, 255, 255, 50));
                p.drawLine(selectionRect.topLeft()    + QPoint(0, -2), selectionRect.topRight()    - QPoint(0, 2));
                p.drawLine(selectionRect.bottomLeft() + QPoint(0, 1),  selectionRect.bottomRight() + QPoint(0, 1));
                p.setPen(QColor(255, 255, 255, 40));
                p.drawLine(selectionRect.topLeft()    + QPoint(0, 0),  selectionRect.topRight());
                p.drawLine(selectionRect.topRight()   + QPoint(0, 1),  selectionRect.bottomRight() - QPoint(0, 1));
                p.drawLine(selectionRect.bottomLeft() + QPoint(0, -1), selectionRect.bottomRight() - QPoint(0, 1));

            }

            // Mouse hover effect
            if(!selected && index == mouseHoverTabIndex && isTabEnabled(index))
            {
                p.save();
                QLinearGradient grad(selectionRect.topLeft(),  selectionRect.topRight());
                grad.setColorAt(0, Qt::transparent);
                grad.setColorAt(0.5, QColor(255, 255, 255, 40));
                grad.setColorAt(1, Qt::transparent);
                p.fillRect(selectionRect, grad);
                p.setPen(QPen(grad, 1.0));
                p.drawLine(selectionRect.topLeft(),     selectionRect.topRight());
                p.drawLine(selectionRect.bottomRight(), selectionRect.bottomLeft());
                p.restore();
            }

            // Label (Icon and Text) 
            {
                p.save();
                QTransform m;
                int textFlags;
                Qt::Alignment iconFlags;

                QRect tabrectText;
                QRect tabrectLabel;

                if (verticalTextTabs) {
                    m = QTransform::fromTranslate(tabrect.left(), tabrect.bottom());
                    m.rotate(-90);
                    textFlags = Qt::AlignLeft | Qt::AlignVCenter ;
                    iconFlags = Qt::AlignLeft  | Qt::AlignVCenter;

                    tabrectLabel = QRect(QPoint(0, 0), m.mapRect(tabrect).size());

                    tabrectText = tabrectLabel;
                    tabrectText.translate(30,0);
                } else {
                    m = QTransform::fromTranslate(tabrect.left(), tabrect.top());
                    textFlags = Qt::AlignHCenter | Qt::AlignBottom ;
                    iconFlags = Qt::AlignHCenter | Qt::AlignTop;

                    tabrectLabel = QRect(QPoint(0, 0), m.mapRect(tabrect).size());

                    tabrectText = tabrectLabel;
                    tabrectText.translate(0,-5);
                }

                p.setTransform(m);

                QFont boldFont(p.font());
                boldFont.setPointSizeF(Utils::StyleHelper::sidebarFontSize());
                boldFont.setBold(true);
                p.setFont(boldFont);

                // Text drop shadow color 
                p.setPen(selected ? QColor(255,255,255,160) : QColor(0,0,0,110) );
                p.translate(0, 3);
                p.drawText(tabrectText, textFlags, tabText(index));

                // Text foreground color 
                p.translate(0, -1);
                p.setPen(selected ? QColor(60, 60, 60) : Utils::StyleHelper::panelTextColor());
                p.drawText(tabrectText, textFlags, tabText(index));


                // Draw the icon
                QRect tabrectIcon;
                const int PADDING = 5;
                if(verticalTextTabs) {
                    tabrectIcon = tabrectLabel;
                    tabrectIcon.setSize(FancyTabWidget::IconSize_SmallSidebar);   
                    tabrectIcon.translate(PADDING,PADDING);
                } else {
                    tabrectIcon = tabrectLabel;
                    tabrectIcon.setSize(FancyTabWidget::IconSize_LargeSidebar);   
                    // Center the icon
                    const int moveRight = (FancyTabWidget::TabSize_LargeSidebar.width() - 
                                          FancyTabWidget::IconSize_LargeSidebar.width() -1)/2;
                    tabrectIcon.translate(moveRight,PADDING);
                }
                tabIcon(index).paint(&p, tabrectIcon, iconFlags);
                p.restore();
           }
        }
    }
};
 
// Spacers are just disabled pages
void FancyTabWidget::addSpacer() {
    QWidget *spacer = new QWidget();
    const int index = addTab(spacer,QIcon(),QString());
    setTabEnabled(index,false);
}

void FancyTabWidget::setBackgroundPixmap(const QPixmap& pixmap) {
    background_pixmap_ = pixmap;
    update();
}

void FancyTabWidget::setCurrentIndex(int index) {
    QWidget* currentPage = widget(index);
    
    QLayout *layout = currentPage->layout();
    if(bottom_widget_ != nullptr)
        layout->addWidget(bottom_widget_);

    QTabWidget::setCurrentIndex(index);
}

// Slot
void FancyTabWidget::currentTabChanged(int index) {
    QWidget* currentPage = currentWidget();
    
    QLayout *layout = currentPage->layout();
    if(bottom_widget_ != nullptr)
        layout->addWidget(bottom_widget_);
    emit CurrentChanged(index);
}

FancyTabWidget::FancyTabWidget(QWidget* parent) : QTabWidget(parent), 
      menu_(nullptr),
      mode_(Mode_None),
      bottom_widget_(nullptr) {
    FancyTabBar *tabBar = new FancyTabBar(this);

    setTabBar(tabBar);
    setTabPosition(QTabWidget::West);
    setMovable(true);

    connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
}

void FancyTabWidget::loadSettings(const char *kSettingsGroup) {
    QSettings settings;
    settings.beginGroup(kSettingsGroup);

    for(int i =0;i<count();i++) {
        int originalIndex = tabBar()->tabData(i).toInt();
        std::string k = "tab_index_" + std::to_string(originalIndex);

        int newIndex = settings.value(QString::fromStdString(k), i).toInt();

        if(newIndex >= 0)
            tabBar()->moveTab(i,newIndex);
        else
            removeTab(i); // Does not delete page
    }
}

void FancyTabWidget::saveSettings(const char *kSettingsGroup) {
    QSettings settings;
    settings.beginGroup(kSettingsGroup);

    for(int i =0;i<count();i++) {
        int originalIndex = tabBar()->tabData(i).toInt();
        std::string k = "tab_index_" + std::to_string(originalIndex);

        settings.setValue(QString::fromStdString(k), i);
    }
}


void FancyTabWidget::addBottomWidget(QWidget* widget) {
    bottom_widget_ = widget;
}

int FancyTabWidget::addTab(QWidget * page, const QIcon & icon, const QString & label) {
    return insertTab(count(),page,icon,label);
}

int FancyTabWidget::insertTab(int index, QWidget * page, const QIcon & icon, const QString & label) {
    // In order to achieve the same effect as the "Bottom Widget" of the 
    // old Nokia based FancyTabWidget a VBoxLayout is used on each page
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(page);

    QWidget *newPage = new QWidget();
    newPage->setLayout(layout);

    const int actualIndex = QTabWidget::insertTab(index,newPage,icon,label);
    
    // Remember the original index. Needed to save order of tabs
    tabBar()->setTabData(actualIndex,QVariant(actualIndex));
    return actualIndex;
}

void FancyTabWidget::paintEvent(QPaintEvent *pe) {
    if(mode() != FancyTabWidget::Mode_LargeSidebar &&
       mode() != FancyTabWidget::Mode_SmallSidebar) {
        QTabWidget::paintEvent(pe);
        return;
    }
    QStylePainter p(this);

    // The brown color (Ubuntu) you see on the background gradient 
    QColor baseColor = StyleHelper::baseColor();

    QRect backgroundRect = rect();
    backgroundRect.setWidth(((FancyTabBar*)tabBar())->width());
    p.fillRect(backgroundRect,baseColor);

    // Horizontal gradient over the sidebar from transparent to dark 
    Utils::StyleHelper::verticalGradient(&p,backgroundRect,backgroundRect,false);

    // Draw the translucent png graphics over the gradient fill
    {
        if (!background_pixmap_.isNull()) {
            QRect pixmap_rect(background_pixmap_.rect());
            pixmap_rect.moveTo(backgroundRect.topLeft());

            while (pixmap_rect.top() < backgroundRect.bottom()) {
                QRect source_rect(pixmap_rect.intersected(backgroundRect));
                source_rect.moveTo(0, 0);
                p.drawPixmap(pixmap_rect.topLeft(), background_pixmap_,source_rect);
                pixmap_rect.moveTop(pixmap_rect.bottom() - 10);
            }
        }
    }

    // Shadow effect of the background
    {
        QColor light(255, 255, 255, 80);
        p.setPen(light);
        p.drawLine(backgroundRect.topRight() - QPoint(1, 0),  backgroundRect.bottomRight() - QPoint(1, 0));
        QColor dark(0, 0, 0, 90);
        p.setPen(dark);
        p.drawLine(backgroundRect.topLeft(), backgroundRect.bottomLeft());

        p.setPen(Utils::StyleHelper::borderColor());
        p.drawLine(backgroundRect.topRight(), backgroundRect.bottomRight());
    }

}

void FancyTabWidget::tabBarUpdateGeometry() {
    tabBar()->updateGeometry();
}

void FancyTabWidget::SetMode(FancyTabWidget::Mode mode) {
    mode_ = mode;

    if(mode == FancyTabWidget::Mode_Tabs || 
       mode == FancyTabWidget::Mode_IconOnlyTabs) {
        setTabPosition(QTabWidget::North);
    } else {
        setTabPosition(QTabWidget::West);
    }

    tabBar()->updateGeometry();
    updateGeometry();

    // There appears to be a bug in QTabBar which causes tabSizeHint 
    // to be ignored thus the need for this second shot repaint
    QTimer::singleShot(1,this,SLOT(tabBarUpdateGeometry()));

    emit ModeChanged(mode);
}

void FancyTabWidget::addMenuItem(QSignalMapper* mapper, QActionGroup* group,
                                 const QString& text, Mode mode) {
  QAction* action = group->addAction(text);
  action->setCheckable(true);
  mapper->setMapping(action, mode);
  connect(action, SIGNAL(triggered()), mapper, SLOT(map()));

  if (mode == mode_) action->setChecked(true);
}


void FancyTabWidget::contextMenuEvent(QContextMenuEvent* e) {
  if (!menu_) {
    menu_ = new QMenu(this);

    QSignalMapper* mapper = new QSignalMapper(this);
    QActionGroup* group = new QActionGroup(this);
    addMenuItem(mapper, group, tr("Large sidebar"), Mode_LargeSidebar);
    addMenuItem(mapper, group, tr("Small sidebar"), Mode_SmallSidebar);
    addMenuItem(mapper, group, tr("Plain sidebar"), Mode_PlainSidebar);
    addMenuItem(mapper, group, tr("Tabs on top"), Mode_Tabs);
    addMenuItem(mapper, group, tr("Icons on top"), Mode_IconOnlyTabs);
    menu_->addActions(group->actions());

    connect(mapper, SIGNAL(mapped(int)), SLOT(SetMode(int)));
  }

  menu_->popup(e->globalPos());
}
