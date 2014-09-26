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
#ifndef _FIFO_HPP_
#define _FIFO_HPP_  1
#include <cstddef>
#include <typeinfo>
#include <iterator>
#include <list>
#include <vector>
#include <map>
#include <functional>

#include "blocked.hpp"
#include "signalvars.hpp"


class FIFO
{
public:
   /**
    * FIFO - default constructor for base class for 
    * all subsequent ringbuffers.
    */
   FIFO();
   
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
    * get_signal - used to get asynchronous signals
    * from the queue, the depth of each signalling
    * queue is implementation specific but must be 
    * at least (1).
    * @return  raft::signal, default is NONE
    */
   virtual raft::signal get_signal() = 0;

   /**
    * send_signal - used to send asynchronous signals
    * from the queue, the depth of each signalling
    * queue is implementation specific but must be
    * at least (1).
    * @param   signal - const raft::signal reference
    * @return  bool   - true if signal was sent
    */
   virtual bool send_signal( const raft::signal &signal ) = 0;

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
   virtual std::size_t capacity() const  = 0;

   /**
    * allocate - returns a reference to a writeable 
    * member at the tail of the FIFO.  You must have 
    * a subsequent call to push in order to release
    * this object to the FIFO once it is written.
    * @return  T&
    */
   template < class T > T& allocate()
   {
      void *ptr( nullptr );
      /** call blocks till an element is available **/
      local_allocate( &ptr );
      return( *( reinterpret_cast< T* >( ptr ) ) );
   }

   /**
    * releases the last item allocated by allocate() to the 
    * queue.  Function will simply return if allocate wasn't
    * called prior to calling this function.
    * @param   signal - const RBSignal signal, default: NONE
    */
   virtual void push( const RBSignal signal = RBSignal::NONE ) = 0;


   /**
    * push - function which takes an object of type T and a 
    * signal, makes a copy of the object using the copy 
    * constructor and passes it to the FIFO along with the
    * signal which is guaranteed to be delivered at the 
    * same time as the object (if of course the receiving 
    * object is responding to signals).
    * @param   item -  T&
    * @param   signal -  RBSignal, default RBSignal::NONE
    */
   template < class T > 
   void push( T &item, const RBSignal signal = RBSignal::NONE )
   {
      void *ptr( (void*) &item );
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
    * @param   signal - RBSignal, default RBSignal::NONE
    */
   template< class iterator_type >
   void insert(   iterator_type begin,
                  iterator_type end,
                  const RBSignal signal = RBSignal::NONE )
   {
      void *begin_ptr( (void*)&begin );
      void *end_ptr  ( (void*)&end   );
      local_insert( begin_ptr, end_ptr, signal, typeid( iterator_type ).hash_code() );
      return;
   }
   
   /**
    * pop - pops the head of the queue.  If the receiving
    * object wants to watch use the signal, then the signal
    * parameter should not be null.
    * @param   item - T&
    * @param   signal - RBSignal
    */
   template< class T >
   void pop( T &item, RBSignal *signal = nullptr )
   {
      void *ptr( (void*)&item );
      local_pop( ptr, signal );
      return;
   }
  
   /**
    * pop_range - pops a range and stores it into the array pointed to 
    * by items.  The exact range is specified by n_items.  If signalling 
    * is desired then the signal array should be set to an array of
    * where the size matches n_items.
    * @param   items - T* 
    * @param   n_items - std::size_t
    * @param   signal  - RBSignal*, default = nullptr
    */
   template< class T >
   void pop_range( T *items,
                   std::size_t n_items,
                   RBSignal   *signal = nullptr )
   {
      void *ptr_items( (void*)items );
      local_pop_range( ptr_items, signal, n_items );
      return;
   }

   template< class T >
   T& peek( RBSignal *signal = nullptr )
   {
      void *ptr( nullptr );
      local_peek( &ptr, signal );
      return;
   }


   /** 
    * recycle - if using peek() to view an element and you wish
    * to discard it then this is your function.  Takes a parameter
    * for future expansion where we allow more than one item to be
    * peeked.
    * @param   range - const std::size_t
    */
   virtual void recycle( const std::size_t range = 1 ) = 0;

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
    * get_write_finished - the function param is set to true
    * if the server process has completed.  Currently this
    * is hacked to return true when an RBSignal::EOF signal
    * is passed through, future versions will be a bit more
    * precise and not signal dependant.
    * @param   write_finished - bool&
    */
   virtual void get_write_finished( bool &write_finished ) = 0;

protected:
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
    * local_push - pushes the object reference by the void
    * ptr and pushes it to the FIFO with the associated 
    * signal.  Once this function returns, the value is 
    * copied according to the objects copy constructor, 
    * which should be set up to "deep" copy the object
    * @param   ptr - void* 
    * @param   signal - RBSignal reference
    */
   virtual void local_push( void *ptr, const RBSignal &signal ) = 0;

   /**
    * local_insert - inserts a range from ptr_begin to ptr_end
    * and inserts the signal at the last element inserted, the 
    * rest of the signals are set to RBSignal::NONE.
    * @param   ptr_begin - void*
    * @param   ptr_end   - void*
    * @param   signal    - const RBSignal&
    */
   virtual void local_insert( void *ptr_begin, 
                              void *ptr_end, 
                              const RBSignal &signal,
                              const std::size_t iterator_type ) = 0;
  
   /**
    * local_pop - makes a copy, pops an item from the queue and 
    * stores the copy at memory located at *ptr.
    * @param   ptr    - void*
    * @param   signal - RBSignal*
    */
   virtual void local_pop( void *ptr, RBSignal *signal ) = 0;

   /**
    * local_pop_range - pops a range, of n_items and stores
    * them to the array of T* items pointed to by ptr_data.
    * @param   ptr_data - void*
    * @param   signal   - RBSignal*
    * @param   n_items  - std::size_t
    */
   virtual void local_pop_range( void *ptr_data,
                                 RBSignal *signal,
                                 std::size_t n_items ) = 0;
   /**
    * local_peek - peeks at the head of the queue, the element
    * may be modified but not erased.
    * @param   ptr - void**
    * @param   signal - RBSignal*
    */
   virtual void local_peek( void **ptr,
                            RBSignal *signal ) = 0;
};
#endif /* END _FIFO_HPP_ */
