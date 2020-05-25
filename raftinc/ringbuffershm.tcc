/**
 * ringbuffershm.tcc - 
 * @author: Jonathan Beard
 * @version: Sun Sep 11 05:03:58 2016
 * 
 * Copyright 2016 Jonathan Beard
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
#ifndef RAFTRINGBUFFERSHM_TCC
#define RAFTRINGBUFFERSHM_TCC  1

#include "portexception.hpp"
#include "ringbufferheap_abstract.tcc"
#include "defs.hpp"
#include "alloc_traits.tcc"
#include "prefetch.hpp"
#include "defs.hpp"
#include "sysschedutil.hpp"


/** inline alloc **/
template < class T >
class RingBufferBase<
    T,
    Type::SharedMemory,
    typename std::enable_if< inline_nonclass_alloc< T >::value >::type >
: public RingBufferBaseHeap< T, Type::SharedMemory >
{
public:
   RingBufferBase() : RingBufferBaseHeap< T, Type::SharedMemory >()
   {
   }

   virtual ~RingBufferBase() = default;

   virtual void deallocate()
   {
      /**
       * at this point nothing has been allocated 
       * externally, and the write pointer hasn't been
       * incremented so just unset the allocate flag
       * and then signal the data manager that we're
       * exiting the buffer 
       */
      (this)->producer_data.allocate_called = false;
      (this)->datamanager.exitBuffer( dm::allocate );
   }

   /**
    * send- releases the last item allocated by allocate() to
    * the queue.  Function will imply return if allocate wasn't
    * called prior to calling this function.
    * @param signal - const raft::signal signal, default: NONE
    */
   virtual void send( const raft::signal signal = raft::none )
   {
      if( R_UNLIKELY( ! (this)->producer_data.allocate_called ) )
      {
         return;
      }
      /** should be the end of the write, regardless of which allocate called **/
      auto * const buff_ptr( (this)->datamanager.get() );
      const size_t write_index( Pointer::val( buff_ptr->write_pt ) );
      buff_ptr->signal[ write_index ] = signal;
      (this)->producer_data.write_stats->bec.count++;
      (this)->producer_data.allocate_called = false;
      Pointer::inc( buff_ptr->write_pt );
      (this)->datamanager.exitBuffer( dm::allocate );
   }

   /**
    * send_range - releases the last item allocated by allocate_range() to
    * the queue.  Function will imply return if allocate wasn't
    * called prior to calling this function.
    * @param signal - const raft::signal signal, default: NONE
    */
   virtual void send_range( const raft::signal signal = raft::none )
   {
      if( ! (this)->producer_data.allocate_called )
      {
        return;
      }
      /** should be the end of the write, regardless of which allocate called **/
      const size_t write_index( Pointer::val( (this)->datamanager.get()->write_pt ) );
      (this)->datamanager.get()->signal[ write_index ] = signal;
      /* only need to inc one more **/
      auto &n_allocated( (this)->producer_data.n_allocated );
      Pointer::incBy( (this)->datamanager.get()->write_pt,
                      n_allocated );
      (this)->producer_data.write_stats->bec.count += n_allocated;
      (this)->producer_data.allocate_called = false;
      n_allocated     = 0;
      (this)->datamanager.exitBuffer( dm::allocate_range );
   }



   virtual void unpeek()
   {
      (this)->datamanager.exitBuffer( dm::peek );
   }

protected:

   /**
    * removes range items from the buffer, ignores
    * them without the copy overhead.
    */
   virtual void local_recycle( std::size_t range )
   {
      if( range == 0 )
      {
         /** do nothing **/
         return;
      }
      do{ /** at least one to remove **/
         for( ;; )
         {
            (this)->datamanager.enterBuffer( dm::recycle );
            if( (this)->datamanager.notResizing() )
            {
               if( (this)->size() > 0 )
               {
                  break;
               }
               else if( (this)->is_invalid() && (this)->size() == 0 )
               {
                  (this)->datamanager.exitBuffer( dm::recycle );
                  return;
               }
            }
            (this)->datamanager.exitBuffer( dm::recycle );
            raft::yield();
         }
         auto * const buff_ptr( (this)->datamanager.get() );
         /**
          * TODO, this whole func can be optimized a bit more
          * using the incBy func of Pointer
          */
         Pointer::inc( buff_ptr->read_pt );
         (this)->datamanager.exitBuffer( dm::recycle );
      }while( --range > 0 );
      return;
   }



   /**
    * local_allocate - get a reference to an object of type T at the
    * end of the queue.  Should be released to the queue using
    * the push command once the calling thread is done with the
    * memory.
    * @return T&, reference to memory location at head of queue
    */
   virtual void local_allocate( void **ptr )
   {
      for(;;)
      {
         (this)->datamanager.enterBuffer( dm::allocate );
         if( (this)->datamanager.notResizing() && (this)->space_avail() > 0  )
         {
            break;
         }
         (this)->datamanager.exitBuffer( dm::allocate );
         /** else, set stats,  spin **/
         auto &wr_stats( (this)->producer_data.write_stats->bec.blocked );
         if( wr_stats == 0 )
         {
            wr_stats = 1;
         }
#if __x86_64
         __asm__ volatile("\
           pause"
           :
           :
           : );
#endif
         raft::yield();
      }
      auto * const buff_ptr( (this)->datamanager.get() );
      const size_t write_index( Pointer::val( buff_ptr->write_pt ) );
      *ptr = (void*)&( buff_ptr->store[ write_index ] );
      (this)->producer_data.allocate_called = true;
      /** call exitBuffer during push call **/
   }

   virtual void local_allocate_n( void *ptr, const std::size_t n )
   {
      for( ;; )
      {
         (this)->datamanager.enterBuffer( dm::allocate_range );
         if( (this)->datamanager.notResizing() && (this)->space_avail() >= n )
         {
            break;
         }
         /** 
          * if capacity is in fact too little then:
          * 1) signal exit buffer
          * 2) spin
          * 3) hope the resize thread hits soon
          */
         if( (this)->capacity() < n )
         {
            ((this)->datamanager.get()->force_resize) = n;
         }
         (this)->datamanager.exitBuffer( dm::allocate_range );
         /** else, set stats,  spin **/
         auto &wr_stats( (this)->producer_data.write_stats->bec.blocked );
         if( wr_stats == 0 )
         {
            wr_stats = 1;
         }
#if __x86_64
       __asm__ volatile("\
         pause"
         :
         :
         : );
#endif
         raft::yield();
      }
      auto *container(
         reinterpret_cast< std::vector< std::reference_wrapper< T > >* >( ptr ) );
      /**
       * TODO, fix this condition where the user can ask for more,
       * double buffer.
       */
      /** iterate over range, pause if not enough items **/
      auto * const buff_ptr( (this)->datamanager.get() );
      std::size_t write_index( Pointer::val( buff_ptr->write_pt ) );
      for( std::size_t index( 0 ); index < n; index++ )
      {
         /**
          * TODO, fix this logic here, write index must get iterated, but
          * not here
          */
         container->emplace_back( buff_ptr->store[ write_index ] );
         buff_ptr->signal[ write_index ] = raft::none;
         write_index = ( write_index + 1 ) % buff_ptr->max_cap;
      }
      (this)->producer_data.n_allocated = 
        static_cast< decltype( (this)->producer_data.n_allocated ) >( n );
      (this)->producer_data.allocate_called = true;
      /** exitBuffer() called by push_range **/
   }


   /**
    * local_push - implements the pure virtual function from the
    * FIFO interface.  Takes a void ptr as the object which is
    * cast into the correct form and an raft::signal signal. If
    * the ptr is null, then the signal is pushed and the item
    * is junk.  This is an internal method so the only case where
    * ptr should be null is in the case of a system signal being
    * sent.
    * @param   item, void ptr
    * @param   signal, const raft::signal&
    */
   virtual void  local_push( void *ptr, const raft::signal &signal )
   {
      for(;;)
      {
         (this)->datamanager.enterBuffer( dm::push );
         if( (this)->datamanager.notResizing() )
         {
            if( (this)->space_avail() > 0 )
            {
               break;
            }
         }
         (this)->datamanager.exitBuffer( dm::push );
         /** else, set stats,  spin **/
         auto &wr_stats( (this)->producer_data.write_stats->bec.blocked );
         if( wr_stats == 0 )
         {
            wr_stats = 1;
         }
#if __x86_64
         __asm__ volatile("\
           pause"
           :
           :
           : );
#endif
         raft::yield();
      }
      auto * const buff_ptr( (this)->datamanager.get() );
       const size_t write_index( Pointer::val( buff_ptr->write_pt ) );
      if( ptr != nullptr )
      {
         T *item( reinterpret_cast< T* >( ptr ) );
          buff_ptr->store[ write_index ]          = *item;
          (this)->producer_data.write_stats->bec.count++;
       }
      buff_ptr->signal[ write_index ]         = signal;
       Pointer::inc( buff_ptr->write_pt );
      (this)->datamanager.exitBuffer( dm::push );
   }

   /**
    * local_pop - read one item from the ring buffer,
    * will block till there is data to be read.  If
    * ptr == nullptr then the item is just thrown away.
    * @return  T, item read.  It is removed from the
    *          q as soon as it is read
    */
   virtual void
   local_pop( void *ptr, raft::signal *signal )
   {
      for(;;)
      {
         (this)->datamanager.enterBuffer( dm::pop );
         if( (this)->datamanager.notResizing() )
         {
            if( (this)->size() > 0 )
            {
               break;
            }
            else if( (this)->is_invalid() && (this)->size() == 0 )
            {
               throw ClosedPortAccessException(
                  "Accessing closed port with pop call, exiting!!" );
            }
         }
         else
         {
            (this)->datamanager.exitBuffer( dm::pop );
            /** handle stats **/
            auto &rd_stats( (this)->consumer_data.read_stats->bec.blocked );
            if( rd_stats == 0 )
            {
               rd_stats  = 1;
            }
            raft::yield();
         }
      }
      auto * const buff_ptr( (this)->datamanager.get() );
      const std::size_t read_index( Pointer::val( buff_ptr->read_pt ) );
      if( signal != nullptr )
      {
         *signal = buff_ptr->signal[ read_index ];
      }
      assert( ptr != nullptr );
      /** gotta dereference pointer and copy **/
      T *item( reinterpret_cast< T* >( ptr ) );
      *item = buff_ptr->store[ read_index ];
      /** only increment here b/c we're actually reading an item **/
      (this)->consumer_data.read_stats->bec.count++;
      Pointer::inc( buff_ptr->read_pt );
      (this)->datamanager.exitBuffer( dm::pop );
   }


   /**
    * local_peek() - look at a reference to the head of the
    * ring buffer.  This doesn't remove the item, but it
    * does give the user a chance to take a look at it without
    * removing.
    * @return T&
    */
   virtual void local_peek(  void **ptr, raft::signal *signal )
   {
      for(;;)
      {

         (this)->datamanager.enterBuffer( dm::peek );
         if( (this)->datamanager.notResizing() )
         {
            if( (this)->size() > 0  )
            {
               break;
            }
            else if( (this)->is_invalid() && (this)->size() == 0 )
            {
               throw ClosedPortAccessException(
                  "Accessing closed port with local_peek call, exiting!!" );
            }
         }
         (this)->datamanager.exitBuffer( dm::peek );
#if  __x86_64
         __asm__ volatile("\
           pause"
           :
           :
           : );
#endif
         raft::yield();
      }
      auto * const buff_ptr( (this)->datamanager.get() );
      const auto read_index( Pointer::val( buff_ptr->read_pt ) );
      if( signal != nullptr )
      {
         *signal = buff_ptr->signal[ read_index ];
      }
      *ptr = reinterpret_cast< void* >( &( buff_ptr->store[ read_index ] ) );
      return;
      /**
       * exitBuffer() called when recycle is called, can't be sure the
       * reference isn't being used until all outside accesses to it are
       * invalidated from the buffer.
       */
   }

   virtual void local_peek_range( void **ptr,
                                  void **sig,
                                  const std::size_t n,
                                  std::size_t &curr_pointer_loc )
   {
      for(;;)
      {

         (this)->datamanager.enterBuffer( dm::peek );
         if( (this)->datamanager.notResizing() )
         {
            if( (this)->size() >= n  )
            {
               break;
            }
            else if( (this)->is_invalid() && (this)->size() == 0 )
            {
               throw ClosedPortAccessException(
                  "Accessing closed port with local_peek_range call, exiting!!" );
            }
            else if( (this)->is_invalid() && (this)->size() < n )
            {
               throw NoMoreDataException( "Too few items left on closed port, kernel exiting" );
            }
         }
         (this)->datamanager.exitBuffer( dm::peek );
#if  __x86_64
         __asm__ volatile("\
           pause"
           :
           :
           : );
#endif
         raft::yield();
      }

      /**
       * TODO, fix this condition where the user can ask for more,
       * double buffer.
       */
      /** iterate over range, pause if not enough items **/
      auto * const buff_ptr( (this)->datamanager.get() );
      const std::size_t cpl( Pointer::val( buff_ptr->read_pt ) );
      curr_pointer_loc = cpl;
      *sig =  reinterpret_cast< void* >(  &buff_ptr->signal[ cpl ] );
      *ptr =  buff_ptr->store;
      return;
   }

};

/*********************************
 FOR SHM, ALL CLASSES ARE ALLOCATED INLINE
 *********************************/
template < class T >
class RingBufferBase<
    T,
    Type::SharedMemory,
    typename std::enable_if< ! inline_nonclass_alloc< T >::value >::type >
: public RingBufferBaseHeap< T, Type::SharedMemory >
{
public:
   RingBufferBase() : RingBufferBaseHeap< T, Type::SharedMemory >()
   {
   }

   virtual ~RingBufferBase() = default;

   virtual void deallocate()
   {
      auto * const buff_ptr( (this)->datamanager.get() );
      const auto read_index( Pointer::val( buff_ptr->read_pt ) );
      auto * const ptr = reinterpret_cast< T* >( &( buff_ptr->store[ read_index ] ) );
      /** destruct **/
      ptr->~T();
      (this)->producer_data.allocate_called = false;
      (this)->datamanager.exitBuffer( dm::allocate );
   }

    /**
     * send- releases the last item allocated by allocate() to
     * the queue.  Function will imply return if allocate wasn't
     * called prior to calling this function.
     * @param signal - const raft::signal signal, default: NONE
     */
    virtual void send( const raft::signal signal = raft::none )
    {
        if( R_UNLIKELY( ! (this)->producer_data.allocate_called ) )
        {
           return;
        }
        /** should be the end of the write, regardless of which allocate called **/
        auto * const buff_ptr( (this)->datamanager.get() );
        const size_t write_index( Pointer::val( buff_ptr->write_pt ) );
        buff_ptr->signal[ write_index ] = signal;
        (this)->producer_data.write_stats->bec.count++;
        (this)->producer_data.allocate_called = false;
        Pointer::inc( buff_ptr->write_pt );
        (this)->datamanager.exitBuffer( dm::allocate );
    }

   /**
    * send_range - releases the last item allocated by allocate_range() to
    * the queue.  Function will imply return if allocate wasn't
    * called prior to calling this function.
    * @param signal - const raft::signal signal, default: NONE
    */
   virtual void send_range( const raft::signal signal = raft::none )
   {
        if( ! (this)->producer_data.allocate_called )
        {
            return;
        }
        auto * const buff_ptr( (this)->datamanager.get() );
        /** should be the end of the write, regardless of which allocate called **/
        const size_t write_index( Pointer::val( buff_ptr->write_pt ) );
        buff_ptr->signal[ write_index ] = signal;
        /* only need to inc one more, the rest have already**/
        auto &n_allocated( (this)->producer_data.n_allocated );
        Pointer::incBy( buff_ptr->write_pt,
                        n_allocated );
        
        (this)->producer_data.write_stats->bec.count += n_allocated;
        /** cleanup **/
        (this)->producer_data.allocate_called = false;
        n_allocated     = 0;
        (this)->datamanager.exitBuffer( dm::allocate_range );
   }


   virtual void unpeek()
   {
      (this)->datamanager.exitBuffer( dm::peek );
   }

protected:

   /**
    * removes range items from the buffer, ignores
    * them without the copy overhead.
    */
   virtual void local_recycle( std::size_t range )
   {
      if( range == 0 )
      {
         /** do nothing **/
         return;
      }
      do{ /** at least one to remove **/
         for( ;; )
         {
            (this)->datamanager.enterBuffer( dm::recycle );
            if( (this)->datamanager.notResizing() )
            {
               if( (this)->size() > 0 )
               {
                  break;
               }
               else if( (this)->is_invalid() && (this)->size() == 0 )
               {
                  (this)->datamanager.exitBuffer( dm::recycle );
                  return;
               }
            }
            (this)->datamanager.exitBuffer( dm::recycle );
            raft::yield();
         }
         auto * const buff_ptr( (this)->datamanager.get() );
         const size_t read_index( Pointer::val( buff_ptr->read_pt ) );
         auto *ptr =
            reinterpret_cast< T* >( &( buff_ptr->store[ read_index ] ) );
         /** call destructor direct, faster than recyle func **/
         ptr->~T();
         Pointer::inc( buff_ptr->read_pt );
         (this)->datamanager.exitBuffer( dm::recycle );
      }while( --range > 0 );
      return;
   }



   /**
    * local_allocate - get a reference to an object of type T at the
    * end of the queue.  Should be released to the queue using
    * the push command once the calling thread is done with the
    * memory.
    * @return T&, reference to memory location at head of queue
    */
   virtual void local_allocate( void **ptr )
   {
      for(;;)
      {
         (this)->datamanager.enterBuffer( dm::allocate );
         if( (this)->datamanager.notResizing() && (this)->space_avail() > 0  )
         {
            break;
         }
         (this)->datamanager.exitBuffer( dm::allocate );
         /** else, set stats,  spin **/
         auto &wr_stats( (this)->producer_data.write_stats->bec.blocked );
         if( wr_stats == 0 )
         {
            wr_stats = 1;
         }
#if __x86_64
         __asm__ volatile("\
           pause"
           :
           :
           : );
#endif
         raft::yield();
      }
      auto * const buff_ptr( (this)->datamanager.get() );
      const size_t write_index( Pointer::val( buff_ptr->write_pt ) );
      *ptr = (void*)&( buff_ptr->store[ write_index ] );
      (this)->producer_data.allocate_called = true;
      /** call exitBuffer during push call **/
   }

   virtual void local_allocate_n( void *ptr, const std::size_t n )
   {
      for( ;; )
      {
         (this)->datamanager.enterBuffer( dm::allocate_range );
         if( (this)->datamanager.notResizing() && (this)->space_avail() >= n )
         {
            break;
         }
         (this)->datamanager.exitBuffer( dm::allocate_range );
         /** else, set stats,  spin **/
         auto &wr_stats( (this)->producer_data.write_stats->bec.blocked );
         if( wr_stats == 0 )
         {
            wr_stats = 1;
         }
#if __x86_64
       __asm__ volatile("\
         pause"
         :
         :
         : );
#endif
         raft::yield();
      }
      auto *container(
         reinterpret_cast< std::vector< std::reference_wrapper< T > >* >( ptr ) );
      /**
       * TODO, fix this condition where the user can ask for more,
       * double buffer.
       */
      /** iterate over range, pause if not enough items **/
      auto * const buff_ptr( (this)->datamanager.get() );
      std::size_t write_index( Pointer::val( buff_ptr->write_pt ) );
      for( std::size_t index( 0 ); index < n; index++ )
      {
         /**
          * TODO, fix this logic here, write index must get iterated, but
          * not here
          */
         container->emplace_back( buff_ptr->store[ write_index ] );
         buff_ptr->signal[ write_index ] = raft::none;
         write_index = ( write_index + 1 ) % buff_ptr->max_cap;
      }
      (this)->producer_data.n_allocated = 
        static_cast< decltype( (this)->producer_data.n_allocated ) >( n );
      (this)->producer_data.allocate_called = true;
      /** exitBuffer() called by push_range **/
   }


   /**
    * local_push - implements the pure virtual function from the
    * FIFO interface.  Takes a void ptr as the object which is
    * cast into the correct form and an raft::signal signal. If
    * the ptr is null, then the signal is pushed and the item
    * is junk.  This is an internal method so the only case where
    * ptr should be null is in the case of a system signal being
    * sent.
    * @param   item, void ptr
    * @param   signal, const raft::signal&
    */
   virtual void  local_push( void *ptr, const raft::signal &signal )
   {
      for(;;)
      {
         (this)->datamanager.enterBuffer( dm::push );
         if( (this)->datamanager.notResizing() )
         {
            if( (this)->space_avail() > 0 )
            {
               break;
            }
         }
         (this)->datamanager.exitBuffer( dm::push );
         /** else, set stats,  spin **/
         auto &wr_stats( (this)->producer_data.write_stats->bec.blocked );
         if( wr_stats == 0 )
         {
            wr_stats = 1;
         }
#if __x86_64
         __asm__ volatile("\
           pause"
           :
           :
           : );
#endif
         raft::yield();
      }
      auto * const buff_ptr( (this)->datamanager.get() );
       const size_t write_index( Pointer::val( buff_ptr->write_pt ) );
      if( ptr != nullptr )
      {
          T *item( reinterpret_cast< T* >( ptr ) );
          T * temp( new (
            &buff_ptr->store[ write_index ]
          ) T( *item  ) );
          UNUSED( temp );
          (this)->producer_data.write_stats->bec.count++;
       }
      buff_ptr->signal[ write_index ]         = signal;
       Pointer::inc( buff_ptr->write_pt );
#if 0
      if( signal == raft::quit )
      {
         (this)->write_finished = true;
      }
#endif      
      (this)->datamanager.exitBuffer( dm::push );
   }

   /**
    * local_pop - read one item from the ring buffer,
    * will block till there is data to be read.  If
    * ptr == nullptr then the item is just thrown away.
    * @return  T, item read.  It is removed from the
    *          q as soon as it is read
    */
   virtual void
   local_pop( void *ptr, raft::signal *signal )
   {
      for(;;)
      {
         (this)->datamanager.enterBuffer( dm::pop );
         if( (this)->datamanager.notResizing() )
         {
            if( (this)->size() > 0 )
            {
               break;
            }
            else if( (this)->is_invalid() && (this)->size() == 0 )
            {
               throw ClosedPortAccessException(
                  "Accessing closed port with pop call, exiting!!" );
            }
         }
         else
         {
            (this)->datamanager.exitBuffer( dm::pop );
            /** handle stats **/
            auto &rd_stats( (this)->consumer_data.read_stats->bec.blocked );
            if( rd_stats == 0 )
            {
               rd_stats  = 1;
            }
            raft::yield();
         }
      }
      auto * const buff_ptr( (this)->datamanager.get() );
      const std::size_t read_index( Pointer::val( buff_ptr->read_pt ) );
      if( signal != nullptr )
      {
         *signal = buff_ptr->signal[ read_index ];
      }
      assert( ptr != nullptr );
      /** gotta dereference pointer and copy **/
      T *item( reinterpret_cast< T* >( ptr ) );
      *item = buff_ptr->store[ read_index ];
      /** only increment here b/c we're actually reading an item **/
      (this)->consumer_data.read_stats->bec.count++;
      Pointer::inc( buff_ptr->read_pt );
      (this)->datamanager.exitBuffer( dm::pop );
   }


   /**
    * local_peek() - look at a reference to the head of the
    * ring buffer.  This doesn't remove the item, but it
    * does give the user a chance to take a look at it without
    * removing.
    * @return T&
    */
   virtual void local_peek(  void **ptr, raft::signal *signal )
   {
      for(;;)
      {

         (this)->datamanager.enterBuffer( dm::peek );
         if( (this)->datamanager.notResizing() )
         {
            if( (this)->size() > 0  )
            {
               break;
            }
            else if( (this)->is_invalid() && (this)->size() == 0 )
            {
               throw ClosedPortAccessException(
                  "Accessing closed port with local_peek call, exiting!!" );
            }
         }
         (this)->datamanager.exitBuffer( dm::peek );
#if  __x86_64
         __asm__ volatile("\
           pause"
           :
           :
           : );
#endif
         raft::yield();
      }
      auto * const buff_ptr( (this)->datamanager.get() );
      const size_t read_index( Pointer::val( buff_ptr->read_pt ) );
      if( signal != nullptr )
      {
         *signal = buff_ptr->signal[ read_index ];
      }
      *ptr = (void*) &( buff_ptr->store[ read_index ] );
      return;
      /**
       * exitBuffer() called when recycle is called, can't be sure the
       * reference isn't being used until all outside accesses to it are
       * invalidated from the buffer.
       */
   }

   virtual void local_peek_range( void **ptr,
                                  void **sig,
                                  const std::size_t n,
                                  std::size_t &curr_pointer_loc )
   {
      for(;;)
      {

         (this)->datamanager.enterBuffer( dm::peek );
         if( (this)->datamanager.notResizing() )
         {
            if( (this)->size() >= n  )
            {
               break;
            }
            else if( (this)->is_invalid() && (this)->size() == 0 )
            {
               throw ClosedPortAccessException(
                  "Accessing closed port with local_peek_range call, exiting!!" );
            }
            else if( (this)->is_invalid() && (this)->size() < n )
            {
               throw NoMoreDataException( "Too few items left on closed port, kernel exiting" );
            }
         }
         (this)->datamanager.exitBuffer( dm::peek );
#if  __x86_64
         __asm__ volatile("\
           pause"
           :
           :
           : );
#endif
         raft::yield();
      }

      /**
       * TODO, fix this condition where the user can ask for more,
       * double buffer.
       */
      /** iterate over range, pause if not enough items **/
      auto * const buff_ptr( (this)->datamanager.get() );
      const auto cpl( Pointer::val( buff_ptr->read_pt ) );
      curr_pointer_loc = cpl;
      *sig =  reinterpret_cast< void* >(  &buff_ptr->signal[ cpl ] );
      *ptr =  buff_ptr->store;
      return;
   }

};

#endif /* END RAFTRINGBUFFERSHM_TCC */
