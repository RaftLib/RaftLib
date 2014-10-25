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
#include <cassert>

#include "blocked.hpp"
#include "signalvars.hpp"



class FIFO
{
public:

   enum autotype { allocatetype, poptype };

   template< class T, autotype type = poptype > class autorelease
   {
   public:
      autorelease( FIFO &fifo ) : fifo( fifo ),
                                     item( fifo.peek< T >( &signal ) )
      {}

      autorelease( const autorelease< T, type >  &other ) : 
         fifo( other.fifo ),
         item( other.item )
      {
         other.copied = true;
      }
      
      ~autorelease()
      {
         if( ! copied )
         {
            fifo.unpeek();
            fifo.recycle( 1 );
         }
      }

      T& operator *()
      {
         return( item );
      }

      raft::signal& sig()
      {
         return( (this)->signal );
      }
   private:
      FIFO         &fifo;
      raft::signal signal;
      T            &item;
      bool         copied = false;
   };


   template < class T > class autorelease< T, autotype::allocatetype >
   {
   public:
      
      autorelease(  T *item, FIFO &fifo ) : item( item ), fifo( fifo )
      {
      }

      autorelease( const autorelease< T, autotype::allocatetype > &other ) : 
         item( other.item ), 
         fifo( other.fifo )
      {
         other.copied = true;
      }
      
      ~autorelease()
      {
         if( ! copied )
         {
            fifo.push( signal );   
         }
      }
   
      T& operator *()
      {
         return( (*item) ); 
      }
   
      raft::signal& sig()
      {
         return( (this)->signal );
      }
   
   private:
      T                   *item;
      raft::signal         signal = raft::none;
      FIFO                 &fifo;
      bool                 copied = false;
   };

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
   virtual std::size_t capacity() = 0;

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

   template < class T > auto allocate_s() -> 
      autorelease< T, autotype::allocatetype >
   {
      void *ptr( nullptr );
      local_allocate( &ptr );
      return( autorelease< T, autotype::allocatetype >( 
         reinterpret_cast< T* >( ptr ), (*this) ) );
   }
   
   /**
    * allocate_range - returns a std::vector of references to 
    * n items on the queue.  If for some reason the capacity
    * of the queue is less than n or some other unspecified
    * error occurs then the number allocated is returned in 
    * n_ret. To release items to the queue, use push_range
    * as opposed to the standard push.
    * @param   n - const std::size_t, # items to allocate
    * @param   n_ret - std::size_t number of items actually allocated
    * @return  std::vector< std::reference_wrapper< T > >
    */
   template < class T > auto allocate_range( const std::size_t n, 
                                             std::size_t &n_ret ) -> std::vector< 
                                                std::reference_wrapper< T > >
   {
      std::vector< std::reference_wrapper< T > > output;
      void *ptr( (void*) &output );
      n_ret = local_allocate_n( ptr, n );
      /** compiler should optimize this copy, if not then it'll be a copy of referneces not full objects **/
      return( output );
   }
   /**
    * releases the last item allocated by allocate() to the 
    * queue.  Function will simply return if allocate wasn't
    * called prior to calling this function.
    * @param   signal - const raft::signal, default: NONE
    */
   virtual void push( const raft::signal = raft::none ) = 0;

   /** 
    * push_range - releases the items allocated by allocate_range
    * to the queue.  Function will simply return if allocate wasn't
    * called prior to calling this function.
    * @param   signal - const raft::signal, default: NONE
    */
    virtual void push_range( const raft::signal = raft::none ) = 0;

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
   void push( T &item, const raft::signal signal = raft::none )
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
    * @param   signal - raft::signal, default raft::none
    */
   template< class iterator_type >
   void insert(   iterator_type begin,
                  iterator_type end,
                  const raft::signal signal = raft::none )
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
    * @param   signal - raft::signal
    */
   template< class T >
   void pop( T &item, raft::signal *signal = nullptr )
   {
      void *ptr( (void*)&item );
      local_pop( ptr, signal );
      return;
   }

   template< class T >
   auto pops() -> autorelease< T, autotype::poptype >
   {
      return( autorelease< T, autotype::poptype >( (*this) ) );
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
   void pop_range( std::vector< std::pair< T , raft::signal > > &items,
                   const std::size_t n_items )
   {
      void *ptr_items( (void*)&items );
      local_pop_range( ptr_items, n_items );
      return;
   }

   template< class T >
   T& peek( raft::signal *signal = nullptr )
   {
      void *ptr( nullptr );
      local_peek( &ptr, signal );
      return( *( reinterpret_cast< T* >( ptr ) ) );
   }

   /**
    * unpeek - call after peek to let the runtime know that 
    * all references to the returned value are no longer in
    * use.  Calling has the effect of popping the value
    * from the fifo without actually using it.
    */
   virtual void unpeek() = 0;

   /** 
    * recycle - so you want to ignore some items from the
    * input stream without ever even looking at them, then
    * this is the function for you.  
    *
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

   void invalidate();
   bool is_invalid();
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
    * local_allocate_n - copies std::ref's to the data structure
    * passed in by ptr.  
    * @param   - ptr, void* dereferenced std::vector
    * @param   - n, const std::size_t
    * @return  # of items allocated, different only if capacity is less than asked
    */
   virtual std::size_t local_allocate_n( void *ptr, const std::size_t n ) = 0;
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
                                 std::size_t n_items ) = 0;
   /**
    * local_peek - peeks at the head of the queue, the element
    * may be modified but not erased.
    * @param   ptr - void**
    * @param   signal - raft::signal*
    */
   virtual void local_peek( void **ptr,
                            raft::signal *signal ) = 0;

   volatile bool valid = true;

private:
};


#endif /* END _FIFO_HPP_ */
