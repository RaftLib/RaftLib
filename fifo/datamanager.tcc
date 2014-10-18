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
#include <array>
#include <mutex>
#include <thread>
#include <queue>

#include "ringbuffertypes.hpp"
#include "bufferdata.tcc"

namespace dm{
enum access_key : int { allocate = 0, allocate_range, push, 
                        recycle, pop, peek, size, N };
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
      DataManager< T, B >::getlock( mutex_arr );
      /** nobody should have outstanding references to the old buff **/
      Buffer::Data< T, B > *old_buffer( (this)->get() );
      new_buffer->copyFrom( old_buffer );
      (this)->set( new_buffer );
      delete( old_buffer );
      DataManager< T, B >::unlock( mutex_arr );
   }

   auto get() noexcept -> Buffer::Data< T, B >*
   {
      struct Copy
      {
         Copy( Buffer::Data< T, B > *a, Buffer::Data< T, B > *b ) : a( a ),
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
      mutex_arr[ (std::size_t)key ].lock();
   }

   void exitBuffer( dm::access_key key ) noexcept
   {
      mutex_arr[ (std::size_t)key ].unlock();
   }


private:
   Buffer::Data< T, B > *buffer_b            = (Buffer::Data< T, B >*)0; 
   Buffer::Data< T, B > *buffer_a            = (Buffer::Data< T, B >*)1;
   
   typedef std::array< std::mutex, dm::access_key::N > mutex_arr_t;

   static void getlock( mutex_arr_t &mutex_arr )
   {
      std::queue< int > lock_queue;
      for( int index( 0 ); index < ( int )dm::access_key::N ; index++ )
      {
         lock_queue.push( index );
      }
      while( lock_queue.size() > 0 )
      {
         auto index( lock_queue.front() );
         lock_queue.pop();
         if( ! mutex_arr[ index ].try_lock() )
         {
            lock_queue.push( index );
         }
      }
   }

   static void unlock( mutex_arr_t &mutex_arr )
   {
      /** assume all are locked **/
      for( auto &mu : mutex_arr )
      {
         mu.unlock();
      }
   }

   mutex_arr_t       mutex_arr; 
};
#endif /* END _DATAMANAGER_TCC_ */
