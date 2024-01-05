/**
 * fifo.hpp - 
 * @author: Jonathan Beard
 * @version: Thu Sep  4 12:59:45 2014
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
#ifndef RAFTFIFO_HPP
#define RAFTFIFO_HPP  1
#include <cstddef>
#include <typeinfo>
#include <iterator>
#include <list>
#include <vector>
#include <sstream>
#include <map>
#include <utility>
#include <functional>
#include <cassert>
#include <type_traits>
#include <stdexcept>
#include "bufferdata.tcc"
#include "blocked.hpp"
#include "signalvars.hpp"
#include "alloc_traits.tcc"


#include "defs.hpp"

/** pre-declare Schedule class **/
class Schedule;
class Allocate;

namespace raft
{
   class kernel;
}

class FIFO
{
#include "autorelease.tcc"
public:

   
   /**
    * FIFO - default constructor for base class for 
    * all subsequent ringbuffers.
    */
   FIFO() = default;
   
   /**
    * ~FIFO - default destructor
    */
   virtual ~FIFO() = default;

   /**
    * size - returns the current size of this FIFO
    * @return  std::size_t
    */
   virtual std::size_t size() = 0;

   /**
    * space_avail - convenience function to get the current
    * space available in the FIFO, could otherwise be calculated
    * by taking the capacity() - size().
    * @return std::size_t
    */
   virtual std::size_t space_avail() = 0;

   /**
    * capacity - returns the set maximum capacity of the 
    * FIFO.
    * @return std::size_t
    */
   virtual std::size_t capacity() = 0;

   /**
    * allocate - returns a reference to a writeable 
    * member at the tail of the FIFO.  You must have 
    * a subsequent call to send in order to release
    * this object to the FIFO once it is written.
    * If the user needs to de-allocate the memory without
    * using it, they can call the deallocate function.
    * @return  T&
    */
   template < class T,
              typename std::enable_if< 
                  inline_nonclass_alloc< T >::value >::type* = nullptr > 
   T& allocate()
   {
      void *ptr( nullptr );
      /** call blocks till an element is available **/
      local_allocate( &ptr );
      return( *( reinterpret_cast< T* >( ptr ) ) );
   }


   template < class T,
              class ... Args,
              typename std::enable_if< 
                inline_class_alloc< T >::value >::type* = nullptr > 
   T& 
   allocate( Args&&... params )
   {
      void *ptr( nullptr );
      /** call blocks till an element is available **/
      local_allocate( &ptr );
      T * temp( new (ptr) T( std::forward< Args >( params )... ) );
      UNUSED( temp );
      return( *( reinterpret_cast< T* >( ptr ) ) );
   }
   
   template < class T,
              class ... Args,
              typename std::enable_if< ext_alloc< T >::value >::type* = nullptr > 
   T& 
   allocate( Args&&... params )
   {
      T **ptr( nullptr );
      /** call blocks till an element is available **/
      local_allocate( (void**) &ptr );
      *ptr = new T( std::forward< Args >( params )... );
      return( **ptr );
   }

   /**
    * deallocate - call after allocate if memory allocated
    * is not needed.  Will deallocate all memory allocated
    * by a previous allocate call.
    */
   virtual void deallocate() = 0;
   
   /**
    * allocate_s - "auto-release" version of allocate,
    * where the action of pushing the memory allocated
    * to the consumer is handled by the returned object
    * exiting the calling stack frame. There are two functions
    * here, one that uses the objec constructor type. This
    * one is for plain old data types.
    * @return autorelease< T, allocatetype >
    */
   template < class T,
              typename std::enable_if< 
                inline_nonclass_alloc< T >::value >::type* = nullptr > 
   auto allocate_s() -> autorelease< T, allocatetype >
   {
      void *ptr( nullptr );
      local_allocate( &ptr );
      return( autorelease< T, allocatetype >( 
         reinterpret_cast< T* >( ptr ), (*this) ) );
   }
   
   /**
    * allocate_s - "auto-release" version of allocate,
    * where the action of pushing the memory allocated
    * to the consumer is handled by the returned object
    * exiting the calling stack frame. There are two functions
    * here, one that uses the objec constructor type. This
    * one is for object types for inline. The next is for 
    * external objects, all of which can have the constructor
    * called with zero arguments.
    * @return autorelease< T, allocatetype >
    */
   template < class T,
              class ... Args,
              typename std::enable_if< 
                inline_class_alloc< T >::value >::type* = nullptr > 
   auto allocate_s( Args&&... params ) -> autorelease< T, allocatetype >
   {
      void *ptr( nullptr );
      /** call blocks till an element is available **/
      local_allocate( &ptr );
      new (ptr) T( std::forward< Args >( params )... );
      return( autorelease< T, allocatetype >( 
         reinterpret_cast< T* >( ptr ), (*this) ) );
   }
   
   
   template < class T,
              class ... Args,
              typename std::enable_if< ext_alloc< T >::value >::type* = nullptr > 
   auto allocate_s( Args&&... params ) -> autorelease< T, allocatetype >
   {
      T **ptr( nullptr );
      /** call blocks till an element is available **/
      local_allocate( (void**) &ptr );
      *ptr = new T( std::forward< Args >( params )... );
      return( autorelease< T, allocatetype >( 
         reinterpret_cast< T* >( *ptr ), (*this) ) );
   }



   /** 
    * TODO, fix allocate_range to double buffer properly
    * if not enough mem available 
    */

   /**
    * allocate_range - returns a std::vector of references to 
    * n items on the queue.  If for some reason the capacity
    * of the queue is less than n or some other unspecified
    * error occurs then the number allocated is returned in 
    * n_ret. To release items to the queue, use push_range
    * as opposed to the standard push.
    * @param   n - const std::size_t, # items to allocate
    * @return  std::vector< std::reference_wrapper< T > >
    */
   template < class T > 
      auto allocate_range( const std::size_t n ) -> 
         std::vector< std::reference_wrapper< T > >
   {
      std::vector< std::reference_wrapper< T > > output;
      void *ptr( (void*) &output );
      local_allocate_n( ptr, n );
      /** compiler should optimize this copy, if not then it'll be a copy of referneces not full objects **/
      return( std::forward<
         std::vector< 
            std::reference_wrapper< T > > >( output ) );
   }

   //FIXME, implement allocate_range for object types
   /**
    * send - releases the last item allocated by allocate() to the 
    * queue.  Function will simply return if allocate wasn't
    * called prior to calling this function.
    * @param   signal - const raft::signal, default: NONE
    */
   virtual void send( const raft::signal = raft::none ) = 0;

   /** 
    * send_range - releases the items allocated by allocate_range
    * to the queue.  Function will simply return if allocate wasn't
    * called prior to calling this function.
    * @param   signal - const raft::signal, default: NONE
    */
    virtual void send_range( const raft::signal = raft::none ) = 0;

   /**
    * push - function which takes an object of type T and a 
    * signal, makes a copy of the object using the copy 
    * constructor and passes it to the FIFO along with the
    * signal which is guaranteed to be delivered at the 
    * same time as the object (if of course the receiving 
    * object is responding to signals).
    * @param   item -  T&
    * @param   signal -  raft::signal, default raft::none
    */
   template < class T >
   void push( const T &item, const raft::signal signal = raft::none )
   {
      void * const ptr( (void*) &item );
      /** call blocks till element is written and released to queue **/
      local_push( ptr, signal );
      return;
   }
   
   /**
    * push - function which takes an object of type T and a 
    * signal, makes a copy of the object using the copy 
    * constructor and passes it to the FIFO along with the
    * signal which is guaranteed to be delivered at the 
    * same time as the object (if of course the receiving 
    * object is responding to signals).
    * @param   item -  T&&
    * @param   signal -  raft::signal, default raft::none
    */
   template < class T >
   void push( const T &&item, const raft::signal signal = raft::none )
   {
      void * const ptr( (void*) &item );
      /** call blocks till element is written and released to queue **/
      local_push( ptr, signal );
      return;
   }


   /**
    * insert - inserts the range from begin to end in the FIFO,
    * blocks until space is available.  If the range is greater
    * than the space available it'll simply block and add items
    * as space becomes available.  There is the implicit assumption
    * that another thread is consuming the data, so eventually there 
    * will be room.
    * @param   begin - iterator_type, iterator to begin of range
    * @param   end   - iterator_type, iterator to end of range
    * @param   signal - raft::signal, default raft::none
    */
   template< class iterator_type >
   void insert(   iterator_type begin,
                  iterator_type end,
                  const raft::signal signal = raft::none )
   {
      void *begin_ptr( reinterpret_cast< void* >( &begin ) );
      void *end_ptr  ( reinterpret_cast< void* >( &end   ) );
      local_insert( begin_ptr, 
                    end_ptr, 
                    signal, 
                    typeid( iterator_type ).hash_code() );
      return;
   }
   
   /**
    * pop - pops the head of the queue.  If the receiving
    * object wants to watch use the signal, then the signal
    * parameter should not be null.
    * @param   item - T&
    * @param   signal - raft::signal
    */
   template< class T >
   void pop( T &item, raft::signal *signal = nullptr )
   {
      void *ptr( reinterpret_cast< void* >( &item ) );
      local_pop( ptr, signal );
      return;
   }
   
   template< class T >
   auto pop_s() -> autorelease< T, poptype >
   {
      return( autorelease< T, poptype >( (*this) ) );
   }
   
   /**
    * pop_range - pops n_items from the buffer into the 
    * std::vector pointed to by pop_range.  There are 
    * two different ways this function could operate,
    * either with a push_back type symantic which would 
    * mean three copies or dealing with a pre-allocated
    * vector.  This function assumes that the user has
    * allocated a vector withthe correct size (= n_items).
    * @param   items    - std::vector< std::pair< T, raft::signal > >& 
    * @param   n_items  - std::size_t
    */
   template< class T >
   void pop_range( pop_range_t< T >  &items,
                   const std::size_t n_items )
   {
      void *ptr_items( (void*)&items );
      local_pop_range( ptr_items, n_items );
      return;
   }

   /**
    * peek - returns a reference to the head of the
    * queue.  unpeek() must be called after this to 
    * tell the runtime that the reference is no longer
    * being used.  
    * @param   signal - raft::signal, default: nullptr
    * @return T&
    */
   template< class T,
             typename std::enable_if< inline_alloc< T >::value >::type* = nullptr >
   T& peek( raft::signal *signal = nullptr )
   {
      void *ptr( nullptr );
      local_peek( &ptr, signal );
      return( *( reinterpret_cast< T* >( ptr ) ) );
   }
   
   template< class T,
             typename std::enable_if< ext_alloc< T >::value >::type* = nullptr >
   T& peek( raft::signal *signal = nullptr )
   {
      T **ptr( nullptr );
      local_peek( (void**)&ptr, signal );
      return( **ptr );
   }

   /**
    * peek_range - analogous to peek, only the user gets
    * a list of items.  unpeek() must be called after
    * using this function to let the runtime know that
    * the user is done with the references.
    * @ n - const std::size_t, number of items to peek
    * @return - std::vector< std::reference_wrapper< T > >
    */
   template< class T,
             typename std::enable_if< inline_alloc< T >::value >::type* = nullptr >
   auto  peek_range( const std::size_t n ) -> 
      autorelease< T, peekrange > 
   {
      void *ptr = nullptr;
      void *sig = nullptr;
      std::size_t curr_pointer_loc( 0 );
      local_peek_range( &ptr, &sig, n, curr_pointer_loc );
      return( autorelease< T, peekrange >( 
         (*this),
         reinterpret_cast< T * const >( ptr ),
         reinterpret_cast< Buffer::Signal* >( sig ),
         curr_pointer_loc,
         n ) );
   }
   
   template< class T,
             typename std::enable_if< ext_alloc< T >::value >::type* = nullptr >
   auto  peek_range( const std::size_t n ) -> 
      autorelease< T, peekrange >
   {
      /** FIXME: still not implemented yet for externally allocated objects **/
      assert( false );
      void *ptr = nullptr;
      void *sig = nullptr;
      std::size_t curr_pointer_loc( 0 );
      local_peek_range( &ptr, &sig, n, curr_pointer_loc );
      return( autorelease< T, peekrange >( 
         (*this),
         reinterpret_cast< T * const >( ptr ),
         reinterpret_cast< Buffer::Signal* >( sig ),
         curr_pointer_loc,
         n ) );
   }


   /**
    * unpeek - call after peek to let the runtime know that 
    * all references to the returned value are no longer in
    * use. Keeps the memory location in a valid state for that
    * input port, i.e., it doesn't release the memory location
    * so that the next call to peek will return the same 
    * exact location. A call to recycle will release the memory,
    * or invalidate it.
    */
   virtual void unpeek() = 0;

   /** 
    * recycle - so you want to ignore some items from the
    * input stream without ever even looking at them, then
    * this is the function for you. It is also used with the
    * peek call in order to invalidate or free memory to the
    * queue so that the next peek or pop operation will see a
    * a different location. 
    * @param   t - refernce to object or type of object to be 
    * recycled, can be null or invalid since this function will
    * never access it.
    * @param   range - const std::size_t
    */
   void recycle( const std::size_t range = 1 )
   {
      local_recycle( range ); 
   }
   

   
   
   /**
    * get_zero_read_stats - sets the param variable to the 
    * current blocked stats and then sets the current vars 
    * to zero.  Default version here does nothing, when
    * instrumentation is enabled then the function called
    * actually does something.
    * @param   copy - Blocked&
    */
   virtual void get_zero_read_stats( Blocked &copy );

   /**
    * get_zero_write_stats - sets the param variable to
    * the current blocked stats and then sets the current
    * vars to zero.  Default version here does nothing, 
    * when instrumentation is enabled then the function,
    * called actually does something.
    * @param   copy - Blocked&
    */
   virtual void get_zero_write_stats( Blocked &copy );

   /**
    * resize - called from the dynamic allocator  to 
    * resize the queue.  The function itself is 
    * implemented in the various template specializations
    * found in ringuffer.tcc.  A new queue is allocated
    * with the size specified and alignment and the old 
    * queue is copied over.  The third parameter, exit_alloc
    * must be passed from the dynamic allocator to signal
    * with the application is finished so that the resize
    * function doesn't wait indefinitely for queue conditions
    * that will never arise (for exact conditions, see 
    * datamanager.tcc.
    * @param   n_items - number of items to resize q to
    * @param   align   - alignment of queue to allocate
    * @param   exit_alloc - bool to signal when app is finished
    */
   virtual void resize( const std::size_t n_items, 
                        const std::size_t align, 
                        volatile bool &exit_alloc ) = 0;

   /**
    * get_frac_write_blocked - returns the fraction
    * of time that this queue was blocked.  This might
    * become a private function accessible only to the
    * dynamic allocator mechanism, but for now its a 
    * public function.
    * @return float
    */
   virtual float get_frac_write_blocked() = 0;
    
    /**
     * get_suggested_count - returns the suggested count
     * for the dynamic allocator if the user has ever
     * asked for more than is currently available 
     * space in the FIFO. This could also serve as a 
     * minimum size for future allocations (implementation
     * dependent.
     * @return  std::size_t suggested count
     */
     virtual std::size_t get_suggested_count() = 0;

   /**
    * invalidate - used by producer thread to label this
    * queue as invalid.  Could be for many differing reasons,
    * however the bottom line is that once empty, this queue
    * will receive no extra data and the receiver must
    * do something to deal with this type of behavior
    * if more data is requested.
    */
   virtual void invalidate() = 0;

   virtual void revalidate() = 0;
   
   /**
    * is_invalid - called by the consumer thread to check 
    * if this queue is in fact valid.  This is typically 
    * only called if the queue is empty or if the consumer
    * is asking for more data than is currently available.
    * @return bool - true if invalid
    */
   virtual bool is_invalid() = 0;
protected:
   /**
    * setPtrMap - 
    */
   virtual void setPtrMap( ptr_map_t * const in );
   /**
    * setPtrSet - 
    */
   virtual void setPtrSet( ptr_set_t * const out );
   /**
    * setInPeekSet -
    */
   virtual void setInPeekSet( ptr_set_t * const peekset );
   /**
    * setOutPeekSet -
    */
   virtual void setOutPeekSet( ptr_set_t * const peekset );
   /**
    * signal_peek - special function for the scheduler
    * to peek at a signal on the head of the queue.
    * @return raft::signal
    */
   virtual raft::signal signal_peek() = 0;
   /**
    * signal_pop - special function fo rthe scheduler to 
    * pop the current signal and associated item.
    */
   virtual void signal_pop() = 0;

   /**
    * inline_signal_send - pretty much exactly like 
    * it sounds, the implementations of this function
    * must pass a signal inline with the data stream 
    * so that this signal is received immediately
    * after the data element sent before it.
    * @param sig - raft::signal, signal to be sent
    */
   virtual void inline_signal_send( const raft::signal sig ) = 0;

   /** 
    * local_allocate - in order to get this whole thing
    * to work with multiple "ports" contained within the
    * same container we have to erase the type and then 
    * put it back.  To do this the template function
    * gives a void ptr mem address which is given the 
    * location to the head of the queue.  Once returned
    * the main allocate function reinterprets this as
    * the proper object type
    * @param   ptr - void **
    */
   virtual void local_allocate( void **ptr ) = 0;
   
   /**
    * local_allocate_n - copies std::ref's to the data structure
    * passed in by ptr.  
    * @param   - ptr, void* dereferenced std::vector
    * @param   - n, const std::size_t
    */
   virtual void  local_allocate_n( void *ptr, const std::size_t n ) = 0;
   /**
    * local_push - pushes the object reference by the void
    * ptr and pushes it to the FIFO with the associated 
    * signal.  Once this function returns, the value is 
    * copied according to the objects copy constructor, 
    * which should be set up to "deep" copy the object
    * @param   ptr - void* 
    * @param   signal - raft::signal reference
    */
   virtual void local_push( void *ptr, const raft::signal &signal ) = 0;

   /**
    * local_insert - inserts a range from ptr_begin to ptr_end
    * and inserts the signal at the last element inserted, the 
    * rest of the signals are set to raft::none.
    * @param   ptr_begin - void*
    * @param   ptr_end   - void*
    * @param   signal    - const raft::signal& 
    */
   virtual void local_insert( void *ptr_begin, 
                              void *ptr_end, 
                              const raft::signal &signal,
                              const std::size_t iterator_type ) = 0;
  
   /**
    * local_pop - makes a copy, pops an item from the queue and 
    * stores the copy at memory located at *ptr.
    * @param   ptr    - void*
    * @param   signal - raft::signal* 
    */
   virtual void local_pop( void *ptr, raft::signal *signal ) = 0;

   /**
    * local_pop_range - pops a range, of n_items and stores
    * them to the array of T* items pointed to by ptr_data.
    * @param   ptr_data - void*
    * @param   n_items  - std::size_t
    */
   virtual void local_pop_range( void *ptr_data,
                                 const std::size_t n_items ) = 0;
   /**
    * local_peek - peeks at the head of the queue, the element
    * may be modified but not erased.
    * @param   ptr - void**
    * @param   signal - raft::signal*
    */
   virtual void local_peek( void **ptr,
                            raft::signal *signal ) = 0;
   /**
    * local_peek_range - peeks at head of queue with the specified
    * range, data may be modified but not erased.  Since the queue
    * might be non-contiguous then we must return the memory location
    * of each element.
    * @param   ptr - void**, pointer to pointers at which to 
    *                store the pointers to items on the queue
    * @param   sig - void**, same as above but for signal queue
    * @param   n_items - const std::size_t, number of items requested
    * @param   curr_pointer_loc - number of items able to be returned
    */
   virtual void local_peek_range( void **ptr, 
                                  void **sig,
                                  const std::size_t n_items,
                                  std::size_t &curr_pointer_loc ) = 0;
   
   /**
    * local_recycle - called by template recycle function
    * after calling destructor (for non-POD types).
    * @param range - std::size_t
    */
   virtual void local_recycle( std::size_t range ) = 0;

   
   /**
    * needed to keep as a friend for signalling access 
    */
   friend class Schedule;
   friend class Allocate;
};


#endif /* END RAFTFIFO_HPP */
