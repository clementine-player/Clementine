/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#ifndef FANCYTABWIDGET_H
#define FANCYTABWIDGET_H

#include <memory>

#include <QIcon>
#include <QPropertyAnimation>
#include <QProxyStyle>
#include <QTabBar>
#include <QTimer>
#include <QWidget>

class QActionGroup;
class QMenu;
class QPainter;
class QSignalMapper;
class QStackedLayout;
class QStatusBar;
class QVBoxLayout;

namespace Core {
namespace Internal {

class FancyTabProxyStyle : public QProxyStyle {
  Q_OBJECT

 public:
  void drawControl(ControlElement element, const QStyleOption* option,
                   QPainter* painter, const QWidget* widget) const;
  void polish(QWidget* widget);
  void polish(QApplication* app);
  void polish(QPalette& palette);

 protected:
  bool eventFilter(QObject* o, QEvent* e);
};

class FancyTab : public QWidget {
  Q_OBJECT

  Q_PROPERTY(float fader READ fader WRITE setFader)
 public:
  FancyTab(QWidget* tabbar);
  float fader() { return m_fader; }
  void setFader(float value);

  QSize sizeHint() const;

  void fadeIn();
  void fadeOut();

  QIcon icon;
  QString text;

 protected:
  bool event(QEvent*);
  void enterEvent(QEvent*);
  void leaveEvent(QEvent*);

 private:
  QPropertyAnimation animator;
  QWidget* tabbar;
  float m_fader;
};

class FancyTabBar : public QWidget {
  Q_OBJECT

 public:
  FancyTabBar(QWidget* parent = nullptr);
  ~FancyTabBar();

  void paintEvent(QPaintEvent* event);
  void paintTab(QPainter* painter, int tabIndex) const;
  void mousePressEvent(QMouseEvent*);
  bool validIndex(int index) const {
    return index >= 0 && index < m_tabs.count();
  }

  QSize sizeHint() const;
  QSize minimumSizeHint() const;

  void addTab(const QIcon& icon, const QString& label);
  void addSpacer(int size = 40);
  void removeTab(int index) {
    FancyTab* tab = m_tabs.takeAt(index);
    delete tab;
  }
  void setCurrentIndex(int index);
  int currentIndex() const { return m_currentIndex; }

  void setTabToolTip(int index, const QString& toolTip);
  QString tabToolTip(int index) const;

  QIcon tabIcon(int index) const { return m_tabs.at(index)->icon; }
  QString tabText(int index) const { return m_tabs.at(index)->text; }
  int count() const { return m_tabs.count(); }
  QRect tabRect(int index) const;

signals:
  void currentChanged(int);

 public slots:
  void emitCurrentIndex();

 private:
  static const int m_rounding;
  static const int m_textPadding;
  int m_currentIndex;
  QList<FancyTab*> m_tabs;
  QTimer m_triggerTimer;
  QSize tabSizeHint(bool minimum = false) const;
};

class FancyTabWidget : public QWidget {
  Q_OBJECT

 public:
  FancyTabWidget(QWidget* parent = nullptr);

  // Values are persisted - only add to the end
  enum Mode {
    Mode_None = 0,
    Mode_LargeSidebar = 1,
    Mode_SmallSidebar = 2,
    Mode_Tabs = 3,
    Mode_IconOnlyTabs = 4,
    Mode_PlainSidebar = 5,
  };

  struct Item {
    Item(const QIcon& icon, const QString& label)
        : type_(Type_Tab),
          tab_label_(label),
          tab_icon_(icon),
          spacer_size_(0) {}
    Item(int size) : type_(Type_Spacer), spacer_size_(size) {}

    enum Type { Type_Tab, Type_Spacer, };

    Type type_;
    QString tab_label_;
    QIcon tab_icon_;
    int spacer_size_;
  };

  void AddTab(QWidget* tab, const QIcon& icon, const QString& label);
  void AddSpacer(int size = 40);
  void SetBackgroundPixmap(const QPixmap& pixmap);

  void AddBottomWidget(QWidget* widget);

  int current_index() const;
  Mode mode() const { return mode_; }

 public slots:
  void SetCurrentIndex(int index);
  void SetCurrentWidget(QWidget* widget);
  void SetMode(Mode mode);
  void SetMode(int mode) { SetMode(Mode(mode)); }

signals:
  void CurrentChanged(int index);
  void ModeChanged(FancyTabWidget::Mode mode);

 protected:
  void paintEvent(QPaintEvent* event);
  void contextMenuEvent(QContextMenuEvent* e);

 private slots:
  void ShowWidget(int index);

 private:
  void MakeTabBar(QTabBar::Shape shape, bool text, bool icons, bool fancy);
  void AddMenuItem(QSignalMapper* mapper, QActionGroup* group,
                   const QString& text, Mode mode);

  Mode mode_;
  QList<Item> items_;

  QWidget* tab_bar_;
  QStackedLayout* stack_;
  QPixmap background_pixmap_;
  QWidget* side_widget_;
  QVBoxLayout* side_layout_;
  QVBoxLayout* top_layout_;

  bool use_background_;

  QMenu* menu_;

  std::unique_ptr<FancyTabProxyStyle> proxy_style_;
};

}  // namespace Internal
}  // namespace Core

Q_DECLARE_METATYPE(QPropertyAnimation*);

using Core::Internal::FancyTab;
using Core::Internal::FancyTabBar;
using Core::Internal::FancyTabWidget;

#endif  // FANCYTABWIDGET_H
