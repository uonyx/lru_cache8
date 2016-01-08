///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Copyright (c) 2015 Ubaka Onyechi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef LRUCACHE8_H
#define LRUCACHE8_H

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// debug stuff
#define LRUCACHE8_DEBUG_DISABLE_BRANCH_FREE_LRU 0

// Is compiler is C++11 or newer?
#define LRUCACHE8_CPP11 (__cplusplus >= 201103L) || (_MSC_VER >= 1600)

// Use C++11 native 'hash' and 'equal_to' functions as defaults. 
// These provides better support for STL data types (but is platform-dependent).
#define LRUCACHE8_PREFER_CPP11_FUNCTION_DEFAULTS (LRUCACHE8_CPP11 && 1)

#if !LRUCACHE8_DEBUG_DISABLE_BRANCH_FREE_LRU && defined (LRUCACHE8_ENABLE_INTRINSICS) && defined (_MSC_VER)
#define LRUCACHE8_USE_INTRINSICS 
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>

#ifdef LRUCACHE8_USE_INTRINSICS
#include <intrin.h>
#define _lc8_nlz __lzcnt64
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#if LRUCACHE8_PREFER_CPP11_FUNCTION_DEFAULTS

#include <functional>

template<typename _Key, typename _Val, typename _KeyHash = std::hash<_Key>, typename _KeyEqual = std::equal_to<_Key> >

#else

#include <string>

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t LC8Hash_DJBX33X (const unsigned char *src, size_t sz)
{
  uint32_t hash = 5381;
  for (size_t i = 0; i < sz; ++i) { hash = ((hash << 5) + hash) ^ src [i]; }
  return hash;
}

template<typename _Key> struct LRU8HashBitwise
{
  uint32_t operator() (_Key k) const
  {
    return LC8Hash_DJBX33X (reinterpret_cast<const unsigned char *>(&k), sizeof (_Key));
  }
};

template<typename _Key> struct LRU8HashNumeric
{
  uint32_t operator() (_Key k) const
  {
    return static_cast<uint32_t>(k);
  }
};

#if LRUCACHE8_CPP11
#include <type_traits>
template<typename _Key> struct LRU8Hash : public LRU8HashBitwise<_Key>
{
  static_assert (std::is_enum<_Key>::value, "No default LRU8Hash available for this type");
};
#else
template<typename _Key> struct LRU8Hash
{
};
#endif

template<> struct LRU8Hash<char> : public LRU8HashNumeric<char> {};
template<> struct LRU8Hash<unsigned char> : public LRU8HashNumeric<unsigned char> {};
template<> struct LRU8Hash<short> : public LRU8HashNumeric<short> {};
template<> struct LRU8Hash<unsigned short> : public LRU8HashNumeric<unsigned short> {};
template<> struct LRU8Hash<int> : public LRU8HashNumeric<int> {};
template<> struct LRU8Hash<unsigned int> : public LRU8HashNumeric<unsigned int> {};
template<> struct LRU8Hash<long> : public LRU8HashBitwise<long> {};
template<> struct LRU8Hash<unsigned long> : public LRU8HashBitwise<unsigned long> {};
template<> struct LRU8Hash<long long> : public LRU8HashBitwise<long long> {};
template<> struct LRU8Hash<unsigned long long> : public LRU8HashBitwise<unsigned long long> {};
template<> struct LRU8Hash<std::string>
{
  uint32_t operator() (const std::string &k) const
  {
    return LC8Hash_DJBX33X (reinterpret_cast<const unsigned char *>(k.c_str ()), k.size ());
  }
};

template<typename _Key> struct LRU8Hash<_Key *>
{
  uint32_t operator() (_Key *k) const 
  { 
    return static_cast<uint32_t>(reinterpret_cast<size_t>(k)); 
  }
};

template<typename _Key> struct LRU8EqualTo
{
  bool operator() (const _Key &k1, const _Key &k2) const 
  { 
    return (k1 == k2); 
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

template<typename _Key, typename _Val, typename _KeyHash = LRU8Hash<_Key>, typename _KeyEqual = LRU8EqualTo<_Key> >

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class lru_cache8
{
private:

#if LRUCACHE8_CPP11
  static_assert (sizeof (uint64_t) == 8, "lru_cache8 not usable on this platform");
#endif

  static const uint8_t MAX_SIZE = sizeof (uint64_t);
  static const uint8_t MAX_SIZE_MINUS_1 = MAX_SIZE - 1;
  static const uint8_t IDX_INVALID = 0xff;

  struct node_t
  {
    _Key      m_key;
    _Val      m_val;
    uint32_t  m_hash;

    void set (_Key key, _Val val, uint32_t hash)
    {
      m_key = key;
      m_val = val;
      m_hash = hash;
    }

    node_t () : m_hash (0) {}
  };

public:

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  void write (const _Key &key, const _Val &val)
  {
    uint32_t h = (uint32_t) this->m_khash (key);
    uint8_t  i = h & MAX_SIZE_MINUS_1;
    uint8_t idx = m_lmap [i];

    uint8_t lru_idx = this->get_matrix_lru ();
    while ((idx != lru_idx) && (idx != IDX_INVALID))
    {
      // if key exists, update value
      node_t *n = &m_node [idx];
      if ((n->m_hash == h) && this->m_kequal (n->m_key, key))
      {
        n->m_val = val;
        this->set_matrix_mru (idx);
        return;
      }

      // probe next slot
      i = (i + 1) & MAX_SIZE_MINUS_1;
      idx = m_lmap [i];
    }

    m_lmap [i] = lru_idx;
    m_node [lru_idx].set (key, val, h);
    this->set_matrix_mru (lru_idx);
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  bool read (const _Key &key, _Val *val)
  {
    uint32_t h = (uint32_t) this->m_khash (key);
    uint8_t  i = h & MAX_SIZE_MINUS_1;
    uint8_t idx = m_lmap [i];

    uint8_t c = 0;
    while ((idx != IDX_INVALID) && (c++ < MAX_SIZE))
    {
      node_t *n = &m_node [idx];
      if ((n->m_hash == h) && this->m_kequal (n->m_key, key))
      {
        *val = n->m_val;
        this->set_matrix_mru (idx);
        return true;
      }

      i = (i + 1) & MAX_SIZE_MINUS_1;
      idx = m_lmap [i];
    }

    return false;
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  void clear ()
  {
    this->new_matrix ();
    memset (m_lmap, IDX_INVALID, sizeof (m_lmap));
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  lru_cache8 () : m_matrix (0)
  {
    this->clear ();
  }

private:

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  void new_matrix ()
  {
    /*
      the low-order 8 bits hold row 0 of the matrix
      the next 8 bits hold row 1, etc
      byte i, i + 1, i + 2 etc

      every 0-th bit of each byte holds column 0
      the next byte's 0-th bit holds column 1
      bits i, i + 8, i + 16, ... is set to 0

      00000000
      00000001
      00000011
      00000111
      00001111
      00011111
      00111111
      01111111
    */
#if 0
    uint64_t m = 0;
    uint8_t p = 0;
    for (uint8_t i = 1; i < MAX_SIZE; ++i)
    {
      uint8_t n = p | (1 << (i - 1));
      uint64_t n64 = static_cast<uint64_t>(n);
      uint64_t mask = (n64 << (MAX_SIZE * i));
      m |= mask;
      p = n;
    }

    m_matrix = m;
#else
    m_matrix = 0x7f3f1f0f07030100;
#endif
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  uint8_t get_matrix_lru ()
  {
    uint64_t m = m_matrix;

#if LRUCACHE8_DEBUG_DISABLE_BRANCH_FREE_LRU

    // search for zero byte    
    if ((m & 0x00000000000000ff) == 0) return 0;
    if ((m & 0x000000000000ff00) == 0) return 1;
    if ((m & 0x0000000000ff0000) == 0) return 2;
    if ((m & 0x00000000ff000000) == 0) return 3;
    if ((m & 0x000000ff00000000) == 0) return 4;
    if ((m & 0x0000ff0000000000) == 0) return 5;
    if ((m & 0x00ff000000000000) == 0) return 6;
    if ((m & 0xff00000000000000) == 0) return 7;
    return 0xff;

#else

    // search for zero byte (branch-free)
    static const uint64_t c = 0x7f7f7f7f7f7f7f7f;    
    uint64_t y = (m & c) + c;
    y = ~(y | m | c);                         // convert 0-bytes to 0x80 and non-0-bytes to 0x00

#ifdef LRUCACHE8_USE_INTRINSICS
    uint64_t n = _lc8_nlz (y);                // number of leading zero bits from the right
    uint8_t r = static_cast<uint8_t>(n >> 3); // convert bit count to byte count
    return 7u - r;                            // reverse index position to the left
#else    
    static const uint8_t nlzlut [128] =
    {
      0x00, 0x01, 0xff, 0x02, 0xff, 0xff, 0xff, 0x03,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x04,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x05,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x06,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07,
    };

    uint8_t idx = ((y * 0x0002040810204081) >> 56) - 1;
    uint8_t r = nlzlut [idx];
    return r;
#endif

#endif
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  void set_matrix_mru (uint8_t i)
  {
    // set row i (every bit of byte i) to 1s
    uint64_t rmask = 0xffull << (i << 3);
    m_matrix |= rmask;

    // set column i (all i-th bits of each byte) to 0s     
    uint64_t cmask = ~(0x0101010101010101 << i);
    m_matrix &= cmask;
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  node_t      m_node [MAX_SIZE];
  uint8_t     m_lmap [MAX_SIZE];
  uint64_t    m_matrix;
  _KeyHash    m_khash;
  _KeyEqual   m_kequal;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
