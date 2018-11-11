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

#ifndef FANCYTABWIDGET_H
#define FANCYTABWIDGET_H

#include <memory>

#include <QTabWidget>
#include <QWidget>

class QActionGroup;
class QMenu;
class QSignalMapper;

namespace Core {
namespace Internal {


class FancyTabWidget : public QTabWidget {
  Q_OBJECT

    public:
        FancyTabWidget(QWidget* parent = 0);
        int addTab(QWidget * page, const QIcon & icon, const QString & label);
        int insertTab(int index, QWidget * page, const QIcon & icon, const QString & label);
        void addBottomWidget(QWidget* widget);

        void setBackgroundPixmap(const QPixmap& pixmap);
        void addSpacer();

        void loadSettings(const char *);
        void saveSettings(const char *);
        // Values are persisted - only add to the end
        enum Mode {
            Mode_None = 0,
            Mode_LargeSidebar,
            Mode_SmallSidebar,
            Mode_Tabs,
            Mode_IconOnlyTabs,
            Mode_PlainSidebar,
        };
    
        static const QSize TabSize_LargeSidebar;

        static const QSize IconSize_LargeSidebar;
        static const QSize IconSize_SmallSidebar;
        
        Mode mode() { return mode_; }

    signals:
        void ModeChanged(FancyTabWidget::Mode mode);
        void CurrentChanged(int);

       public slots:
        void setCurrentIndex(int index);
        void SetMode(Mode mode);
        // Mapper mapped signal needs this convenience function 
        void SetMode(int mode) { SetMode(Mode(mode)); }

    private slots:
        void tabBarUpdateGeometry();
        void currentTabChanged(int);

    protected:
        void paintEvent(QPaintEvent *);
        void contextMenuEvent(QContextMenuEvent* e);
    private:
        void addMenuItem(QSignalMapper* mapper, QActionGroup* group,
                                 const QString& text, Mode mode);

        QPixmap background_pixmap_;
        QMenu* menu_;
        Mode mode_;
        QWidget *bottom_widget_;

};

}  // namespace Internal
}  // namespace Core

using Core::Internal::FancyTabWidget;

#endif  // FANCYTABWIDGET_H
