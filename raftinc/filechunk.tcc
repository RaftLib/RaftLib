/**
 * filechunk.tcc - 
 * @author: Jonathan Beard
 * @version: Sun Oct  4 08:29:07 2020
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
#ifndef FILECHUNK_TCC
#define FILECHUNK_TCC  1

#include "chunkiterator.tcc"

namespace raft
{

template < std::size_t size = 65536 > struct filechunk
{
   constexpr filechunk() = default;

   filechunk( const filechunk< size > &other )
   {
      std::memcpy( buffer, other.buffer, other.length + 1 /** cp null term **/ );
      start_position = other.start_position;
      length = other.length;
   }
   
   constexpr filechunk< size >& operator = ( const filechunk< size > &other )
   {
        //TODO - this will work for now, but there's better things
        //that we can do. 
        std::memcpy( buffer, other.buffer, other.length + 1 /** cp null term **/ );
        start_position = other.start_position;
        length = other.length;
        return( *this );
   }
#pragma pack( push, 1 )
   char           buffer[ size ];
   std::size_t    start_position    = 0;
   std::size_t    length            = 0;
   std::uint64_t  index             = 0;
#pragma pack( pop )

   constexpr static std::size_t getChunkSize() noexcept
   {
      return( size );
   }

   friend std::ostream& operator << ( std::ostream &output, const filechunk< size > &c )
   {
      output << c.buffer;
      return( output );
   }

   chunk_iterator< size > begin() noexcept
   {
      return( chunk_iterator< size >( this ) );
   }

   chunk_iterator< size > end() noexcept
   {
      return( chunk_iterator< size >( this, length ) );
   }

   inline char operator []( const std::size_t n )
   {
      assert( n >= 0  && n < size );
      return( buffer[ n ] );
   }
};

} //end namespace raft

#endif /* END FILECHUNK_TCC */
