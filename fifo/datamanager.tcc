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
#include <atomic>

#include "ringbuffertypes.hpp"
#include "bufferdata.tcc"

namespace dm{
/**
 * access_key - each one of these is to be used as a 
 * key for  buffer access functions.  Everything <= 
 * push is expected to be a write type function, everything
 * else is expected to be a read type operation.
 */
enum access_key : int { allocate = 0, allocate_range, push, 
                        recycle, pop, peek, N };
}

template < class T, 
           Type::RingBufferType B,
           size_t SIZE = 0 > class DataManager
{
public:
   DataManager( )         = default;
   virtual ~DataManager() = default;
   
   /**
    * set - set the buffer from the parameter
    * as the buffer to be managed by this
    * object.
    * @param   buffer - Buffer::Data< T, B >
    */
   void set( Buffer::Data< T, B > *buffer )
   {
      assert( buffer != nullptr );
      (this)->buffer = buffer;
   }

   /**
    * resize - resize the buffer currently held by this
    * object.  It is expected that the buffer passed in
    * by the parameter is larger than the current buffer.
    * a second param exit_buffer is also required and
    * should be available from the allocator object calling
    * this function.  When exit_buffer is set to exit, the
    * function returns without actually resizing the buffer
    * since the application has finished.
    * @param buffer, - Buffer::Data< T, B>
    * @param exit_alloc, - set to false initially, true
    * when the application is complete
    */
   void resize( Buffer::Data< T, B > *new_buffer, volatile bool &exit_buffer )
   {
      /**
       * allclear - call this function to see
       * if all fifo functions have completed
       * their operations.
       */
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

      /**
       * buffercondition - call to see if the 
       * current buffer state is amenable to
       * expanding.
       */
      auto buffercondition = [&]() -> bool
      {
         /** 
          * there's only a few conditions that you can copy
          * the ringbuffer in one contiguous section without
          * having to re-order the elements which is far 
          * more expensive in most cases than simply waiting.
          * The only real case is when rpt < wpt.  This should
          * happen quite a bit but we do have to make sure.  For
          * other bad things that could happen, see the conditions
          * in the size() function from the ringbufferheap.tcc 
          * file.
          */
         auto * const buff_ptr( (this)->get() );
         const auto rpt( Pointer::val( buff_ptr->read_pt  ) );
         const auto wpt( Pointer::val( buff_ptr->write_pt ) );
         return( rpt < wpt );
      };
      
      for(;;)
      {
         /** check to see if program is done **/
         if( exit_buffer )
         {
            /** get rid of newly allocated buff, don't need **/
            delete( new_buffer );
            return;
         }
         /** set resizing global flag **/
         resizing = true;
         /** see if everybody is done with the current buffer **/
         if( allclear() )
         {
            /** check to see if the state of the buffer is good **/
            if( buffercondition() )
            {
               break;
            }
         }
         resizing = false;
         std::this_thread::yield();
      }
      /** nobody should have outstanding references to the old buff **/
      auto *old_buffer( get() );
      new_buffer->copyFrom( old_buffer );
      set( new_buffer );
      delete( old_buffer );
      resizing = false;
   }
   
   /**
    * get - returns the current buffer object 
    * @return - Buffer< T, B >*
    */
   auto get() noexcept -> Buffer::Data< T, B >*
   {
      return( buffer );
   }

   /**
    * enterBuffer - call from the function with the
    * appropriate access key to signal that the buffer
    * will soon be in use.
    * @param - key, dm::access_key
    */
   void  enterBuffer( dm::access_key key ) noexcept
   {
      /** see lambda below **/
      set_helper( key, 1 );
   }

   /**
    * exitBuffer - call from fifo function with the appropriate
    * access key to signal that the buffer is no longer
    * in use.
    * @param - key, dm::access_key
    */
   void exitBuffer( dm::access_key key ) noexcept
   {
      /** see lambda below **/
      set_helper( key, 0 );
   }

   /**
    * notResizing - called by various fifo functions
    * to check first entry flag before signalling enterBuffer()
    * @return bool - currently not resizing 
    */
   bool notResizing() noexcept
   {
      return( ! resizing ); 
   }
   

private:
   Buffer::Data< T, B > *buffer              = nullptr; 
   volatile bool         resizing            =  false;
   
   
   struct ThreadAccess
   {
      union{
         std::uint64_t whole = 0; /** just in case, default zero **/
         std::uint8_t  flag[ 8 ];
      };
      std::uint8_t padding[ 56 /** 64 - 8, 64 byte padding **/ ];
   } __attribute__((aligned(64))) volatile thread_access[ 2 ];
   
   void set_helper( dm::access_key key, const int val )
   {
      if( (int) key <= (int) dm::push )
      {
         thread_access[ 0 ].flag[ (int) key ] = val;
      }
      else
      {
         thread_access[ 1 ].flag[ (int) key ] = val;
      }
      return;
   }
};
#endif /* END _DATAMANAGER_TCC_ */
