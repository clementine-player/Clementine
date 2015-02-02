/*
   Copyright 2005-2009 Last.fm Ltd. <mir@last.fm>

   This file is part of liblastfm.

   liblastfm is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   liblastfm is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with liblastfm.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __CIRCULAR_ARRAY_H
#define __CIRCULAR_ARRAY_H

#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <cstdlib> // for memset
#include <algorithm> // for max

#ifndef NULL
#define NULL 0
#endif

template< typename T >
class CircularArray 
{

public:

   typedef size_t size_type;

   /////////////////////////////////////////////////////////////

   // IMPORTANT: The distance must be redefined!!
   // See declaration of iterator from stl_iterator_base_types.h:
   //   template<typename _Category, typename _Tp, typename _Distance = ptrdiff_t,
   //            typename _Pointer = _Tp*, typename _Reference = _Tp&>
   //   struct iterator { ...

   // ---------- Forward declarations

   class iterator : 
      public std::iterator<std::random_access_iterator_tag, T>
   {
      // it should be by default because is an inner class, but I put it just to be sure..
      friend class CircularArray;

   private:
      iterator( size_type idx, T* pData, size_type size ) : _idx(idx), _pData(pData), _size(size) {}

   public:

      //typedef random_access_iterator_tag iterator_category;

      iterator() : _idx(0), _pData(NULL) {}

      iterator& operator++()
      { // preincrement
         _idx = (_idx + 1) % _size;
         return (*this);
      }

      iterator operator++(int)
      { // postincrement
         iterator _Tmp = *this;
         _idx = (_idx + 1) % _size;
         return (_Tmp);
      }

      void operator+=(size_type offs)
      {
         this->_idx = (_idx + offs) % _size;
      }

      iterator operator+(size_type offs) const 
      {
         size_type newIdx = (_idx + offs) % _size;
         iterator _Tmp(newIdx, _pData, _size);
         return _Tmp;
      }

      // return the distance between this iterator and it
      size_t operator-(const iterator& it) const
      {
         if ( this->_idx > it._idx )
            return this->_idx - it._idx;
         else
            return this->_idx + (_size - it._idx);
      }

      iterator operator-(size_type offs) const
      {
         size_type newIdx;

         if ( offs <= _idx )
            newIdx = _idx - offs;
         else
            newIdx = _size - ((_idx - offs) % _size); // note: should be ok, but to be checked better

         iterator _Tmp(newIdx, _pData, _size);
         return _Tmp;
      }

      iterator& operator--()
      { // predecrement
         if (_idx == 0)
            _idx = _size - 1;
         else
            --_idx;
         return (*this);
      }

      iterator operator--(int)
      { // postdecrement
         iterator _Tmp = *this;
         if (_idx == 0)
            _idx = _size - 1;
         else
            --_idx;
         return (_Tmp);
      }

      T& operator*() const
      { // return designated object
         return _pData[_idx];
      }

      T* operator->() const
      { // return pointer to class object
         return &_pData[_idx];
      }    

      /*    T& operator=(const T& right)
      { // assign reference right to _val
      return ( this->_idx = right._idx );
      }*/    

      bool operator==(const iterator& right) const
      { // test for iterator equality
         return ( this->_idx == right._idx );
      }

      bool operator!=(const iterator& right) const
      { // test for iterator inequality
         return ( this->_idx != right._idx );
      }

   protected:
      size_type  _idx;
      T*         _pData;
      size_type  _size;
   };

   /////////////////////////////////////////////////////////////


   CircularArray()
      : _headIdx(0), _pData(NULL), _size(0)
   { }

   CircularArray( size_type size )
      : _headIdx(0), _pData(NULL)
   {
      this->resize(size);
   }

   CircularArray( size_type size, const T& init )
      : _headIdx(0), _pData(NULL)
   {
      this->resize(size, init);
   }

   ~CircularArray()
   {
      this->clear();
   }

   // remember: it is not working (yet!) with negative numbers!
   T& operator[](size_type offset)
   {    
      return _pData[ (_headIdx + offset) % _size ];
   }

   void resize( size_type size )
   {
      _headIdx = 0;
      if ( size == _size )
         return;

      this->clear();
      _pData = new T[size];
      _size = size;
   }

   void resize( size_type size, const T& init )
   {
      this->resize(size, false);
      this->fill(init);
   }

   void fill( const T& val )
   {
      for (size_type i=0; i<_size; ++i)
         _pData[i] = val;
   }

   void zero_fill()
   {
      memset( _pData, 0, _size * sizeof(T) );
   }

   bool empty() const
   {
      return ( _pData == NULL );
   }

   void clear()
   {
      if (_pData)
         delete [] _pData;
      _pData = NULL;
      _headIdx = 0;
      _size = 0;
   }

   iterator head() const
   {
      if (_pData == NULL)
         std::cerr << "WARNING: iterator in CircularArray points to an empty CircularArray" << std::endl;
      return iterator(_headIdx, _pData, _size);
   }

   void shift_head( int offset )
   {
      if ( offset < 0)
      {
         int mod = (-offset) % (int)_size;
         mod -= (int)_headIdx;
         _headIdx = _size - mod;
      }
      else
         _headIdx = (_headIdx + offset) % _size;
   }

   size_type size() const
   {
      return _size;
   }

   //// to be changed to an input forward iterator
   //template <typename TIterator>
   //void get_data( TIterator toFillIt, size_type size = 0 )
   //{
   //  if ( size == 0 )
   //    size = _size;
   //  iterator it = head();
   //  
   //  for (size_type i = 0; i < size; ++i)
   //    *(toFillIt++) = *(it++);
   //}

   // IMPORTANT! Destination buffer MUST be the same size!
   void copy_buffer( T* pDest )
   {
      memcpy( pDest, _pData, sizeof(T)*_size );
   }

   // returns the buffer
   T* get_buffer() const
   {
      return _pData;
   }


private:

   size_type  _headIdx; // index
   T*         _pData;   // array of data
   size_type  _size;    // size of data

};

#endif // __CIRCULAR_ARRAY_H
