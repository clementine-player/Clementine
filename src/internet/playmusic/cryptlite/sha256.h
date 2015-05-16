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

#ifndef _CRYPTLITE_SHA256_H_
#define _CRYPTLITE_SHA256_H_
#include <string>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <cryptlite/base64.h>
#include <boost/cstdint.hpp>

namespace cryptlite {

#define SHA256_SHR(bits,word)      ((word) >> (bits))
#define SHA256_ROTL(bits,word)                         \
  (((word) << (bits)) | ((word) >> (32-(bits))))
#define SHA256_ROTR(bits,word)                         \
  (((word) >> (bits)) | ((word) << (32-(bits))))

#define SHA256_SIGMA0(word)   \
  (SHA256_ROTR( 2,word) ^ SHA256_ROTR(13,word) ^ SHA256_ROTR(22,word))
#define SHA256_SIGMA1(word)   \
  (SHA256_ROTR( 6,word) ^ SHA256_ROTR(11,word) ^ SHA256_ROTR(25,word))
#define SHA256_sigma0(word)   \
  (SHA256_ROTR( 7,word) ^ SHA256_ROTR(18,word) ^ SHA256_SHR( 3,word))
#define SHA256_sigma1(word)   \
  (SHA256_ROTR(17,word) ^ SHA256_ROTR(19,word) ^ SHA256_SHR(10,word))

#define SHA256_ADD_LENGTH(this, temp, length)   \
  (*temp = (this)->length_low_, (this)->corrupted_ = \
    (((this)->length_low_ += (length)) < *temp) && \
    (++((this)->length_high_) == 0) ? 1 : 0)

#define SHA256_CH(x, y, z)      (((x) & ((y) ^ (z))) ^ (z))
#define SHA256_MAJ(x, y, z)     (((x) & ((y) | (z))) | ((y) & (z)))
#define SHA256_PARITY(x, y, z)  ((x) ^ (y) ^ (z))

class sha256 {

 public:

  static const unsigned int BLOCK_SIZE     = 64;
  static const unsigned int HASH_SIZE      = 32;
  static const unsigned int HASH_SIZE_BITS = 256;

  static void hash(const std::string& s, boost::uint8_t digest[HASH_SIZE])
  {
    sha256 ctx;
    ctx.input(reinterpret_cast<const boost::uint8_t*>(s.c_str()), s.size());
    ctx.result(digest);
  }

  static std::string hash_hex(const std::string& s) 
  {
    int i;
    boost::uint8_t digest[HASH_SIZE];
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    sha256 ctx;
    ctx.input(reinterpret_cast<const boost::uint8_t*>(s.c_str()), s.size());
    ctx.result(digest);
    for (i = 0; i < HASH_SIZE; ++i)
      oss << std::setw(2) << (digest[i] & 0xff);
    oss << std::dec;
    return oss.str();
  }

  static std::string hash_base64(const std::string& s) {
    boost::uint8_t digest[HASH_SIZE];
    sha256 ctx;
    ctx.input(reinterpret_cast<const boost::uint8_t*>(s.c_str()), s.size());
    ctx.result(digest);
    return base64::encode_from_array(digest, HASH_SIZE);
  }

  sha256() 
    : computed_(false)
    , corrupted_(false)
    , length_low_(0)
    , length_high_(0)
    , message_block_index_(0)
  {
    intermediate_hash_[0] = 0x6A09E667; 
    intermediate_hash_[1] = 0xBB67AE85;
    intermediate_hash_[2] = 0x3C6EF372; 
    intermediate_hash_[3] = 0xA54FF53A;
    intermediate_hash_[4] = 0x510E527F; 
    intermediate_hash_[5] = 0x9B05688C; 
    intermediate_hash_[6] = 0x1F83D9AB; 
    intermediate_hash_[7] = 0x5BE0CD19;
  }

  ~sha256() { }

  void reset() 
  {
    computed_            = false;
    corrupted_           = false;
    length_low_          = 0;
    length_high_         = 0;
    message_block_index_ = 0;

    intermediate_hash_[0] = 0x6A09E667; 
    intermediate_hash_[1] = 0xBB67AE85;
    intermediate_hash_[2] = 0x3C6EF372; 
    intermediate_hash_[3] = 0xA54FF53A;
    intermediate_hash_[4] = 0x510E527F; 
    intermediate_hash_[5] = 0x9B05688C; 
    intermediate_hash_[6] = 0x1F83D9AB; 
    intermediate_hash_[7] = 0x5BE0CD19;
  }

  void input(const boost::uint8_t *message_array, unsigned int length)
  {
    assert(message_array);
    if (computed_ || corrupted_ || !length)
        return;
    boost::uint32_t temp;
    while (length-- && !corrupted_) {
      message_block_[message_block_index_++] = (*message_array & 0xFF);
      if (!SHA256_ADD_LENGTH(this, &temp, 8) && (message_block_index_ == BLOCK_SIZE))
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
    SHA256_ADD_LENGTH(this, &temp, length);
    finalize((boost::uint8_t)((message_bits & masks[length]) | markbit[length]));
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
    digest[20] = static_cast<boost::uint8_t>(intermediate_hash_[5] >> 24);
    digest[21] = static_cast<boost::uint8_t>(intermediate_hash_[5] >> 16);
    digest[22] = static_cast<boost::uint8_t>(intermediate_hash_[5] >>  8);
    digest[23] = static_cast<boost::uint8_t>(intermediate_hash_[5]      );
    digest[24] = static_cast<boost::uint8_t>(intermediate_hash_[6] >> 24);
    digest[25] = static_cast<boost::uint8_t>(intermediate_hash_[6] >> 16);
    digest[26] = static_cast<boost::uint8_t>(intermediate_hash_[6] >>  8);
    digest[27] = static_cast<boost::uint8_t>(intermediate_hash_[6]      );
    digest[28] = static_cast<boost::uint8_t>(intermediate_hash_[7] >> 24);
    digest[29] = static_cast<boost::uint8_t>(intermediate_hash_[7] >> 16);
    digest[30] = static_cast<boost::uint8_t>(intermediate_hash_[7] >>  8);
    digest[31] = static_cast<boost::uint8_t>(intermediate_hash_[7]      );
  }

private:
  boost::uint32_t intermediate_hash_[HASH_SIZE/4];
  boost::uint32_t length_low_;
  boost::uint32_t length_high_;
  boost::int_least16_t message_block_index_;
  boost::uint8_t message_block_[BLOCK_SIZE];
  bool computed_;
  bool corrupted_;

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
    length_low_ = 0;
    length_high_ = 0;
    computed_ = true;
  }

  void process_message_block()
  {
    static const boost::uint32_t K[64] = {
      0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b,
      0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
      0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7,
      0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
      0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152,
      0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
      0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
      0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
      0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819,
      0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08,
      0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f,
      0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
      0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };
    boost::uint32_t   temp1, temp2;
    boost::uint32_t   W[64];
    boost::uint32_t   A, B, C, D, E, F, G, H;

    W[0] = (((boost::uint32_t)message_block_[0]) << 24) |
        (((boost::uint32_t)message_block_[0 + 1]) << 16) |
        (((boost::uint32_t)message_block_[0 + 2]) << 8) |
        (((boost::uint32_t)message_block_[0 + 3]));
    W[1] = (((boost::uint32_t)message_block_[4]) << 24) |
        (((boost::uint32_t)message_block_[4 + 1]) << 16) |
        (((boost::uint32_t)message_block_[4 + 2]) << 8) |
        (((boost::uint32_t)message_block_[4 + 3]));
    W[2] = (((boost::uint32_t)message_block_[8]) << 24) |
        (((boost::uint32_t)message_block_[8 + 1]) << 16) |
        (((boost::uint32_t)message_block_[8 + 2]) << 8) |
        (((boost::uint32_t)message_block_[8 + 3]));
    W[3] = (((boost::uint32_t)message_block_[12]) << 24) |
        (((boost::uint32_t)message_block_[12 + 1]) << 16) |
        (((boost::uint32_t)message_block_[12 + 2]) << 8) |
        (((boost::uint32_t)message_block_[12 + 3]));
    W[4] = (((boost::uint32_t)message_block_[16]) << 24) |
        (((boost::uint32_t)message_block_[16 + 1]) << 16) |
        (((boost::uint32_t)message_block_[16 + 2]) << 8) |
        (((boost::uint32_t)message_block_[16 + 3]));
    W[5] = (((boost::uint32_t)message_block_[20]) << 24) |
        (((boost::uint32_t)message_block_[20 + 1]) << 16) |
        (((boost::uint32_t)message_block_[20 + 2]) << 8) |
        (((boost::uint32_t)message_block_[20 + 3]));
    W[6] = (((boost::uint32_t)message_block_[24]) << 24) |
        (((boost::uint32_t)message_block_[24 + 1]) << 16) |
        (((boost::uint32_t)message_block_[24 + 2]) << 8) |
        (((boost::uint32_t)message_block_[24 + 3]));
    W[7] = (((boost::uint32_t)message_block_[28]) << 24) |
        (((boost::uint32_t)message_block_[28 + 1]) << 16) |
        (((boost::uint32_t)message_block_[28 + 2]) << 8) |
        (((boost::uint32_t)message_block_[28 + 3]));
    W[8] = (((boost::uint32_t)message_block_[32]) << 24) |
        (((boost::uint32_t)message_block_[32 + 1]) << 16) |
        (((boost::uint32_t)message_block_[32 + 2]) << 8) |
        (((boost::uint32_t)message_block_[32 + 3]));
    W[9] = (((boost::uint32_t)message_block_[36]) << 24) |
        (((boost::uint32_t)message_block_[36 + 1]) << 16) |
        (((boost::uint32_t)message_block_[36 + 2]) << 8) |
        (((boost::uint32_t)message_block_[36 + 3]));
    W[10] = (((boost::uint32_t)message_block_[40]) << 24) |
        (((boost::uint32_t)message_block_[40 + 1]) << 16) |
        (((boost::uint32_t)message_block_[40 + 2]) << 8) |
        (((boost::uint32_t)message_block_[40 + 3]));
    W[11] = (((boost::uint32_t)message_block_[44]) << 24) |
        (((boost::uint32_t)message_block_[44 + 1]) << 16) |
        (((boost::uint32_t)message_block_[44 + 2]) << 8) |
        (((boost::uint32_t)message_block_[44 + 3]));
    W[12] = (((boost::uint32_t)message_block_[48]) << 24) |
        (((boost::uint32_t)message_block_[48 + 1]) << 16) |
        (((boost::uint32_t)message_block_[48 + 2]) << 8) |
        (((boost::uint32_t)message_block_[48 + 3]));
    W[13] = (((boost::uint32_t)message_block_[52]) << 24) |
        (((boost::uint32_t)message_block_[52 + 1]) << 16) |
        (((boost::uint32_t)message_block_[52 + 2]) << 8) |
        (((boost::uint32_t)message_block_[52 + 3]));
    W[14] = (((boost::uint32_t)message_block_[56]) << 24) |
        (((boost::uint32_t)message_block_[56 + 1]) << 16) |
        (((boost::uint32_t)message_block_[56 + 2]) << 8) |
        (((boost::uint32_t)message_block_[56 + 3]));
    W[15] = (((boost::uint32_t)message_block_[60]) << 24) |
        (((boost::uint32_t)message_block_[60 + 1]) << 16) |
        (((boost::uint32_t)message_block_[60 + 2]) << 8) |
        (((boost::uint32_t)message_block_[60 + 3]));
    W[16] = SHA256_sigma1(W[14]) + W[9] + SHA256_sigma0(W[1]) + W[0];
    W[17] = SHA256_sigma1(W[15]) + W[10] + SHA256_sigma0(W[2]) + W[1];
    W[18] = SHA256_sigma1(W[16]) + W[11] + SHA256_sigma0(W[3]) + W[2];
    W[19] = SHA256_sigma1(W[17]) + W[12] + SHA256_sigma0(W[4]) + W[3];
    W[20] = SHA256_sigma1(W[18]) + W[13] + SHA256_sigma0(W[5]) + W[4];
    W[21] = SHA256_sigma1(W[19]) + W[14] + SHA256_sigma0(W[6]) + W[5];
    W[22] = SHA256_sigma1(W[20]) + W[15] + SHA256_sigma0(W[7]) + W[6];
    W[23] = SHA256_sigma1(W[21]) + W[16] + SHA256_sigma0(W[8]) + W[7];
    W[24] = SHA256_sigma1(W[22]) + W[17] + SHA256_sigma0(W[9]) + W[8];
    W[25] = SHA256_sigma1(W[23]) + W[18] + SHA256_sigma0(W[10]) + W[9];
    W[26] = SHA256_sigma1(W[24]) + W[19] + SHA256_sigma0(W[11]) + W[10];
    W[27] = SHA256_sigma1(W[25]) + W[20] + SHA256_sigma0(W[12]) + W[11];
    W[28] = SHA256_sigma1(W[26]) + W[21] + SHA256_sigma0(W[13]) + W[12];
    W[29] = SHA256_sigma1(W[27]) + W[22] + SHA256_sigma0(W[14]) + W[13];
    W[30] = SHA256_sigma1(W[28]) + W[23] + SHA256_sigma0(W[15]) + W[14];
    W[31] = SHA256_sigma1(W[29]) + W[24] + SHA256_sigma0(W[16]) + W[15];
    W[32] = SHA256_sigma1(W[30]) + W[25] + SHA256_sigma0(W[17]) + W[16];
    W[33] = SHA256_sigma1(W[31]) + W[26] + SHA256_sigma0(W[18]) + W[17];
    W[34] = SHA256_sigma1(W[32]) + W[27] + SHA256_sigma0(W[19]) + W[18];
    W[35] = SHA256_sigma1(W[33]) + W[28] + SHA256_sigma0(W[20]) + W[19];
    W[36] = SHA256_sigma1(W[34]) + W[29] + SHA256_sigma0(W[21]) + W[20];
    W[37] = SHA256_sigma1(W[35]) + W[30] + SHA256_sigma0(W[22]) + W[21];
    W[38] = SHA256_sigma1(W[36]) + W[31] + SHA256_sigma0(W[23]) + W[22];
    W[39] = SHA256_sigma1(W[37]) + W[32] + SHA256_sigma0(W[24]) + W[23];
    W[40] = SHA256_sigma1(W[38]) + W[33] + SHA256_sigma0(W[25]) + W[24];
    W[41] = SHA256_sigma1(W[39]) + W[34] + SHA256_sigma0(W[26]) + W[25];
    W[42] = SHA256_sigma1(W[40]) + W[35] + SHA256_sigma0(W[27]) + W[26];
    W[43] = SHA256_sigma1(W[41]) + W[36] + SHA256_sigma0(W[28]) + W[27];
    W[44] = SHA256_sigma1(W[42]) + W[37] + SHA256_sigma0(W[29]) + W[28];
    W[45] = SHA256_sigma1(W[43]) + W[38] + SHA256_sigma0(W[30]) + W[29];
    W[46] = SHA256_sigma1(W[44]) + W[39] + SHA256_sigma0(W[31]) + W[30];
    W[47] = SHA256_sigma1(W[45]) + W[40] + SHA256_sigma0(W[32]) + W[31];
    W[48] = SHA256_sigma1(W[46]) + W[41] + SHA256_sigma0(W[33]) + W[32];
    W[49] = SHA256_sigma1(W[47]) + W[42] + SHA256_sigma0(W[34]) + W[33];
    W[50] = SHA256_sigma1(W[48]) + W[43] + SHA256_sigma0(W[35]) + W[34];
    W[51] = SHA256_sigma1(W[49]) + W[44] + SHA256_sigma0(W[36]) + W[35];
    W[52] = SHA256_sigma1(W[50]) + W[45] + SHA256_sigma0(W[37]) + W[36];
    W[53] = SHA256_sigma1(W[51]) + W[46] + SHA256_sigma0(W[38]) + W[37];
    W[54] = SHA256_sigma1(W[52]) + W[47] + SHA256_sigma0(W[39]) + W[38];
    W[55] = SHA256_sigma1(W[53]) + W[48] + SHA256_sigma0(W[40]) + W[39];
    W[56] = SHA256_sigma1(W[54]) + W[49] + SHA256_sigma0(W[41]) + W[40];
    W[57] = SHA256_sigma1(W[55]) + W[50] + SHA256_sigma0(W[42]) + W[41];
    W[58] = SHA256_sigma1(W[56]) + W[51] + SHA256_sigma0(W[43]) + W[42];
    W[59] = SHA256_sigma1(W[57]) + W[52] + SHA256_sigma0(W[44]) + W[43];
    W[60] = SHA256_sigma1(W[58]) + W[53] + SHA256_sigma0(W[45]) + W[44];
    W[61] = SHA256_sigma1(W[59]) + W[54] + SHA256_sigma0(W[46]) + W[45];
    W[62] = SHA256_sigma1(W[60]) + W[55] + SHA256_sigma0(W[47]) + W[46];
    W[63] = SHA256_sigma1(W[61]) + W[56] + SHA256_sigma0(W[48]) + W[47];

    A = intermediate_hash_[0];
    B = intermediate_hash_[1];
    C = intermediate_hash_[2];
    D = intermediate_hash_[3];
    E = intermediate_hash_[4];
    F = intermediate_hash_[5];
    G = intermediate_hash_[6];
    H = intermediate_hash_[7];

    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[0] + W[0];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[1] + W[1];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[2] + W[2];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[3] + W[3];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[4] + W[4];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[5] + W[5];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[6] + W[6];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[7] + W[7];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[8] + W[8];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[9] + W[9];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[10] + W[10];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[11] + W[11];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[12] + W[12];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[13] + W[13];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[14] + W[14];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[15] + W[15];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[16] + W[16];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[17] + W[17];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[18] + W[18];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[19] + W[19];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[20] + W[20];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[21] + W[21];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[22] + W[22];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[23] + W[23];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[24] + W[24];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[25] + W[25];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[26] + W[26];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[27] + W[27];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[28] + W[28];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[29] + W[29];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[30] + W[30];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[31] + W[31];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[32] + W[32];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[33] + W[33];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[34] + W[34];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[35] + W[35];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[36] + W[36];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[37] + W[37];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[38] + W[38];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[39] + W[39];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[40] + W[40];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[41] + W[41];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[42] + W[42];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[43] + W[43];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[44] + W[44];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[45] + W[45];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[46] + W[46];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[47] + W[47];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[48] + W[48];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[49] + W[49];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[50] + W[50];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[51] + W[51];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[52] + W[52];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[53] + W[53];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[54] + W[54];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[55] + W[55];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[56] + W[56];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[57] + W[57];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[58] + W[58];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[59] + W[59];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[60] + W[60];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[61] + W[61];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[62] + W[62];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;
    temp1 = H + SHA256_SIGMA1(E) + SHA256_CH(E,F,G) + K[63] + W[63];
    temp2 = SHA256_SIGMA0(A) + SHA256_MAJ(A,B,C);
    H = G;
    G = F;
    F = E;
    E = D + temp1;
    D = C;
    C = B;
    B = A;
    A = temp1 + temp2;

    intermediate_hash_[0] += A;
    intermediate_hash_[1] += B;
    intermediate_hash_[2] += C;
    intermediate_hash_[3] += D;
    intermediate_hash_[4] += E;
    intermediate_hash_[5] += F;
    intermediate_hash_[6] += G;
    intermediate_hash_[7] += H;

    message_block_index_ = 0;
  }
}; // end of class

}  // end of namespace

#endif
