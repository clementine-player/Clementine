/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

// This class assumes that std::basic_string<T> has a contiguous and null-terminated buffer.

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "tstring.h"
#include "tdebug.h"
#include "tstringlist.h"
#include "trefcounter.h"
#include "tutils.h"

#include <iostream>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
# include <windows.h>
#else
# include "unicode.h"
#endif

namespace
{
  using namespace TagLib;

  inline size_t UTF16toUTF8(
    const wchar_t *src, size_t srcLength, char *dst, size_t dstLength)
  {
    size_t len = 0;

#ifdef _WIN32

    len = ::WideCharToMultiByte(CP_UTF8, 0, src, srcLength, dst, dstLength, NULL, NULL);

#else

    using namespace Unicode;

    const UTF16 *srcBegin = src;
    const UTF16 *srcEnd   = srcBegin + srcLength;

    UTF8 *dstBegin = reinterpret_cast<UTF8*>(dst);
    UTF8 *dstEnd   = dstBegin + dstLength;

    ConversionResult result = ConvertUTF16toUTF8(
      &srcBegin, srcEnd, &dstBegin, dstEnd, lenientConversion);

    if(result == conversionOK)
      len = dstBegin - reinterpret_cast<UTF8*>(dst);

#endif

    if(len == 0)
      debug("String::UTF16toUTF8() - Unicode conversion error.");

    return len;
  }

  inline size_t UTF8toUTF16(
    const char *src, size_t srcLength, wchar_t *dst, size_t dstLength)
  {
    size_t len = 0;

#ifdef _WIN32

    len = ::MultiByteToWideChar(CP_UTF8, 0, src, srcLength, dst, dstLength);

#else

    using namespace Unicode;

    const UTF8 *srcBegin = reinterpret_cast<const UTF8*>(src);
    const UTF8 *srcEnd   = srcBegin + srcLength;

    UTF16 *dstBegin = dst;
    UTF16 *dstEnd   = dstBegin + dstLength;

    ConversionResult result = ConvertUTF8toUTF16(
      &srcBegin, srcEnd, &dstBegin, dstEnd, lenientConversion);

    if(result == conversionOK)
      len = dstBegin - dst;

#endif

    if(len == 0)
      debug("String::UTF8toUTF16() - Unicode conversion error.");

    return len;
  }
}

namespace TagLib {

class String::StringPrivate : public RefCounter
{
public:
  StringPrivate()
    : RefCounter()
  {
  }

  StringPrivate(const wstring &s)
    : RefCounter()
    , data(s)
  {
  }

  StringPrivate(uint n, wchar_t c)
    : RefCounter()
    , data(static_cast<size_t>(n), c)
  {
  }

  /*!
   * Stores string in UTF-16. The byte order depends on the CPU endian.
   */
  TagLib::wstring data;

  /*!
   * This is only used to hold the the most recent value of toCString().
   */
  std::string cstring;
};

String String::null;

////////////////////////////////////////////////////////////////////////////////

String::String()
  : d(new StringPrivate())
{
}

String::String(const String &s)
  : d(s.d)
{
  d->ref();
}

String::String(const std::string &s, Type t)
  : d(new StringPrivate())
{
  if(t == Latin1)
    copyFromLatin1(s.c_str(), s.length());
  else if(t == String::UTF8)
    copyFromUTF8(s.c_str(), s.length());
  else {
    debug("String::String() -- std::string should not contain UTF16.");
  }
}

String::String(const wstring &s, Type t)
  : d(new StringPrivate())
{
  if(t == UTF16 || t == UTF16BE || t == UTF16LE) {
    // This looks ugly but needed for the compatibility with TagLib1.8.
    // Should be removed in TabLib2.0.
    if (t == UTF16BE)
      t = WCharByteOrder;
    else if (t == UTF16LE)
      t = (WCharByteOrder == UTF16LE ? UTF16BE : UTF16LE);

    copyFromUTF16(s.c_str(), s.length(), t);
  }
  else {
    debug("String::String() -- TagLib::wstring should not contain Latin1 or UTF-8.");
  }
}

String::String(const wchar_t *s, Type t)
  : d(new StringPrivate())
{
  if(t == UTF16 || t == UTF16BE || t == UTF16LE) {
    // This looks ugly but needed for the compatibility with TagLib1.8.
    // Should be removed in TabLib2.0.
    if (t == UTF16BE)
      t = WCharByteOrder;
    else if (t == UTF16LE)
      t = (WCharByteOrder == UTF16LE ? UTF16BE : UTF16LE);

    copyFromUTF16(s, ::wcslen(s), t);
  }
  else {
    debug("String::String() -- const wchar_t * should not contain Latin1 or UTF-8.");
  }
}

String::String(const char *s, Type t)
  : d(new StringPrivate())
{
  if(t == Latin1)
    copyFromLatin1(s, ::strlen(s));
  else if(t == String::UTF8)
    copyFromUTF8(s, ::strlen(s));
  else {
    debug("String::String() -- const char * should not contain UTF16.");
  }
}

String::String(wchar_t c, Type t)
  : d(new StringPrivate())
{
  if(t == UTF16 || t == UTF16BE || t == UTF16LE)
    copyFromUTF16(&c, 1, t);
  else {
    debug("String::String() -- wchar_t should not contain Latin1 or UTF-8.");
  }
}

String::String(char c, Type t)
  : d(new StringPrivate(1, static_cast<uchar>(c)))
{
  if(t != Latin1 && t != UTF8) {
    debug("String::String() -- char should not contain UTF16.");
  }
}

String::String(const ByteVector &v, Type t)
  : d(new StringPrivate())
{
  if(v.isEmpty())
    return;

  if(t == Latin1)
    copyFromLatin1(v.data(), v.size());
  else if(t == UTF8)
    copyFromUTF8(v.data(), v.size());
  else
    copyFromUTF16(v.data(), v.size(), t);

  // If we hit a null in the ByteVector, shrink the string again.
  d->data.resize(::wcslen(d->data.c_str()));
}

////////////////////////////////////////////////////////////////////////////////

String::~String()
{
  if(d->deref())
    delete d;
}

std::string String::to8Bit(bool unicode) const
{
  const ByteVector v = data(unicode ? UTF8 : Latin1);
  return std::string(v.data(), v.size());
}

TagLib::wstring String::toWString() const
{
  return d->data;
}

const char *String::toCString(bool unicode) const
{
  d->cstring = to8Bit(unicode);
  return d->cstring.c_str();
}

const wchar_t *String::toCWString() const
{
  return d->data.c_str();
}

String::Iterator String::begin()
{
  detach();
  return d->data.begin();
}

String::ConstIterator String::begin() const
{
  return d->data.begin();
}

String::Iterator String::end()
{
  detach();
  return d->data.end();
}

String::ConstIterator String::end() const
{
  return d->data.end();
}

int String::find(const String &s, int offset) const
{
  return d->data.find(s.d->data, offset);
}

int String::rfind(const String &s, int offset) const
{
  return d->data.rfind(s.d->data, offset);
}

StringList String::split(const String &separator) const
{
  StringList list;
  for(int index = 0;;) {
    int sep = find(separator, index);
    if(sep < 0) {
      list.append(substr(index, size() - index));
      break;
    }
    else {
      list.append(substr(index, sep - index));
      index = sep + separator.size();
    }
  }
  return list;
}

bool String::startsWith(const String &s) const
{
  if(s.length() > length())
    return false;

  return substr(0, s.length()) == s;
}

String String::substr(uint position, uint n) const
{
  return String(d->data.substr(position, n));
}

String &String::append(const String &s)
{
  detach();
  d->data += s.d->data;
  return *this;
}

String String::upper() const
{
  String s;

  static int shift = 'A' - 'a';

  for(wstring::const_iterator it = d->data.begin(); it != d->data.end(); ++it) {
    if(*it >= 'a' && *it <= 'z')
      s.d->data.push_back(*it + shift);
    else
      s.d->data.push_back(*it);
  }

  return s;
}

TagLib::uint String::size() const
{
  return d->data.size();
}

TagLib::uint String::length() const
{
  return size();
}

bool String::isEmpty() const
{
  return d->data.size() == 0;
}

bool String::isNull() const
{
  return d == null.d;
}

ByteVector String::data(Type t) const
{
  switch(t)
  {
  case Latin1:
    {
      ByteVector v(size(), 0);
      char *p = v.data();

      for(wstring::const_iterator it = d->data.begin(); it != d->data.end(); it++)
        *p++ = static_cast<char>(*it);

      return v;
    }
  case UTF8:
    if(!d->data.empty())
    {
      ByteVector v(size() * 4 + 1, 0);

      const size_t len = UTF16toUTF8(
        d->data.c_str(), d->data.size(), v.data(), v.size());
      v.resize(len);

      return v;
    }
    else {
      return ByteVector::null;
    }
  case UTF16:
    {
      ByteVector v(2 + size() * 2, 0);
      char *p = v.data();

      // We use little-endian encoding here and need a BOM.

      *p++ = '\xff';
      *p++ = '\xfe';

      for(wstring::const_iterator it = d->data.begin(); it != d->data.end(); it++) {
        *p++ = static_cast<char>(*it & 0xff);
        *p++ = static_cast<char>(*it >> 8);
      }

      return v;
    }
  case UTF16BE:
    {
      ByteVector v(size() * 2, 0);
      char *p = v.data();

      for(wstring::const_iterator it = d->data.begin(); it != d->data.end(); it++) {
        *p++ = static_cast<char>(*it >> 8);
        *p++ = static_cast<char>(*it & 0xff);
      }

      return v;
    }
  case UTF16LE:
    {
      ByteVector v(size() * 2, 0);
      char *p = v.data();

      for(wstring::const_iterator it = d->data.begin(); it != d->data.end(); it++) {
        *p++ = static_cast<char>(*it & 0xff);
        *p++ = static_cast<char>(*it >> 8);
      }

      return v;
    }
  default:
    {
      debug("String::data() - Invalid Type value.");
      return ByteVector::null;
    }
  }
}

int String::toInt() const
{
  return toInt(0);
}

int String::toInt(bool *ok) const
{
  int value = 0;

  uint size = d->data.size();
  bool negative = size > 0 && d->data[0] == '-';
  uint start = negative ? 1 : 0;
  uint i = start;

  for(; i < size && d->data[i] >= '0' && d->data[i] <= '9'; i++)
    value = value * 10 + (d->data[i] - '0');

  if(negative)
    value = value * -1;

  if(ok)
    *ok = (size > start && i == size);

  return value;
}

String String::stripWhiteSpace() const
{
  wstring::const_iterator begin = d->data.begin();
  wstring::const_iterator end = d->data.end();

  while(begin != end &&
        (*begin == '\t' || *begin == '\n' || *begin == '\f' ||
         *begin == '\r' || *begin == ' '))
  {
    ++begin;
  }

  if(begin == end)
    return null;

  // There must be at least one non-whitespace character here for us to have
  // gotten this far, so we should be safe not doing bounds checking.

  do {
    --end;
  } while(*end == '\t' || *end == '\n' ||
          *end == '\f' || *end == '\r' || *end == ' ');

  return String(wstring(begin, end + 1));
}

bool String::isLatin1() const
{
  for(wstring::const_iterator it = d->data.begin(); it != d->data.end(); it++) {
    if(*it >= 256)
      return false;
  }
  return true;
}

bool String::isAscii() const
{
  for(wstring::const_iterator it = d->data.begin(); it != d->data.end(); it++) {
    if(*it >= 128)
      return false;
  }
  return true;
}

String String::number(int n) // static
{
  return Utils::formatString("%d", n);
}

TagLib::wchar &String::operator[](int i)
{
  detach();
  return d->data[i];
}

const TagLib::wchar &String::operator[](int i) const
{
  return d->data[i];
}

bool String::operator==(const String &s) const
{
  return (d == s.d || d->data == s.d->data);
}

bool String::operator!=(const String &s) const
{
  return !(*this == s);
}

bool String::operator==(const char *s) const
{
  const wchar_t *p = toCWString();

  while(*p != L'\0' || *s != '\0') {
    if(*p++ != static_cast<uchar>(*s++))
      return false;
  }
  return true;
}

bool String::operator!=(const char *s) const
{
  return !(*this == s);
}

bool String::operator==(const wchar_t *s) const
{
  return (d->data == s);
}

bool String::operator!=(const wchar_t *s) const
{
  return !(*this == s);
}

String &String::operator+=(const String &s)
{
  detach();

  d->data += s.d->data;
  return *this;
}

String &String::operator+=(const wchar_t *s)
{
  detach();

  d->data += s;
  return *this;
}

String &String::operator+=(const char *s)
{
  detach();

  for(int i = 0; s[i] != 0; i++)
    d->data += uchar(s[i]);
  return *this;
}

String &String::operator+=(wchar_t c)
{
  detach();

  d->data += c;
  return *this;
}

String &String::operator+=(char c)
{
  detach();

  d->data += uchar(c);
  return *this;
}

String &String::operator=(const String &s)
{
  if(&s == this)
    return *this;

  if(d->deref())
    delete d;
  d = s.d;
  d->ref();
  return *this;
}

String &String::operator=(const std::string &s)
{
  if(d->deref())
    delete d;

  d = new StringPrivate;
  copyFromLatin1(s.c_str(), s.length());

  return *this;
}

String &String::operator=(const wstring &s)
{
  if(d->deref())
    delete d;
  d = new StringPrivate(s);
  return *this;
}

String &String::operator=(const wchar_t *s)
{
  if(d->deref())
    delete d;

  d = new StringPrivate(s);
  return *this;
}

String &String::operator=(char c)
{
  if(d->deref())
    delete d;

  d = new StringPrivate(1, static_cast<uchar>(c));
  return *this;
}

String &String::operator=(wchar_t c)
{
  if(d->deref())
    delete d;

  d = new StringPrivate(1, c);
  return *this;
}

String &String::operator=(const char *s)
{
  if(d->deref())
    delete d;

  d = new StringPrivate;
  copyFromLatin1(s, ::strlen(s));

  return *this;
}

String &String::operator=(const ByteVector &v)
{
  if(d->deref())
    delete d;

  d = new StringPrivate;
  copyFromLatin1(v.data(), v.size());

  // If we hit a null in the ByteVector, shrink the string again.
  d->data.resize(::wcslen(d->data.c_str()));

  return *this;
}

bool String::operator<(const String &s) const
{
  return d->data < s.d->data;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void String::detach()
{
  if(d->count() > 1) {
    d->deref();
    d = new StringPrivate(d->data);
  }
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void String::copyFromLatin1(const char *s, size_t length)
{
  d->data.resize(length);

  for(size_t i = 0; i < length; ++i)
    d->data[i] = static_cast<uchar>(s[i]);
}

void String::copyFromUTF8(const char *s, size_t length)
{
  d->data.resize(length);

  if(length > 0) {
    const size_t len = UTF8toUTF16(s, length, &d->data[0], d->data.size());
    d->data.resize(len);
  }
}

void String::copyFromUTF16(const wchar_t *s, size_t length, Type t)
{
  bool swap;
  if(t == UTF16) {
    if(length >= 1 && s[0] == 0xfeff)
      swap = false; // Same as CPU endian. No need to swap bytes.
    else if(length >= 1 && s[0] == 0xfffe)
      swap = true;  // Not same as CPU endian. Need to swap bytes.
    else {
      debug("String::copyFromUTF16() - Invalid UTF16 string.");
      return;
    }

    s++;
    length--;
  }
  else
    swap = (t != WCharByteOrder);

  d->data.resize(length);
  if(length > 0) {
    if(swap) {
      for(size_t i = 0; i < length; ++i)
        d->data[i] = Utils::byteSwap(static_cast<ushort>(s[i]));
    }
    else {
      ::wmemcpy(&d->data[0], s, length);
    }
  }
}

void String::copyFromUTF16(const char *s, size_t length, Type t)
{
  bool swap;
  if(t == UTF16) {
    if(length < 2) {
      debug("String::copyFromUTF16() - Invalid UTF16 string.");
      return;
    }

    // Uses memcpy instead of reinterpret_cast to avoid an alignment exception.
    ushort bom;
    ::memcpy(&bom, s, 2);

    if(bom == 0xfeff)
      swap = false; // Same as CPU endian. No need to swap bytes.
    else if(bom == 0xfffe)
      swap = true;  // Not same as CPU endian. Need to swap bytes.
    else {
      debug("String::copyFromUTF16() - Invalid UTF16 string.");
      return;
    }

    s += 2;
    length -= 2;
  }
  else
    swap = (t != WCharByteOrder);

  d->data.resize(length / 2);
  for(size_t i = 0; i < length / 2; ++i) {
    ushort c;
    ::memcpy(&c, s, 2);
    if(swap)
      c = Utils::byteSwap(c);

    d->data[i] = static_cast<wchar_t>(c);
    s += 2;
  }
}

const String::Type String::WCharByteOrder
  = (Utils::systemByteOrder() == Utils::BigEndian) ? String::UTF16BE : String::UTF16LE;

}

////////////////////////////////////////////////////////////////////////////////
// related functions
////////////////////////////////////////////////////////////////////////////////

const TagLib::String operator+(const TagLib::String &s1, const TagLib::String &s2)
{
  TagLib::String s(s1);
  s.append(s2);
  return s;
}

const TagLib::String operator+(const char *s1, const TagLib::String &s2)
{
  TagLib::String s(s1);
  s.append(s2);
  return s;
}

const TagLib::String operator+(const TagLib::String &s1, const char *s2)
{
  TagLib::String s(s1);
  s.append(s2);
  return s;
}

std::ostream &operator<<(std::ostream &s, const TagLib::String &str)
{
  s << str.to8Bit();
  return s;
}

