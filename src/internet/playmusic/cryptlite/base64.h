/*
The MIT License

Copyright (c) 2011 lyo.kato@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef _CRYPTLITE_BASE64_H_
#define _CRYPTLITE_BASE64_H_
#include <string>
#include <sstream>
#include <cmath>
#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_array.hpp>
#include <boost/cstdint.hpp>

namespace cryptlite {

class base64 : public boost::noncopyable {

 public:

  static std::string 
  encode_from_string(const std::string& s)
  {
    return encode_from_array(reinterpret_cast<const boost::uint8_t*>(s.c_str()), s.size());
  }

  static std::string 
  encode_from_array(const boost::uint8_t* s, unsigned int size) 
  {
    std::ostringstream os;
    boost::uint8_t c1, c2, c3;
    unsigned int i = 0;

    while (i < size) {
      c1 = s[i++] & 0xff;
      if (i == size) {
        os << enctable[c1 >> 2];
        os << enctable[(c1 & 0x3) << 4];
        os << "==";
        break;
      }
      c2 = s[i++];
      if (i == size) {
        os << enctable[c1 >> 2];
        os << enctable[((c1 & 0x3) << 4)|((c2 & 0xf0) >> 4)];
        os << enctable[(c2 & 0xf) << 2];
        os << '=';
        break;
      }
      c3 = s[i++];
      os << enctable[c1 >> 2];
      os << enctable[((c1 & 0x3) << 4) | ((c2 & 0xf0) >> 4)];
      os << enctable[((c2 & 0xf) << 2) | ((c3 & 0xc0) >> 6)];
      os << enctable[c3 & 0x3f];
    }
    return os.str();
  }

  static boost::tuple<boost::shared_array<boost::uint8_t>, std::size_t> 
  decode_to_array(const std::string& s)
  {
    unsigned char c1, c2, c3, c4;
    unsigned int size = static_cast<unsigned int>(s.size());
    unsigned int i = 0;
    float dest_guide_size = static_cast<float>(size * 3) / 4;

    unsigned int reserved = static_cast<unsigned int>(std::ceil(dest_guide_size));

    boost::shared_array<boost::uint8_t> dest(new boost::uint8_t[reserved]);
    std::size_t dest_len = 0;

    while (i < size) {
      do {
        c1 = dectable[s[i++] & 0xff];
      } while (i < size && c1 == -1);
      if (c1 == -1)
        break;

      do {
        c2 = dectable[s[i++] & 0xff];
      } while (i < size && c2 == -1);
      if (c2 == -1)
        break;

      dest[dest_len++] = (static_cast<boost::uint8_t>(((c1 << 2)|((c2 & 0x30) >> 4)) & 0xff));

      do {
        c3 = s[i++] & 0xff;
        if (c3 == 61)
          return boost::make_tuple(dest, dest_len);
        c3 = dectable[c3];
      } while (i < size && c3 == -1);
      if (c3 == -1)
        break;

      dest[dest_len++] = (static_cast<boost::uint8_t>((((c2 & 0xf) << 4)|((c3 & 0x3c) >> 2)) & 0xff));

      do {
        c4 = s[i++] & 0xff;
        if (c4 == 61)
          return boost::make_tuple(dest, dest_len);
        c4 = dectable[c4];
      } while (i < size && c4 == -1);
      if (c4 == -1)
        break;

      dest[dest_len++] = static_cast<boost::uint8_t>((((c3 & 0x03) << 6)| c4) & 0xff);
    }
    return boost::make_tuple(dest, dest_len);
  }

  template <typename T>
  static void 
  decode(const std::string& s, T& dest)
  {
    unsigned char c1, c2, c3, c4;
    std::size_t size = s.size();
    int i= 0;
    float dest_guide_size = static_cast<float>(size * 3) / 4;

    dest.clear();
    /*
    dest.reserve(size);
    */
    unsigned int reserved = static_cast<unsigned int>(std::ceil(dest_guide_size));
    dest.reserve(reserved);
    /*
    unsigned short mod     = reserved % 4;
    unsigned short padding = (mod == 0) ? 0 : (4 - mod);
    dest.reserve(reserved + padding);
    */

    while (i < size) {
      do {
        c1 = dectable[s[i++] & 0xff];
      } while (i < size && c1 == -1);
      if (c1 == -1)
        break;

      do {
        c2 = dectable[s[i++] & 0xff];
      } while (i < size && c2 == -1);
      if (c2 == -1)
        break;

      dest.push_back(static_cast<boost::uint8_t>(((c1 << 2)|((c2 & 0x30) >> 4) & 0xff)));

      do {
        c3 = s[i++] & 0xff;
        if (c3 == 61)
            return;
        c3 = dectable[c3];
      } while (i < size && c3 == -1);
      if (c3 == -1)
        break;

      dest.push_back(static_cast<boost::uint8_t>((((c2 & 0xf) << 4)|((c3 & 0x3c) >> 2) & 0xff)));

      do {
        c4 = s[i++] & 0xff;
        if (c4 == 61)
          return;
        c4 = dectable[c4];
      } while (i < size && c4 == -1);
      if (c4 == -1)
        break;

      dest.push_back(static_cast<boost::uint8_t>((((c3 & 0x03) << 6)| c4) & 0xff));
    }
  }

 private:
  static const char enctable[65];
  static const char dectable[128];

}; // end of class

const char base64::enctable[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char base64::dectable[128] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};

}  // end of namespace

#endif
