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
   DataManager( )        = default;
   virtual ~DataManager() = default;
   
   void set( Buffer::Data< T, B > *buffer )
   {
      assert( buffer != nullptr );
      (this)->buffer = buffer;
   }

   void resize( Buffer::Data< T, B > *new_buffer, volatile bool &exit_buffer )
   {
      auto allclear = [&]() -> bool
      {
         for( auto &flag_array : thread_access )
         {
            if( flag_array.whole != 0 )
            {
               return( false );
            }
         }
         return( true );
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
         resizing = true;
         if( allclear() )
         {
            if( buffercondition() )
            {
               break;
            }
            resizing = false;
            std::this_thread::yield();
         }
      }
      /** nobody should have outstanding references to the old buff **/
      auto *old_buffer( get() );
      new_buffer->copyFrom( old_buffer );
      set( new_buffer );
      delete( old_buffer );
      resizing = false;
   }

   auto get() noexcept -> Buffer::Data< T, B >*
   {
      return( buffer );
   }

   void  enterBuffer( dm::access_key key ) noexcept
   {
      set_helper( key, 1 );
   }

   void exitBuffer( dm::access_key key ) noexcept
   {
      set_helper( key, 0 );
   }

   bool notResizing() noexcept
   {
      return( ! resizing ); 
   }

private:
   Buffer::Data< T, B > *buffer              = nullptr; 
   
   volatile bool                 resizing    = false;
   struct
   {
      union{
         std::uint64_t whole = 0; /** just in case, default zero **/
         std::uint8_t  flag[ 8 ];
      };
      std::uint8_t padding[ 56 /** 64 - 8, 64 byte padding **/ ];
   }thread_access[ 2 ];
   
   std::function< void ( dm::access_key key, const int val  ) > set_helper = [&]( dm::access_key key, int val )
   {
      if( (int) key <= (int) dm::push )
      {
         thread_access[ 0 ].flag[ (int) key ] = val;
      }
      else
      {
         thread_access[ 1 ].flag[ (int) key ] = val;
      }
   };
};
#endif /* END _DATAMANAGER_TCC_ */
