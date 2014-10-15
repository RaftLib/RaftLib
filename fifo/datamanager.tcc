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
#include <atomic>

#include "ringbuffertypes.hpp"
#include "bufferdata.tcc"


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
      assert( new_buffer != nullptr );
      resizing = true;
      while( access_count != 0 )
      {
         /* spin */
      }
      /* get stuff from old buffer and put into new buffer */
      buffer->copyTo( new_buffer ); 
      delete( buffer );
      buffer = new_buffer;
      /* set resizing to false, we're done */
      resizing = false;
   }

   auto get() -> Buffer::Data< T, B >*
   {
      return( buffer );
   }

   void  enterBuffer() noexcept
   {
      access_count++;
   }

   void exitBuffer() noexcept
   {
      access_count--;
   }

   bool  notResizing()
   {
      return( ! resizing );
   }

private:
   Buffer::Data< T, B > *buffer            = nullptr; 
   volatile bool         resizing          = false;
   std::atomic< std::size_t > access_count = { 0 };
};
#endif /* END _DATAMANAGER_TCC_ */
