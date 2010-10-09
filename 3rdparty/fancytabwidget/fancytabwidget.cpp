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

#include "fancytabwidget.h"
#include "stylehelper.h"
#include "styledbar.h"

#include <QDebug>

#include <QAnimationGroup>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QSignalMapper>
#include <QSplitter>
#include <QStackedLayout>
#include <QToolButton>
#include <QToolTip>
#include <QVBoxLayout>
#include <QWindowsStyle>

using namespace Core;
using namespace Internal;

const int FancyTabBar::m_rounding = 22;
const int FancyTabBar::m_textPadding = 4;

FancyTab::FancyTab(QWidget* tabbar)
  : QWidget(tabbar), tabbar(tabbar), m_fader(0)
{
  animator.setPropertyName("fader");
  animator.setTargetObject(this);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
}

void FancyTab::fadeIn()
{
    animator.stop();
    animator.setDuration(80);
    animator.setEndValue(40);
    animator.start();
}

void FancyTab::fadeOut()
{
    animator.stop();
    animator.setDuration(160);
    animator.setEndValue(0);
    animator.start();
}

void FancyTab::setFader(float value)
{
    m_fader = value;
    tabbar->update();
}

FancyTabBar::FancyTabBar(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    setStyle(new QWindowsStyle);
    setMinimumWidth(qMax(2 * m_rounding, 40));
    setAttribute(Qt::WA_Hover, true);
    setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true); // Needed for hover events
    m_triggerTimer.setSingleShot(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    // We use a zerotimer to keep the sidebar responsive
    connect(&m_triggerTimer, SIGNAL(timeout()), this, SLOT(emitCurrentIndex()));
}

FancyTabBar::~FancyTabBar()
{
    delete style();
}

QSize FancyTab::sizeHint() const {
  QFont boldFont(font());
  boldFont.setPointSizeF(Utils::StyleHelper::sidebarFontSize());
  boldFont.setBold(true);
  QFontMetrics fm(boldFont);
  int spacing = 8;
  int width = 60 + spacing + 2;
  int iconHeight = 32;
  QSize ret(width, iconHeight + spacing + fm.height());
  return ret;
}

QSize FancyTabBar::tabSizeHint(bool minimum) const
{
  QFont boldFont(font());
  boldFont.setPointSizeF(Utils::StyleHelper::sidebarFontSize());
  boldFont.setBold(true);
  QFontMetrics fm(boldFont);
  int spacing = 8;
  int width = 60 + spacing + 2;
  int maxLabelwidth = 0;
  for (int tab=0 ; tab<count() ;++tab) {
      int width = fm.width(m_tabs[tab]->text);
      if (width > maxLabelwidth)
          maxLabelwidth = width;
  }
  int iconHeight = minimum ? 0 : 32;
  return QSize(qMax(width, maxLabelwidth + 4), iconHeight + spacing + fm.height());
}

void FancyTabBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(this);

    for (int i = 0; i < count(); ++i)
        if (i != currentIndex())
            paintTab(&p, i);

    // paint active tab last, since it overlaps the neighbors
    if (currentIndex() != -1)
        paintTab(&p, currentIndex());
}

// Resets hover animation on mouse enter
void FancyTab::enterEvent(QEvent*)
{
    fadeIn();
}

// Resets hover animation on mouse enter
void FancyTab::leaveEvent(QEvent*)
{
    fadeOut();
}

QSize FancyTabBar::sizeHint() const
{
    QSize sh = tabSizeHint();
    return QSize(sh.width(), sh.height() * m_tabs.count());
}

QSize FancyTabBar::minimumSizeHint() const
{
    QSize sh = tabSizeHint(true);
    return QSize(sh.width(), sh.height() * m_tabs.count());
}

QRect FancyTabBar::tabRect(int index) const
{
    return m_tabs[index]->geometry();
}

QString FancyTabBar::tabToolTip(int index) const {
  return m_tabs[index]->toolTip();
}

void FancyTabBar::setTabToolTip(int index, const QString& toolTip) {
  m_tabs[index]->setToolTip(toolTip);
}

// This keeps the sidebar responsive since
// we get a repaint before loading the
// mode itself
void FancyTabBar::emitCurrentIndex()
{
    emit currentChanged(m_currentIndex);
}

void FancyTabBar::mousePressEvent(QMouseEvent *e)
{
    e->accept();
    for (int index = 0; index < m_tabs.count(); ++index) {
        if (tabRect(index).contains(e->pos())) {
            m_currentIndex = index;
            update();
            m_triggerTimer.start(0);
            break;
        }
    }
}

void FancyTabBar::addTab(const QIcon& icon, const QString& label) {
  FancyTab *tab = new FancyTab(this);
  tab->icon = icon;
  tab->text = label;
  m_tabs.append(tab);
  qobject_cast<QVBoxLayout*>(layout())->insertWidget(layout()->count()-1, tab);
}

void FancyTabBar::addSpacer(int size) {
  qobject_cast<QVBoxLayout*>(layout())->insertSpacerItem(layout()->count()-1,
      new QSpacerItem(0, size, QSizePolicy::Fixed, QSizePolicy::Maximum));
}

void FancyTabBar::paintTab(QPainter *painter, int tabIndex) const
{
    if (!validIndex(tabIndex)) {
        qWarning("invalid index");
        return;
    }
    painter->save();

    QRect rect = tabRect(tabIndex);
    bool selected = (tabIndex == m_currentIndex);

    if (selected) {
        //background
        painter->save();
        QLinearGradient grad(rect.topLeft(), rect.topRight());
        grad.setColorAt(0, QColor(255, 255, 255, 140));
        grad.setColorAt(1, QColor(255, 255, 255, 210));
        painter->fillRect(rect.adjusted(0, 0, 0, -1), grad);
        painter->restore();

        //shadows
        painter->setPen(QColor(0, 0, 0, 110));
        painter->drawLine(rect.topLeft() + QPoint(1,-1), rect.topRight() - QPoint(0,1));
        painter->drawLine(rect.bottomLeft(), rect.bottomRight());
        painter->setPen(QColor(0, 0, 0, 40));
        painter->drawLine(rect.topLeft(), rect.bottomLeft());

        //highlights
        painter->setPen(QColor(255, 255, 255, 50));
        painter->drawLine(rect.topLeft() + QPoint(0, -2), rect.topRight() - QPoint(0,2));
        painter->drawLine(rect.bottomLeft() + QPoint(0, 1), rect.bottomRight() + QPoint(0,1));
        painter->setPen(QColor(255, 255, 255, 40));
        painter->drawLine(rect.topLeft() + QPoint(0, 0), rect.topRight());
        painter->drawLine(rect.topRight() + QPoint(0, 1), rect.bottomRight() - QPoint(0, 1));
        painter->drawLine(rect.bottomLeft() + QPoint(0,-1), rect.bottomRight()-QPoint(0,1));
    }

    QString tabText(this->tabText(tabIndex));
    QRect tabTextRect(tabRect(tabIndex));
    QRect tabIconRect(tabTextRect);
    tabIconRect.adjust(+4, +4, -4, -4);
    tabTextRect.translate(0, -2);
    QFont boldFont(painter->font());
    boldFont.setPointSizeF(Utils::StyleHelper::sidebarFontSize());
    boldFont.setBold(true);
    painter->setFont(boldFont);
    painter->setPen(selected ? QColor(255, 255, 255, 160) : QColor(0, 0, 0, 110));
    int textFlags = Qt::AlignCenter | Qt::AlignBottom | Qt::TextWordWrap;
    painter->drawText(tabTextRect, textFlags, tabText);
    painter->setPen(selected ? QColor(60, 60, 60) : Utils::StyleHelper::panelTextColor());
#ifndef Q_WS_MAC
    if (!selected) {
        painter->save();
        int fader = int(m_tabs[tabIndex]->fader());
        QLinearGradient grad(rect.topLeft(), rect.topRight());
        grad.setColorAt(0, Qt::transparent);
        grad.setColorAt(0.5, QColor(255, 255, 255, fader));
        grad.setColorAt(1, Qt::transparent);
        painter->fillRect(rect, grad);
        painter->setPen(QPen(grad, 1.0));
        painter->drawLine(rect.topLeft(), rect.topRight());
        painter->drawLine(rect.bottomLeft(), rect.bottomRight());
        painter->restore();
    }
#endif

    int textHeight = painter->fontMetrics().boundingRect(QRect(0, 0, width(), height()), Qt::TextWordWrap, tabText).height();
    tabIconRect.adjust(0, 4, 0, -textHeight);
    Utils::StyleHelper::drawIconWithShadow(tabIcon(tabIndex), tabIconRect, painter, QIcon::Normal);

    painter->translate(0, -1);
    painter->drawText(tabTextRect, textFlags, tabText);
    painter->restore();
}

void FancyTabBar::setCurrentIndex(int index) {
    m_currentIndex = index;
    update();
    emit currentChanged(m_currentIndex);
}


//////
// FancyColorButton
//////

class FancyColorButton : public QWidget
{
public:
    FancyColorButton(QWidget *parent)
      : m_parent(parent)
    {
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    }

    void mousePressEvent(QMouseEvent *ev)
    {
        if (ev->modifiers() & Qt::ShiftModifier)
            Utils::StyleHelper::setBaseColor(QColorDialog::getColor(Utils::StyleHelper::requestedBaseColor(), m_parent));
    }
private:
    QWidget *m_parent;
};

//////
// FancyTabWidget
//////

FancyTabWidget::FancyTabWidget(QWidget* parent)
  : QWidget(parent),
    mode_(Mode_None),
    tab_bar_(NULL),
    stack_(new QStackedLayout),
    side_widget_(new QWidget),
    side_layout_(new QVBoxLayout),
    top_layout_(new QVBoxLayout),
    menu_(NULL)
{
  side_layout_->setSpacing(0);
  side_layout_->setMargin(0);
  side_layout_->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

  side_widget_->setLayout(side_layout_);
  side_widget_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  top_layout_->setMargin(0);
  top_layout_->setSpacing(0);
  top_layout_->addLayout(stack_);

  QHBoxLayout* main_layout = new QHBoxLayout;
  main_layout->setMargin(0);
  main_layout->setSpacing(1);
  main_layout->addWidget(side_widget_);
  main_layout->addLayout(top_layout_);
  setLayout(main_layout);
}

void FancyTabWidget::AddTab(QWidget* tab, const QIcon& icon, const QString& label) {
  stack_->addWidget(tab);
  items_ << Item(icon, label);
}

void FancyTabWidget::AddSpacer(int size) {
  items_ << Item(size);
}

void FancyTabWidget::SetBackgroundPixmap(const QPixmap& pixmap) {
  background_pixmap_ = pixmap;
  update();
}

void FancyTabWidget::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event)
  QPainter painter(this);

  QRect rect = side_widget_->rect().adjusted(0, 0, 1, 0);
  rect = style()->visualRect(layoutDirection(), geometry(), rect);
  Utils::StyleHelper::verticalGradient(&painter, rect, rect);

  if (!background_pixmap_.isNull()) {
    QRect pixmap_rect(background_pixmap_.rect());
    pixmap_rect.moveTo(rect.topLeft());

    while (pixmap_rect.top() < rect.bottom()) {
      QRect source_rect(pixmap_rect.intersected(rect));
      source_rect.moveTo(0, 0);
      painter.drawPixmap(pixmap_rect.topLeft(), background_pixmap_, source_rect);
      pixmap_rect.moveTop(pixmap_rect.bottom() - 10);
    }
  }

  painter.setPen(Utils::StyleHelper::borderColor());
  painter.drawLine(rect.topRight(), rect.bottomRight());

  QColor light = Utils::StyleHelper::sidebarHighlight();
  painter.setPen(light);
  painter.drawLine(rect.bottomLeft(), rect.bottomRight());
}

int FancyTabWidget::current_index() const {
  return stack_->currentIndex();
}

void FancyTabWidget::SetCurrentIndex(int index) {
  if (FancyTabBar* bar = qobject_cast<FancyTabBar*>(tab_bar_)) {
    bar->setCurrentIndex(index);
  } else if (QTabBar* bar = qobject_cast<QTabBar*>(tab_bar_)) {
    bar->setCurrentIndex(index);
  } else {
    stack_->setCurrentIndex(index);
  }
}

void FancyTabWidget::ShowWidget(int index) {
  stack_->setCurrentIndex(index);
  emit CurrentChanged(index);
}

void FancyTabWidget::AddBottomWidget(QWidget* widget) {
  top_layout_->addWidget(widget);
}

void FancyTabWidget::SetMode(Mode mode) {
  // Remove previous tab bar
  delete tab_bar_;
  tab_bar_ = NULL;

  // Create new tab bar
  switch (mode) {
    case Mode_None:
    default:
      qDebug() << "Unknown fancy tab mode" << mode;
      // fallthrough

    case Mode_LargeSidebar: {
      FancyTabBar* bar = new FancyTabBar(this);
      side_layout_->insertWidget(0, bar);
      tab_bar_ = bar;

      foreach (const Item& item, items_) {
        if (item.type_ == Item::Type_Spacer)
          bar->addSpacer(item.spacer_size_);
        else
          bar->addTab(item.tab_icon_, item.tab_label_);
      }

      bar->setCurrentIndex(stack_->currentIndex());
      connect(bar, SIGNAL(currentChanged(int)), SLOT(ShowWidget(int)));

      break;
    }

    case Mode_Tabs:
      MakeTabBar(QTabBar::RoundedNorth, true, false);
      break;

    case Mode_IconOnlyTabs:
      MakeTabBar(QTabBar::RoundedNorth, false, true);
      break;

    case Mode_SmallSidebar:
      MakeTabBar(QTabBar::RoundedWest, true, true);
      break;
  }

  mode_ = mode;
  emit ModeChanged(mode);
}

void FancyTabWidget::contextMenuEvent(QContextMenuEvent* e) {
  if (!menu_) {
    menu_ = new QMenu(this);

    QSignalMapper* mapper = new QSignalMapper(this);
    QActionGroup* group = new QActionGroup(this);
    AddMenuItem(mapper, group, tr("Large sidebar"), Mode_LargeSidebar);
    AddMenuItem(mapper, group, tr("Small sidebar"), Mode_SmallSidebar);
    AddMenuItem(mapper, group, tr("Tabs on top"), Mode_Tabs);
    AddMenuItem(mapper, group, tr("Icons on top"), Mode_IconOnlyTabs);
    menu_->addActions(group->actions());

    connect(mapper, SIGNAL(mapped(int)), SLOT(SetMode(int)));
  }

  menu_->popup(e->globalPos());
}

void FancyTabWidget::AddMenuItem(QSignalMapper* mapper, QActionGroup* group,
                                 const QString& text, Mode mode) {
  QAction* action = group->addAction(text);
  action->setCheckable(true);
  mapper->setMapping(action, mode);
  connect(action, SIGNAL(triggered()), mapper, SLOT(map()));

  if (mode == mode_)
    action->setChecked(true);
}

void FancyTabWidget::MakeTabBar(QTabBar::Shape shape, bool text, bool icons) {
  QTabBar* bar = new QTabBar(this);
  bar->setShape(shape);
  bar->setDocumentMode(true);
  tab_bar_ = bar;

  if (shape == QTabBar::RoundedNorth)
    top_layout_->insertWidget(0, bar);
  else
    side_layout_->insertWidget(0, bar);

  foreach (const Item& item, items_) {
    if (item.type_ != Item::Type_Tab)
      continue;

    if (icons && text)
      bar->addTab(item.tab_icon_, item.tab_label_);
    else if (icons)
      bar->setTabToolTip(bar->addTab(item.tab_icon_, QString()), item.tab_label_);
    else if (text)
      bar->addTab(item.tab_label_);
  }

  bar->setCurrentIndex(stack_->currentIndex());
  connect(bar, SIGNAL(currentChanged(int)), SLOT(ShowWidget(int)));
}
