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

#ifndef _CRYPTLITE_SHA1_H_
#define _CRYPTLITE_SHA1_H_

#include <string>
#include <cassert>
#include <sstream>
#include <iomanip>
#include "base64.h"
#include <boost/cstdint.hpp>

#define SHA1_ROTL(bits, word) \
    (((word) << (bits))|((word) >> (32-(bits))))

#define SHA1_ADD_LENGTH(this, temp, length) \
    (*temp = (this)->length_low_, \
     (this)->corrupted_ = (((this)->length_low_ += (length)) < *temp) && \
     (++((this)->length_high_) == 0) ? 1 : 0)

#define SHA1_CH(x, y, z)      (((x) & ((y) ^ (z))) ^ (z))
#define SHA1_MAJ(x, y, z)     (((x) & ((y) | (z))) | ((y) & (z)))
#define SHA1_PARITY(x, y, z)  ((x) ^ (y) ^ (z))


namespace cryptlite {

class sha1 {

public:

  static const unsigned int BLOCK_SIZE     = 64;
  static const unsigned int HASH_SIZE      = 20;
  static const unsigned int HASH_SIZE_BITS = 160;

  static void hash(const std::string& s, boost::uint8_t digest[HASH_SIZE]) 
  {
    sha1 ctx;
    ctx.input(reinterpret_cast<const boost::uint8_t*>(s.c_str()), s.size());
    ctx.result(digest);
  }

  static std::string hash_hex(const std::string& s) 
  {
    int i;
    boost::uint8_t digest[HASH_SIZE];
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    sha1 ctx;
    ctx.input(reinterpret_cast<const boost::uint8_t*>(s.c_str()), s.size());
    ctx.result(digest);
    for (i = 0; i < HASH_SIZE; ++i)
      oss << std::setw(2) << (digest[i] & 0xff);
    oss << std::dec;
    return oss.str();
  }

  static std::string hash_base64(const std::string& s) {
    boost::uint8_t digest[HASH_SIZE];
    sha1 ctx;
    ctx.input(reinterpret_cast<const boost::uint8_t*>(s.c_str()), s.size());
    ctx.result(digest);
    return base64::encode_from_array(digest, HASH_SIZE);
  }

  sha1() 
    : computed_(false)
    , corrupted_(false)
    , length_low_(0)
    , length_high_(0)
    , message_block_index_(0)
  {
    intermediate_hash_[0] = 0x67452301;
    intermediate_hash_[1] = 0xEFCDAB89;
    intermediate_hash_[2] = 0x98BADCFE;
    intermediate_hash_[3] = 0x10325476;
    intermediate_hash_[4] = 0xC3D2E1F0;
  }

  ~sha1() { }

  void reset()
  {
    computed_            = false;
    corrupted_           = false;
    length_low_          = 0;
    length_high_         = 0;
    message_block_index_ = 0;

    intermediate_hash_[0] = 0x67452301;
    intermediate_hash_[1] = 0xEFCDAB89;
    intermediate_hash_[2] = 0x98BADCFE;
    intermediate_hash_[3] = 0x10325476;
    intermediate_hash_[4] = 0xC3D2E1F0;
  }

  void input(const boost::uint8_t* message_array, unsigned int length)
  {
    assert(message_array);
    if (computed_ || corrupted_ || !length)
      return;
    boost::uint32_t temp;
    while (length-- && !corrupted_) {
      message_block_[message_block_index_++] = (*message_array & 0xFF);
      if (!SHA1_ADD_LENGTH(this, &temp, 8) && (message_block_index_ == BLOCK_SIZE))
        process_message_block();
      ++message_array;
    }
  }

  void final_bits(const boost::uint8_t message_bits, unsigned int length)
  {
    boost::uint8_t masks[8] = {
      /* 0 0b00000000 */ 0x00, /* 1 0b10000000 */ 0x80,
      /* 2 0b11000000 */ 0xC0, /* 3 0b11100000 */ 0xE0,
      /* 4 0b11110000 */ 0xF0, /* 5 0b11111000 */ 0xF8,
      /* 6 0b11111100 */ 0xFC, /* 7 0b11111110 */ 0xFE
    };
    boost::uint8_t markbit[8] = {
      /* 0 0b10000000 */ 0x80, /* 1 0b01000000 */ 0x40,
      /* 2 0b00100000 */ 0x20, /* 3 0b00010000 */ 0x10,
      /* 4 0b00001000 */ 0x08, /* 5 0b00000100 */ 0x04,
      /* 6 0b00000010 */ 0x02, /* 7 0b00000001 */ 0x01
    };

    if (!length)
      return;

    if (computed_ || (length >= 8) || (length == 0))
      corrupted_ = true;

    if (corrupted_)
      return;

    boost::uint32_t temp;
    SHA1_ADD_LENGTH(this, &temp, length);
    finalize((boost::uint8_t)((message_bits & masks[length])|(markbit[length])));
  }

  void result(boost::uint8_t digest[HASH_SIZE])
  {
    assert(digest);
    if (corrupted_)
      return;
    if (!computed_)
      finalize(0x80);

    digest[ 0] = static_cast<boost::uint8_t>(intermediate_hash_[0] >> 24);
    digest[ 1] = static_cast<boost::uint8_t>(intermediate_hash_[0] >> 16);
    digest[ 2] = static_cast<boost::uint8_t>(intermediate_hash_[0] >>  8);
    digest[ 3] = static_cast<boost::uint8_t>(intermediate_hash_[0]      );
    digest[ 4] = static_cast<boost::uint8_t>(intermediate_hash_[1] >> 24);
    digest[ 5] = static_cast<boost::uint8_t>(intermediate_hash_[1] >> 16);
    digest[ 6] = static_cast<boost::uint8_t>(intermediate_hash_[1] >>  8);
    digest[ 7] = static_cast<boost::uint8_t>(intermediate_hash_[1]      );
    digest[ 8] = static_cast<boost::uint8_t>(intermediate_hash_[2] >> 24);
    digest[ 9] = static_cast<boost::uint8_t>(intermediate_hash_[2] >> 16);
    digest[10] = static_cast<boost::uint8_t>(intermediate_hash_[2] >>  8);
    digest[11] = static_cast<boost::uint8_t>(intermediate_hash_[2]      );
    digest[12] = static_cast<boost::uint8_t>(intermediate_hash_[3] >> 24);
    digest[13] = static_cast<boost::uint8_t>(intermediate_hash_[3] >> 16);
    digest[14] = static_cast<boost::uint8_t>(intermediate_hash_[3] >>  8);
    digest[15] = static_cast<boost::uint8_t>(intermediate_hash_[3]      );
    digest[16] = static_cast<boost::uint8_t>(intermediate_hash_[4] >> 24);
    digest[17] = static_cast<boost::uint8_t>(intermediate_hash_[4] >> 16);
    digest[18] = static_cast<boost::uint8_t>(intermediate_hash_[4] >>  8);
    digest[19] = static_cast<boost::uint8_t>(intermediate_hash_[4]      );

  }

private:

  boost::uint32_t intermediate_hash_[HASH_SIZE/4];
  boost::uint8_t message_block_[BLOCK_SIZE];
  bool computed_;
  bool corrupted_;
  boost::uint32_t length_low_;
  boost::uint32_t length_high_;
  boost::int_least16_t message_block_index_;

  void pad_message(boost::uint8_t pad_byte)
  {
    if (message_block_index_ >= (BLOCK_SIZE - 8)) {
      message_block_[message_block_index_++] = pad_byte;
      while (message_block_index_ < BLOCK_SIZE)
        message_block_[message_block_index_++] = 0;
      process_message_block();
    } else {
      message_block_[message_block_index_++] = pad_byte;
    }

    while (message_block_index_ < (BLOCK_SIZE - 8))
      message_block_[message_block_index_++] = 0;

    message_block_[56] = static_cast<boost::uint8_t>(length_high_ >> 24);
    message_block_[57] = static_cast<boost::uint8_t>(length_high_ >> 16);
    message_block_[58] = static_cast<boost::uint8_t>(length_high_ >>  8);
    message_block_[59] = static_cast<boost::uint8_t>(length_high_      );
    message_block_[60] = static_cast<boost::uint8_t>(length_low_  >> 24);
    message_block_[61] = static_cast<boost::uint8_t>(length_low_  >> 16);
    message_block_[62] = static_cast<boost::uint8_t>(length_low_  >>  8);
    message_block_[63] = static_cast<boost::uint8_t>(length_low_       );

    process_message_block();
  }

  void finalize(boost::uint8_t pad_byte)
  {
    int i;
    pad_message(pad_byte);
    for (i = 0; i < BLOCK_SIZE; ++i)
      message_block_[i] = 0;
    length_low_         = 0;
    length_high_        = 0;
    computed_           = true;
  }

  void process_message_block()
  {
    const boost::uint32_t K[4] = {
      0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6
    };
    boost::uint32_t   temp;
    boost::uint32_t   W[80];
    boost::uint32_t   A, B, C, D, E;

    W[0]  = ((boost::uint32_t)message_block_[0]) << 24;
    W[0] |= ((boost::uint32_t)message_block_[1]) << 16;
    W[0] |= ((boost::uint32_t)message_block_[2]) << 8;
    W[0] |= ((boost::uint32_t)message_block_[3]);
    W[1]  = ((boost::uint32_t)message_block_[4]) << 24;
    W[1] |= ((boost::uint32_t)message_block_[5]) << 16;
    W[1] |= ((boost::uint32_t)message_block_[6]) << 8;
    W[1] |= ((boost::uint32_t)message_block_[7]);
    W[2]  = ((boost::uint32_t)message_block_[8]) << 24;
    W[2] |= ((boost::uint32_t)message_block_[9]) << 16;
    W[2] |= ((boost::uint32_t)message_block_[10]) << 8;
    W[2] |= ((boost::uint32_t)message_block_[11]);
    W[3]  = ((boost::uint32_t)message_block_[12]) << 24;
    W[3] |= ((boost::uint32_t)message_block_[13]) << 16;
    W[3] |= ((boost::uint32_t)message_block_[14]) << 8;
    W[3] |= ((boost::uint32_t)message_block_[15]);
    W[4]  = ((boost::uint32_t)message_block_[16]) << 24;
    W[4] |= ((boost::uint32_t)message_block_[17]) << 16;
    W[4] |= ((boost::uint32_t)message_block_[18]) << 8;
    W[4] |= ((boost::uint32_t)message_block_[19]);
    W[5]  = ((boost::uint32_t)message_block_[20]) << 24;
    W[5] |= ((boost::uint32_t)message_block_[21]) << 16;
    W[5] |= ((boost::uint32_t)message_block_[22]) << 8;
    W[5] |= ((boost::uint32_t)message_block_[23]);
    W[6]  = ((boost::uint32_t)message_block_[24]) << 24;
    W[6] |= ((boost::uint32_t)message_block_[25]) << 16;
    W[6] |= ((boost::uint32_t)message_block_[26]) << 8;
    W[6] |= ((boost::uint32_t)message_block_[27]);
    W[7]  = ((boost::uint32_t)message_block_[28]) << 24;
    W[7] |= ((boost::uint32_t)message_block_[29]) << 16;
    W[7] |= ((boost::uint32_t)message_block_[30]) << 8;
    W[7] |= ((boost::uint32_t)message_block_[31]);
    W[8]  = ((boost::uint32_t)message_block_[32]) << 24;
    W[8] |= ((boost::uint32_t)message_block_[33]) << 16;
    W[8] |= ((boost::uint32_t)message_block_[34]) << 8;
    W[8] |= ((boost::uint32_t)message_block_[35]);
    W[9]  = ((boost::uint32_t)message_block_[36]) << 24;
    W[9] |= ((boost::uint32_t)message_block_[37]) << 16;
    W[9] |= ((boost::uint32_t)message_block_[38]) << 8;
    W[9] |= ((boost::uint32_t)message_block_[39]);
    W[10]  = ((boost::uint32_t)message_block_[40]) << 24;
    W[10] |= ((boost::uint32_t)message_block_[41]) << 16;
    W[10] |= ((boost::uint32_t)message_block_[42]) << 8;
    W[10] |= ((boost::uint32_t)message_block_[43]);
    W[11]  = ((boost::uint32_t)message_block_[44]) << 24;
    W[11] |= ((boost::uint32_t)message_block_[45]) << 16;
    W[11] |= ((boost::uint32_t)message_block_[46]) << 8;
    W[11] |= ((boost::uint32_t)message_block_[47]);
    W[12]  = ((boost::uint32_t)message_block_[48]) << 24;
    W[12] |= ((boost::uint32_t)message_block_[49]) << 16;
    W[12] |= ((boost::uint32_t)message_block_[50]) << 8;
    W[12] |= ((boost::uint32_t)message_block_[51]);
    W[13]  = ((boost::uint32_t)message_block_[52]) << 24;
    W[13] |= ((boost::uint32_t)message_block_[53]) << 16;
    W[13] |= ((boost::uint32_t)message_block_[54]) << 8;
    W[13] |= ((boost::uint32_t)message_block_[55]);
    W[14]  = ((boost::uint32_t)message_block_[56]) << 24;
    W[14] |= ((boost::uint32_t)message_block_[57]) << 16;
    W[14] |= ((boost::uint32_t)message_block_[58]) << 8;
    W[14] |= ((boost::uint32_t)message_block_[59]);
    W[15]  = ((boost::uint32_t)message_block_[60]) << 24;
    W[15] |= ((boost::uint32_t)message_block_[61]) << 16;
    W[15] |= ((boost::uint32_t)message_block_[62]) << 8;
    W[15] |= ((boost::uint32_t)message_block_[63]);

    W[16] = SHA1_ROTL(1, W[13] ^ W[8] ^ W[2] ^W[0]);
    W[17] = SHA1_ROTL(1, W[14] ^ W[9] ^ W[3] ^W[1]);
    W[18] = SHA1_ROTL(1, W[15] ^ W[10] ^ W[4] ^W[2]);
    W[19] = SHA1_ROTL(1, W[16] ^ W[11] ^ W[5] ^W[3]);
    W[20] = SHA1_ROTL(1, W[17] ^ W[12] ^ W[6] ^W[4]);
    W[21] = SHA1_ROTL(1, W[18] ^ W[13] ^ W[7] ^W[5]);
    W[22] = SHA1_ROTL(1, W[19] ^ W[14] ^ W[8] ^W[6]);
    W[23] = SHA1_ROTL(1, W[20] ^ W[15] ^ W[9] ^W[7]);
    W[24] = SHA1_ROTL(1, W[21] ^ W[16] ^ W[10] ^W[8]);
    W[25] = SHA1_ROTL(1, W[22] ^ W[17] ^ W[11] ^W[9]);
    W[26] = SHA1_ROTL(1, W[23] ^ W[18] ^ W[12] ^W[10]);
    W[27] = SHA1_ROTL(1, W[24] ^ W[19] ^ W[13] ^W[11]);
    W[28] = SHA1_ROTL(1, W[25] ^ W[20] ^ W[14] ^W[12]);
    W[29] = SHA1_ROTL(1, W[26] ^ W[21] ^ W[15] ^W[13]);
    W[30] = SHA1_ROTL(1, W[27] ^ W[22] ^ W[16] ^W[14]);
    W[31] = SHA1_ROTL(1, W[28] ^ W[23] ^ W[17] ^W[15]);
    W[32] = SHA1_ROTL(1, W[29] ^ W[24] ^ W[18] ^W[16]);
    W[33] = SHA1_ROTL(1, W[30] ^ W[25] ^ W[19] ^W[17]);
    W[34] = SHA1_ROTL(1, W[31] ^ W[26] ^ W[20] ^W[18]);
    W[35] = SHA1_ROTL(1, W[32] ^ W[27] ^ W[21] ^W[19]);
    W[36] = SHA1_ROTL(1, W[33] ^ W[28] ^ W[22] ^W[20]);
    W[37] = SHA1_ROTL(1, W[34] ^ W[29] ^ W[23] ^W[21]);
    W[38] = SHA1_ROTL(1, W[35] ^ W[30] ^ W[24] ^W[22]);
    W[39] = SHA1_ROTL(1, W[36] ^ W[31] ^ W[25] ^W[23]);
    W[40] = SHA1_ROTL(1, W[37] ^ W[32] ^ W[26] ^W[24]);
    W[41] = SHA1_ROTL(1, W[38] ^ W[33] ^ W[27] ^W[25]);
    W[42] = SHA1_ROTL(1, W[39] ^ W[34] ^ W[28] ^W[26]);
    W[43] = SHA1_ROTL(1, W[40] ^ W[35] ^ W[29] ^W[27]);
    W[44] = SHA1_ROTL(1, W[41] ^ W[36] ^ W[30] ^W[28]);
    W[45] = SHA1_ROTL(1, W[42] ^ W[37] ^ W[31] ^W[29]);
    W[46] = SHA1_ROTL(1, W[43] ^ W[38] ^ W[32] ^W[30]);
    W[47] = SHA1_ROTL(1, W[44] ^ W[39] ^ W[33] ^W[31]);
    W[48] = SHA1_ROTL(1, W[45] ^ W[40] ^ W[34] ^W[32]);
    W[49] = SHA1_ROTL(1, W[46] ^ W[41] ^ W[35] ^W[33]);
    W[50] = SHA1_ROTL(1, W[47] ^ W[42] ^ W[36] ^W[34]);
    W[51] = SHA1_ROTL(1, W[48] ^ W[43] ^ W[37] ^W[35]);
    W[52] = SHA1_ROTL(1, W[49] ^ W[44] ^ W[38] ^W[36]);
    W[53] = SHA1_ROTL(1, W[50] ^ W[45] ^ W[39] ^W[37]);
    W[54] = SHA1_ROTL(1, W[51] ^ W[46] ^ W[40] ^W[38]);
    W[55] = SHA1_ROTL(1, W[52] ^ W[47] ^ W[41] ^W[39]);
    W[56] = SHA1_ROTL(1, W[53] ^ W[48] ^ W[42] ^W[40]);
    W[57] = SHA1_ROTL(1, W[54] ^ W[49] ^ W[43] ^W[41]);
    W[58] = SHA1_ROTL(1, W[55] ^ W[50] ^ W[44] ^W[42]);
    W[59] = SHA1_ROTL(1, W[56] ^ W[51] ^ W[45] ^W[43]);
    W[60] = SHA1_ROTL(1, W[57] ^ W[52] ^ W[46] ^W[44]);
    W[61] = SHA1_ROTL(1, W[58] ^ W[53] ^ W[47] ^W[45]);
    W[62] = SHA1_ROTL(1, W[59] ^ W[54] ^ W[48] ^W[46]);
    W[63] = SHA1_ROTL(1, W[60] ^ W[55] ^ W[49] ^W[47]);
    W[64] = SHA1_ROTL(1, W[61] ^ W[56] ^ W[50] ^W[48]);
    W[65] = SHA1_ROTL(1, W[62] ^ W[57] ^ W[51] ^W[49]);
    W[66] = SHA1_ROTL(1, W[63] ^ W[58] ^ W[52] ^W[50]);
    W[67] = SHA1_ROTL(1, W[64] ^ W[59] ^ W[53] ^W[51]);
    W[68] = SHA1_ROTL(1, W[65] ^ W[60] ^ W[54] ^W[52]);
    W[69] = SHA1_ROTL(1, W[66] ^ W[61] ^ W[55] ^W[53]);
    W[70] = SHA1_ROTL(1, W[67] ^ W[62] ^ W[56] ^W[54]);
    W[71] = SHA1_ROTL(1, W[68] ^ W[63] ^ W[57] ^W[55]);
    W[72] = SHA1_ROTL(1, W[69] ^ W[64] ^ W[58] ^W[56]);
    W[73] = SHA1_ROTL(1, W[70] ^ W[65] ^ W[59] ^W[57]);
    W[74] = SHA1_ROTL(1, W[71] ^ W[66] ^ W[60] ^W[58]);
    W[75] = SHA1_ROTL(1, W[72] ^ W[67] ^ W[61] ^W[59]);
    W[76] = SHA1_ROTL(1, W[73] ^ W[68] ^ W[62] ^W[60]);
    W[77] = SHA1_ROTL(1, W[74] ^ W[69] ^ W[63] ^W[61]);
    W[78] = SHA1_ROTL(1, W[75] ^ W[70] ^ W[64] ^W[62]);
    W[79] = SHA1_ROTL(1, W[76] ^ W[71] ^ W[65] ^W[63]);

    A = intermediate_hash_[0];
    B = intermediate_hash_[1];
    C = intermediate_hash_[2];
    D = intermediate_hash_[3];
    E = intermediate_hash_[4];

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[0] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[1] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[2] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[3] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[4] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[5] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[6] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[7] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[8] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[9] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[10] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[11] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[12] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[13] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[14] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[15] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[16] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[17] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[18] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_CH(B, C, D) + E + W[19] + K[0];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[20] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[21] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[22] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[23] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[24] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[25] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[26] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[27] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[28] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[29] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[30] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[31] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[32] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[33] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[34] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[35] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[36] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[37] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[38] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[39] + K[1];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[40] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[41] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[42] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[43] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[44] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[45] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[46] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[47] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[48] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[49] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[50] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[51] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[52] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[53] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[54] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[55] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[56] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[57] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[58] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_MAJ(B, C, D) + E + W[59] + K[2];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[60] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[61] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[62] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[63] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[64] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[65] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[66] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[67] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[68] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[69] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[70] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[71] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[72] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[73] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[74] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[75] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[76] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[77] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[78] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    temp = SHA1_ROTL(5,A) + SHA1_PARITY(B, C, D) + E + W[79] + K[3];
    E = D;
    D = C;
    C = SHA1_ROTL(30,B);
    B = A;
    A = temp;

    intermediate_hash_[0] += A;
    intermediate_hash_[1] += B;
    intermediate_hash_[2] += C;
    intermediate_hash_[3] += D;
    intermediate_hash_[4] += E;

    message_block_index_ = 0;
  }

}; // end of class

}  // end of namespace

#endif
