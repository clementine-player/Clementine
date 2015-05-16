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

#ifndef _CRYPTLITE_HMAC_H_
#define _CRYPTLITE_HMAC_H_

#include <string>
#include <sstream>
#include <cstring>
#include <cassert>
#include <iomanip>
#include <boost/cstdint.hpp>

namespace cryptlite {

template <typename T>
class hmac {

public:

    static const unsigned int BLOCK_SIZE = T::BLOCK_SIZE;
    static const unsigned int HASH_SIZE  = T::HASH_SIZE;

    static void calc(
            const char* text, int text_len,
            const char* key,  int key_len,
            boost::uint8_t digest[HASH_SIZE]) {
        assert(digest);
        calc(reinterpret_cast<const boost::uint8_t*>(text), text_len,
             reinterpret_cast<const boost::uint8_t*>(key), key_len, digest);
    }

    static void calc(
            const boost::uint8_t* text, int text_len,
            const boost::uint8_t* key,  int key_len,
            boost::uint8_t digest[HASH_SIZE]) {
        assert(digest);
        hmac<T> ctx(key, key_len);
        ctx.input(text, text_len);
        ctx.result(digest);
    }

    inline static void calc(
            const std::string& text,
            const std::string& key,
            boost::uint8_t digest[HASH_SIZE]) {
        assert(digest);
        calc(reinterpret_cast<const char*>(text.c_str()), text.size(),
             reinterpret_cast<const char*>(key.c_str()), key.size(), digest);
    }

    inline static std::string calc_hex(
            const std::string& text,
            const std::string& key ) {
        return calc_hex(reinterpret_cast<const boost::uint8_t*>(text.c_str()), text.size(),
                reinterpret_cast<const boost::uint8_t*>(key.c_str()), key.size());
    }

    static std::string calc_hex(
            const boost::uint8_t* text, int text_len,
            const boost::uint8_t* key,  int key_len ) {
        int i;
        boost::uint8_t digest[HASH_SIZE];
        assert(key);
        assert(text);
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        hmac<T> ctx(key, key_len);
        ctx.input(text, text_len);
        ctx.result(digest);
        for (i = 0; i < HASH_SIZE; i++)
            oss << std::setw(2) << (digest[i] & 0xff);
        oss << std::dec;
        return oss.str();
    }

    hmac(const boost::uint8_t* key, int key_len) : hasher_(T()) {
        assert(key);
        reset(key, key_len);
    }

    hmac(const std::string& key) : hasher_(T()) {
        reset(reinterpret_cast<const boost::uint8_t*>(key.c_str()), key.size());
    }

    ~hmac() { }

    inline void reset(const std::string& key) {
        reset(reinterpret_cast<const boost::uint8_t*>(key.c_str()), key.size());
    }

    void reset(const boost::uint8_t* key, int key_len) {

        int i;
        boost::uint8_t k_ipad[BLOCK_SIZE];
        boost::uint8_t tempkey[HASH_SIZE];

        assert(key);

        if (key_len > BLOCK_SIZE) {
            T sha;
            sha.input(key, key_len);
            sha.result(tempkey);
            key = tempkey;
            key_len = HASH_SIZE;
        }

        for (i=0; i < key_len; i++) {
            k_ipad[i]  = key[i] ^ 0x36;
            k_opad_[i] = key[i] ^ 0x5c;
        }

        for (; i < BLOCK_SIZE; i++) {
            k_ipad[i]  = 0x36;
            k_opad_[i] = 0x5c;
        }

        hasher_.reset();
        hasher_.input(k_ipad, BLOCK_SIZE);
    }

    inline void input(const std::string& text) {
        input(reinterpret_cast<const boost::uint8_t*>(text.c_str()), text.size());
    }

    void input(const boost::uint8_t* text, int text_len) {
        assert(text);
        hasher_.input(text, text_len);
    }

    void final_bits(const boost::uint8_t bits, unsigned int bitcount) {
        hasher_.final_bits(bits, bitcount);
    }

    void result(boost::uint8_t digest[HASH_SIZE]) {
        assert(digest);
        hasher_.result(digest);
        hasher_.reset();
        hasher_.input(k_opad_, BLOCK_SIZE);
        hasher_.input(digest, HASH_SIZE);
        hasher_.result(digest);
    }

private:
    boost::uint8_t k_opad_[BLOCK_SIZE];
    T hasher_;
}; // end of class

}  // end of namespace

#endif
