// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MAC_SCOPED_CFTYPEREF_H_
#define BASE_MAC_SCOPED_CFTYPEREF_H_

#include <CoreFoundation/CoreFoundation.h>

// ScopedCFTypeRef<> is patterned after scoped_ptr<>, but maintains ownership
// of a CoreFoundation object: any object that can be represented as a
// CFTypeRef.  Style deviations here are solely for compatibility with
// scoped_ptr<>'s interface, with which everyone is already familiar.
//
// When ScopedCFTypeRef<> takes ownership of an object (in the constructor or
// in reset()), it takes over the caller's existing ownership claim.  The
// caller must own the object it gives to ScopedCFTypeRef<>, and relinquishes
// an ownership claim to that object.  ScopedCFTypeRef<> does not call
// CFRetain().
template <typename CFT>
class ScopedCFTypeRef {
 public:
  typedef CFT element_type;

  explicit ScopedCFTypeRef(CFT object = nullptr) : object_(object) {}

  ~ScopedCFTypeRef() {
    if (object_) CFRelease(object_);
  }

  void reset(CFT object = nullptr) {
    if (object_) CFRelease(object_);
    object_ = object;
  }

  bool operator==(CFT that) const { return object_ == that; }

  bool operator!=(CFT that) const { return object_ != that; }

  operator CFT() const { return object_; }

  CFT get() const { return object_; }

  void swap(ScopedCFTypeRef& that) {
    CFT temp = that.object_;
    that.object_ = object_;
    object_ = temp;
  }

  // ScopedCFTypeRef<>::release() is like scoped_ptr<>::release.  It is NOT
  // a wrapper for CFRelease().  To force a ScopedCFTypeRef<> object to call
  // CFRelease(), use ScopedCFTypeRef<>::reset().
  CFT release() __attribute__((warn_unused_result)) {
    CFT temp = object_;
    object_ = nullptr;
    return temp;
  }

 private:
  CFT object_;

  Q_DISABLE_COPY(ScopedCFTypeRef);
};

#endif  // BASE_MAC_SCOPED_CFTYPEREF_H_
