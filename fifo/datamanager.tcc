/**
 * datamanager.tcc - 
 * @author: Jonathan Beard
 * @version: Tue Oct 14 14:15:00 2014
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
#ifndef _DATAMANAGER_TCC_
#define _DATAMANAGER_TCC_  1
#include <cassert>
#include <cstddef>
#include <bitset>

#include "ringbuffertypes.hpp"
#include "bufferdata.tcc"

namespace dm{
enum access_key : int { allocate = 0, allocate_range, push, 
                        recycle, pop, peek, N };
}

template < class T, 
           Type::RingBufferType B,
           size_t SIZE = 0 > class DataManager
{
public:
   
   DataManager( ) = default;

   
   void set( Buffer::Data< T, B > *buffer )
   {
      assert( buffer != nullptr );
      buffer_a = buffer;
      buffer_b = buffer;
   }

   void resize( Buffer::Data< T, B > *new_buffer )
   {
      while( resizing ); /* already resizing, spin for a bit */
      resizing = true;
      while( flag.any() )
      {
         /** spin **/
      }
      /** nobody should have outstanding references to the old buff **/
      Buffer::Data< T, B > *old_buffer( (this)->get() );
      old_buffer->copyTo( new_buffer );
      (this)->set( new_buffer );
      delete( old_buffer );
      resizing = false;
   }

   auto get() noexcept -> Buffer::Data< T, B >*
   {
      struct Copy
      {
         Copy( Buffer::Data< T, B > *a, Buffer::Data< T, B > *b ) : a(  a ),
                                                                  b( b )
         {
         }
         Buffer::Data< T, B > *a = nullptr;
         Buffer::Data< T, B > *b = nullptr;
      } copy( buffer_a, buffer_b );
      while( copy.a != copy.b )
      {
         copy.a = buffer_a;
         copy.b = buffer_b;
      }
      return( copy.a );
   }

   void  enterBuffer( dm::access_key key ) noexcept
   {
      flag[ (std::size_t) key ] = true;
   }

   void exitBuffer( dm::access_key key ) noexcept
   {
      flag[ (std::size_t) key ] = false;
   }

   bool  notResizing() noexcept
   {
      return( ! resizing );
   }

private:
   Buffer::Data< T, B > *buffer_b            = (Buffer::Data< T, B >*)0; 
   Buffer::Data< T, B > *buffer_a            = (Buffer::Data< T, B >*)1;

   volatile bool                    resizing = false;
   std::bitset< dm::access_key::N > flag;
};
#endif /* END _DATAMANAGER_TCC_ */
