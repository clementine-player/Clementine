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

void MacLineEdit::setText(const QString& text) {
  wrapper_->setText(text);
}

void MacLineEdit::SetHint(const QString& hint) {
  wrapper_->SetHint(hint);
}
