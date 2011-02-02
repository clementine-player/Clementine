/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "maclineedit.h"

#import <Foundation/NSAutoreleasePool.h>

#include <QtDebug>

@interface SearchTarget : NSObject {
  SearchTargetWrapper* wrapper_;
}

- (id) initWithWrapper: (SearchTargetWrapper*)wrapper;
- (void) action;
@end

class SearchTargetWrapper {
 public:
  explicit SearchTargetWrapper(NSSearchField* search, MacLineEdit* lineedit);
  void TextChanged();

  QString text() const;
  void setText(const QString& text);

  void SetHint(const QString& hint);

  void SetEnabled(bool enabled);

 private:
  NSSearchField* search_;
  SearchTarget* target_;
  MacLineEdit* lineedit_;
};


@implementation SearchTarget
- (id) initWithWrapper: (SearchTargetWrapper*)wrapper {
  wrapper_ = wrapper;
  return self;
}

- (void) action {
  wrapper_->TextChanged();
}
@end

SearchTargetWrapper::SearchTargetWrapper(NSSearchField* search, MacLineEdit* lineedit)
    : search_(search),
      lineedit_(lineedit) {
  target_ = [[SearchTarget alloc] initWithWrapper:this];

  [[search cell] setTarget:target_];
  [[search cell] setAction:@selector(action)];
}

void SearchTargetWrapper::TextChanged() {
  NSString* text = [[search_ cell] stringValue];
  lineedit_->TextChanged(QString::fromUtf8([text UTF8String]));
}

QString SearchTargetWrapper::text() const {
  NSString* text = [[search_ cell] stringValue];
  return QString::fromUtf8([text UTF8String]);
}

void SearchTargetWrapper::setText(const QString& text) {
  NSString* t = [[NSString alloc] initWithUTF8String:text.toUtf8().constData()];
  [[search_ cell] setStringValue:t];
  [t release];
}

void SearchTargetWrapper::SetHint(const QString& hint) {
  NSString* t = [[NSString alloc] initWithUTF8String:hint.toUtf8().constData()];
  [[search_ cell] setPlaceholderString:t];
  [t release];
}

void SearchTargetWrapper::SetEnabled(bool enabled) {
  // TODO: implement this!
}

MacLineEdit::MacLineEdit(QWidget* parent)
    : QMacCocoaViewContainer(0, parent),
      LineEditInterface(this) {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

  NSSearchField* search = [[NSSearchField alloc] init];
  setCocoaView(search);

  wrapper_ = new SearchTargetWrapper(search, this);

  [search release];
  [pool release];  // Pool's closed.
}

MacLineEdit::~MacLineEdit() {
  delete wrapper_;
}

void MacLineEdit::paintEvent(QPaintEvent* e) {
  QMacCocoaViewContainer::paintEvent(e);
}

void MacLineEdit::TextChanged(const QString& text) {
  emit textChanged(text);
  emit textEdited(text);
}

QString MacLineEdit::text() const {
  return wrapper_->text();
}

void MacLineEdit::set_text(const QString& text) {
  wrapper_->setText(text);
}

void MacLineEdit::set_enabled(bool enabled) {
  wrapper_->SetEnabled(enabled);
}

void MacLineEdit::set_hint(const QString& hint) {
  wrapper_->SetHint(hint);
}
