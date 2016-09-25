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
#include <cstdint>

#include "ringbuffertypes.hpp"
#include "bufferdata.tcc"
#include "defs.hpp"

namespace dm
{
/**
 * access_key - each one of these is to be used as a 
 * key for  buffer access functions.  Everything <= 
 * push is expected to be a write type function, everything
 * else is expected to be a read type operation.
 */
enum access_key : key_t { allocate       = 0, 
                          allocate_range = 1, 
                          push           = 3, 
                          recycle        = 4, 
                          pop            = 5, 
                          peek           = 6, 
                          size           = 7,
                          N };
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
   void set( Buffer::Data< T, B > * const buffer ) noexcept 
   {
      assert( buffer != nullptr );
      (this)->buffer = buffer;
      /** check to see if buffer is given is resizeable **/
      resizeable     = (  buffer->external_alloc ? false : true ); 
      /** 
       * need to set thread access structs, should be two of them, 
       * one for producer anad other for the consumer. These are 
       * kept as an array here so, only one ptr.
       */
      thread_access = buffer->thread_access;
   }

   inline bool is_resizeable() noexcept 
   {
      return( resizeable );
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
      auto allclear( []( ThreadAccess * const thread_access,
                         std::atomic< std::uint64_t >  * checking_size ) noexcept -> bool
      {
         assert( thread_access != nullptr );
         assert( checking_size != nullptr );
         for( int i( 0 ); i < 2; i++ )
         {
            if( thread_access[ i ].whole != 0 )
            {
               return( false );
            }
         }
         if( checking_size->load( std::memory_order_relaxed ) != 0 )
         {
            return( false );
         }
         return( true );
      } );

      /**
       * buffercondition - call to see if the 
       * current buffer state is amenable to
       * expanding.
       */
      auto buffercondition( []( Buffer::Data< T, B > * const buff_ptr ) noexcept -> bool
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
         const auto rpt( Pointer::val( buff_ptr->read_pt  ) );
         const auto wpt( Pointer::val( buff_ptr->write_pt ) );
         return( rpt < wpt );
      } );
      
      auto *old_buffer( get() );
      for(;;)
      {
         /** check to see if program is done **/
         if( exit_buffer /** comes from allocator **/ |
             ! old_buffer->is_valid  /** comes indirectly from scheduler **/ )
         {
            /** get rid of newly allocated buff, don't need **/
            delete( new_buffer );
            resizing = false;
            std::this_thread::yield();
            return;
         }
         /** set resizing global flag **/
         resizing = true;
         /** see if everybody is done with the current buffer **/
         if( allclear( thread_access, &checking_size ) )
         {
            /** check to see if the state of the buffer is good **/
            if( buffercondition( old_buffer ) )
            {
               break;
            }

#ifdef   PEEKTEST
            std::cerr << "Peek Loop\n";
#endif
         }
         resizing = false;
         std::this_thread::yield();
      }
      /** 
       * we can't do this with a simple copy constructor easily as 
       * at the time of new buffer creation we don't know the state
       * of the buffer at the time the conditions above were met, so
       * we have to do it once the happen. 
       * 
       * At this point nobody should have outstanding references to 
       * the old buff, free to copy.
       */
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
      /** don't check for nullptr here b/c it's a valid return type **/
      return( buffer );
   }

   /**
    * enterBuffer - call from the function with the
    * appropriate access key to signal that the buffer
    * will soon be in use.
    * @param - key, dm::access_key
    */
   void enterBuffer( const dm::access_key key ) noexcept
   {
      /** see lambda below **/
      set_helper( key, 
                  static_cast< dm::key_t >( 1 ),
                  thread_access,
                  &checking_size );
   }

   /**
    * exitBuffer - call from fifo function with the appropriate
    * access key to signal that the buffer is no longer
    * in use.
    * @param - key, dm::access_key
    */
   void exitBuffer( const dm::access_key key ) noexcept
   {
      /** see lambda below **/
      set_helper( key, 
                  static_cast< dm::key_t >( 0 ),
                  thread_access,
                  &checking_size );
   }

   /**
    * notResizing - called by various fifo functions
    * to check first entry flag before signalling enterBuffer()
    * @return bool - currently not resizing 
    */
   bool notResizing() noexcept
   {
      return( R_UNLIKELY( ! resizing ) ); 
   }
   

private:
   Buffer::Data< T, B > *buffer              = nullptr; 
   volatile bool         resizing            =  false;

   bool                  resizeable          = true;
   
   /** defined in threadaccess.hpp **/
   ThreadAccess *thread_access = nullptr;
  
   std::atomic< std::uint64_t >  checking_size = { 0 };

   static inline void set_helper( const dm::access_key key, 
                                  const dm::key_t      val,
                                  ThreadAccess *thread_access,
                                  std::atomic< 
                                    std::uint64_t > * const checking_size ) noexcept
   {
      assert( thread_access != nullptr );
      assert( checking_size != nullptr );

      switch( key )
      {
         case( dm::allocate ):
         {
            thread_access[ 0 ].flag[ dm::allocate ] = val;
         }
         break;
         case( dm::allocate_range ):
         {
            thread_access[ 0 ].flag[ dm::allocate_range ] = val;
         }
         break;
         case( dm::push ):
         {
            thread_access[ 0 ].flag[ dm::push ] = val;
         }
         break;
         case( dm::recycle ):
         {
            thread_access[ 1 ].flag[ dm::recycle ] = val;
         }
         break;
         case( dm::pop ):
         {
            thread_access[ 1 ].flag[ dm::pop ] = val;
         }
         break;
         case( dm::peek ):
         {
            thread_access[ 1 ].flag[ dm::peek ] = val;
         }
         break;
         case( dm::size ):
         {
            /** this one has to be atomic, multiple updaters **/
            if( val == 0 )
            {
                checking_size->fetch_sub( 1, std::memory_order_relaxed );
            }
            else
            {
                checking_size->fetch_add( 1, std::memory_order_relaxed );
            }
         }
         break;
         default:
            assert( false );
      }
      return;
   }
};
#endif /* END _DATAMANAGER_TCC_ */
