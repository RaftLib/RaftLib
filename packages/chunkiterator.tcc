/**
 * chunkiterator.tcc - 
 * @author: Jonathan Beard
 * @version: Sun Oct  5 08:49:11 2014
 * 
 * Copyright 2014 Jonathan Beard
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
#ifndef _CHUNKITERATOR_HPP_
#define _CHUNKITERATOR_HPP_  1
#include <iterator>
#include <map>
#include <thread>
#include <mutex>
#include <cstddef>

namespace raft
{
/** declare this guy **/
template < std::size_t size > struct filechunk;

template < std::size_t size > class chunk_iterator : public std::iterator< std::forward_iterator_tag, char >
{
public:
   chunk_iterator( filechunk< size > * const chunk ) : chunk( chunk )
   {
      /** nothing to do here **/
   }
   
   chunk_iterator( filechunk< size > * const chunk, 
                   const std::size_t index ) : chunk( chunk ),
                                               index( chunk->length ),
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

   virtual ~chunk_iterator()
   {
      /** nothing to delete **/
   }

   chunk_iterator& operator++() noexcept
   {
      index++;
      return( (*this) );
   }
   
   bool operator==(const chunk_iterator& rhs) noexcept
   {
      return( index == rhs.index );
   }

   bool operator!=(const chunk_iterator& rhs) noexcept
   {
      return( index != rhs.chunk->length );
   }

   char operator*() noexcept
   {
      return( chunk->buffer[ index ] ); 
   }

private:
   filechunk< size > * const       chunk;
   /** current index iterated with respect to the buffer **/
   std::size_t                     index = 0;
   bool                            is_end    = false;
};

}
#endif /* END _CHUNKITERATOR_HPP_ */
