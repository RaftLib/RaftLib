/**
 * chunkiterator.tcc - 
 * @author: Jonathan Beard
 * @version: Sun Oct  5 08:49:11 2014
 * 
 * Copyright 2020 Jonathan Beard
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef CHUNKITERATOR_HPP
#define CHUNKITERATOR_HPP  1
#include <iterator>
#include <map>
#include <thread>
#include <mutex>
#include <cstddef>

namespace raft
{
/** declare this guy **/
template < std::size_t size > struct filechunk;

template < std::size_t size > class chunk_iterator : 
    public std::iterator< std::forward_iterator_tag, char >
{
public:
   constexpr chunk_iterator( filechunk< size > * const chunk ) : chunk( chunk )
   {
      /** nothing to do here **/
   }
   
   chunk_iterator( filechunk< size > * const chunk, 
                   const std::size_t index ) : chunk( chunk ),
                                               index( index ),
                                               is_end( true )
   {
      /** nothing to do here **/
   }

   chunk_iterator( const chunk_iterator &it ) : chunk( it.chunk ),
                                                index( it.index ),
                                                is_end( it.is_end )
   {
      /** nothing to do here **/
   }

   virtual ~chunk_iterator() = default;

   chunk_iterator& operator++() noexcept
   {
      index++;
      return( (*this) );
   }

   template < typename T, 
              typename 
               std::enable_if< std::is_integral< T >::value >::type* = nullptr >
   chunk_iterator& operator += ( const T val )
   {
      index += val;
      return( *this );
   }

   template < typename T, 
              typename 
               std::enable_if< std::is_integral< T >::value >::type* = nullptr >
   inline
   chunk_iterator& operator - ( const T val )
   {
      index -= val;
      assert( index >= 0 );
      return( *this );
   }

   inline chunk_iterator& operator = ( const chunk_iterator &other )
   {
      index = other.index;
      (this)->chunk = other.chunk;
      is_end = other.is_end;
      return( *this );
   }

   inline bool operator <= ( const chunk_iterator &c )
   {
      return( index <= c.index );
   }
   
   inline bool operator < ( const chunk_iterator &c )
   {
      return( index < c.index );
   }
   
   inline bool operator == ( const chunk_iterator& rhs ) noexcept
   {
      return( index == rhs.index );
   }

   inline bool operator!=(const chunk_iterator& rhs) noexcept
   {
      return( ( index != rhs.index ) );
   }

   inline char operator*() noexcept
   {
      return( chunk->buffer[ index ] ); 
   }
   
   inline std::size_t location() noexcept
   {
      return( index + chunk->start_position );
   }


   char operator []( const std::size_t i )
   {
      return( (*chunk)[ index + i ] );
   }

private:
   filechunk< size > *             chunk;
   /** current index iterated with respect to the buffer **/
   std::size_t                     index = 0;
   bool                            is_end    = false;
};

}
#endif /* END CHUNKITERATOR_HPP */
