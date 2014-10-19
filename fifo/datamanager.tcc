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
#include <thread>
#include <bitset>
#include <atomic>

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

   void resize( Buffer::Data< T, B > *new_buffer, volatile bool &exit_buffer )
   {
      auto set = [&]( bool val )
      {
         resizing_a = val;
         resizing_b = val;
      };
      auto getflag = [&]()
      {
         struct{
            std::bitset< dm::access_key::N > a;
            std::bitset< dm::access_key::N > b;
         }copy;
         do
         {
            copy.a = flag_a;
            copy.b = flag_b;
         }while( copy.a != copy.b );
         return( (volatile bool) copy.a.any() );
      };
      auto buffercondition = [&]() -> bool
      {
         /** 
          * there's only a few conditions that you can copy
          * the ringbuffer in one contiguous section without
          * having to re-order the elements which is far 
          * more expensive in most cases than simply waiting.
          * 
          */
         auto * const buff_ptr( (this)->get() );
         const auto rpt( Pointer::val( buff_ptr->read_pt  ) );
         const auto wpt( Pointer::val( buff_ptr->write_pt ) );
         return( rpt < wpt );
      };
      for(;;)
      {
         if( exit_buffer )
         {
            /** get rid of newly allocated buff, don't need **/
            delete( new_buffer );
            return;
         }
         set( true );
         if( ! getflag() )
         {
            if( buffercondition() )
            {
               break;
            }
            set( false );
            std::this_thread::yield();
         }
      }
      /** nobody should have outstanding references to the old buff **/
      Buffer::Data< T, B > *old_buffer( (this)->get() );
      new_buffer->copyFrom( old_buffer );
      (this)->set( new_buffer );
      delete( old_buffer );
      set( false );
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
      flag_a[ (std::size_t) key ] = true;
      flag_b[ (std::size_t) key ] = true;
   }

   void exitBuffer( dm::access_key key ) noexcept
   {
      flag_a[ (std::size_t) key ] = false;
      flag_b[ (std::size_t) key ] = false;
   }

   bool notResizing() noexcept
   {
      struct
      {
         bool a;
         bool b;
      }copy;
      do
      {
         copy.a = resizing_a;
         copy.b = resizing_b;
      }while( copy.a != copy.b );
      return( ! copy.a );
   }

private:
   Buffer::Data< T, B > *buffer_b            = (Buffer::Data< T, B >*)0; 
   Buffer::Data< T, B > *buffer_a            = (Buffer::Data< T, B >*)1;
   
   std::atomic<  bool >                    resizing_a  = { false };
   std::atomic<  bool >                    resizing_b  = { false };

   std::bitset< dm::access_key::N > flag_a;
   std::bitset< dm::access_key::N > flag_b;
};
#endif /* END _DATAMANAGER_TCC_ */
