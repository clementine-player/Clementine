#ifndef EXTENSIONS_H
#define EXTENSIONS_H

#include <gloox/stanzaextension.h>
#include <gloox/tag.h>

#include "common.h"

namespace xrme {

template <typename T>
class XRMEExtension : public gloox::StanzaExtension {
 public:
  XRMEExtension();
  // gloox::StanzaExtension
  const std::string& filterString() const;
  StanzaExtension* newInstance(const gloox::Tag* tag) const;
  gloox::Tag* tag() const;
  StanzaExtension* clone() const;

  int extension_type() const {
    return T::kExtensionType;
  }

 protected:
  explicit XRMEExtension(const gloox::Tag* tag);
  explicit XRMEExtension(const XRMEExtension& other);

  static const char* kMediaPlayerFilterString;
  static const char* kRemoteControlFilterString;

  const std::string filter_string_;

  const gloox::Tag* tag_;
};

class RemoteControlExtension : public XRMEExtension<RemoteControlExtension> {
 public:
  static const int kExtensionType = gloox::ExtUser + 1;
  static const char* kFilterString;
};

class MediaPlayerExtension : public XRMEExtension<MediaPlayerExtension> {
 public:
  static const int kExtensionType = gloox::ExtUser + 2;
  static const char* kFilterString;
};

template <typename T>
XRMEExtension<T>::XRMEExtension()
    : StanzaExtension(T::kExtensionType),
      filter_string_(T::kFilterString),
      tag_(NULL) {
}

template <typename T>
const std::string& XRMEExtension<T>::filterString() const {
  return filter_string_;
}

template <typename T>
gloox::StanzaExtension* XRMEExtension<T>::newInstance(const gloox::Tag* tag) const {
  return new XRMEExtension(tag);
}

template <typename T>
gloox::Tag* XRMEExtension<T>::tag() const {
  return tag_ ? tag_->clone() : NULL;
}

template <typename T>
gloox::StanzaExtension* XRMEExtension<T>::clone() const {
  return new XRMEExtension(*this);
}

template <typename T>
XRMEExtension<T>::XRMEExtension(const gloox::Tag* tag)
    : StanzaExtension(T::kExtensionType),
      filter_string_(T::kFilterString),
      tag_(tag) {
}

template <typename T>
XRMEExtension<T>::XRMEExtension(const XRMEExtension& other)
    : StanzaExtension(T::kExtensionType),
      filter_string_(T::kFilterString),
      tag_(other.tag_) {
}

}  // namespace xrme

#endif  // EXTENSIONS_H
