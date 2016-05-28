/**
 * ringbufferheap_abstract.tcc - 
 * @author: Jonathan Beard
 * @version: Thu Feb 18 20:25:09 2016
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
#ifndef _RINGBUFFERHEAP_ABSTRACT_TCC_
#define _RINGBUFFERHEAP_ABSTRACT_TCC_  1

#include "portexception.hpp"
#include "optdef.hpp"
#include "scheduleconst.hpp"
#include "defs.hpp"

#ifndef NICE
#define NICE 1
#else
#undef NICE
#define NICE 1
#endif

#ifndef NOPREEMPT
#define NOPREEMPT
#endif


template < class T,  Type::RingBufferType type > 
class RingBufferBaseHeap : public FIFOAbstract< T, type> 
{
public:
   /**
    * RingBuffer - default constructor, initializes basic
    * data structures.
    */
   RingBufferBaseHeap() : FIFOAbstract< T, type >(){}
   
   virtual ~RingBufferBaseHeap() = default;


   /**
    * size - as you'd expect it returns the number of 
    * items currently in the queue.
    * @return size_t
    */
   virtual std::size_t   size() noexcept
   {  
      for( ;; )
      {
         datamanager.enterBuffer( dm::size );
         if( datamanager.notResizing() )
         {
            auto * const buff_ptr( datamanager.get() );
TOP:      
            const auto   wrap_write( Pointer::wrapIndicator( buff_ptr->write_pt  ) ),
                         wrap_read(  Pointer::wrapIndicator( buff_ptr->read_pt   ) );

            const auto   wpt( Pointer::val( buff_ptr->write_pt ) ), 
                         rpt( Pointer::val( buff_ptr->read_pt  ) );
            if( R_UNLIKELY (wpt == rpt) )
            {
               /** expect most of the time to be full **/
               if( R_LIKELY( wrap_read < wrap_write ) )
               {
                  datamanager.exitBuffer( dm::size );
                  return( buff_ptr->max_cap );
               }
               else if( wrap_read > wrap_write )
               {
                  /**
                   * TODO, this condition is momentary, however there
                   * is a better way to fix this with atomic operations...
                   * or on second thought benchmarking shows the atomic
                   * operations slows the queue down drastically so, perhaps
                   * this is in fact the best of all possible returns (see 
                   * Leibniz or Candide for further info).
                   */
                  std::this_thread::yield();
                  goto TOP;
               }
               else
               {
                  datamanager.exitBuffer( dm::size );
                  return( 0 );
               }
            }
            else if( rpt < wpt )
            {
               datamanager.exitBuffer( dm::size );
               return( wpt - rpt );
            }
            else if( rpt > wpt )
            {
               datamanager.exitBuffer( dm::size );
               return( buff_ptr->max_cap - rpt + wpt ); 
            }
            datamanager.exitBuffer( dm::size );
            return( 0 );
         }
         datamanager.exitBuffer( dm::size );
      } /** end for **/
      return( 0 ); /** keep some compilers happy **/
   }


   /**
    * invalidate - used by producer thread to label this
    * queue as invalid.  Could be for many differing reasons,
    * however the bottom line is that once empty, this queue
    * will receive no extra data and the receiver must
    * do something to deal with this type of behavior
    * if more data is requested.
    */
   virtual void invalidate()
   {
      auto * const ptr( datamanager.get() );
      ptr->is_valid = false;
      return;
   }
   
   /**
    * is_invalid - called by the consumer thread to check 
    * if this queue is in fact valid.  This is typically 
    * only called if the queue is empty or if the consumer
    * is asking for more data than is currently available.
    * @return bool - true if invalid
    */
   virtual bool is_invalid()
   {
      auto * const ptr( datamanager.get() );
      return( ! ptr->is_valid );
   }


   /**
    * space_avail - returns the amount of space currently
    * available in the queue.  This is the amount a user
    * can expect to write without blocking
    * @return  size_t
    */
   virtual std::size_t   space_avail()
   {
      return( datamanager.get()->max_cap - size() );
   }
  
   /**
    * capacity - returns the capacity of this queue which is 
    * set at compile time by the constructor.
    * @return size_t
    */
   virtual std::size_t   capacity() 
   {
      return( datamanager.get()->max_cap );
   }

   
   
   /**
    * get_zero_read_stats - sets the param variable
    * to the current blocked stats and then sets the
    * current vars to zero.
    * @param   copy - Blocked&
    */
   virtual void get_zero_read_stats( Blocked &copy )
   {
      copy.all       = read_stats.all;
      read_stats.all = 0;
   }

   /**
    * get_zero_write_stats - sets the write variable
    * to the current blocked stats and then sets the 
    * current vars to zero.
    * @param   copy - Blocked&
    */
   virtual void get_zero_write_stats( Blocked &copy )
   {
      copy.all       = write_stats.all;
      write_stats.all = 0;
   }

   /**
    * get_write_finished - does exactly what it says, 
    * sets the param variable to true when all writes
    * have been finished.  This particular funciton 
    * might change in the future but for the moment
    * its vital for instrumentation.
    * @param   write_finished - bool&
    */
   virtual void get_write_finished( bool &write_finished )
   {
      write_finished = (this)->write_finished;
   }
   

protected:
   /**
    * setPtrMap
    */
   virtual void setPtrMap( ptr_map_t * const in )
   {
       assert( in != nullptr );
       (this)->in = in;
   }


   /**
    * setPtrSet
    */
   virtual void setPtrSet( ptr_set_t * const out )
   {
       assert( out != nullptr );
       (this)->out = out;
   }

   virtual void setInPeekSet( ptr_set_t * const peekset )
   {
       assert( peekset != nullptr );
       (this)->in_peek = peekset;
   }
   
   virtual void setOutPeekSet( ptr_set_t * const peekset )
   {
       assert( peekset != nullptr );
       (this)->out_peek = peekset;
   }

   /**
    * set_src_kernel - sets teh protected source
    * kernel for this fifo, necessary for preemption,
    * see comments on variables below.
    * @param   k - raft::kernel*
    */
   virtual void set_src_kernel( raft::kernel * const k )
   {
      assert( k != nullptr );
      while( ! datamanager.notResizing() )
      { 
         /* spin */ 
      }
      auto * const buffer( datamanager.get() );
      buffer->setSourceKernel( k );
   }


   /**
    * set_dst_kernel - sets the protected destination
    * kernel for this fifo, necessary for preemption,
    * see comments on variables below.
    * @param   k - raft::kernel*
    */
   virtual void set_dst_kernel( raft::kernel * const k )
   {
      assert( k != nullptr );
      while( ! datamanager.notResizing() )
      {
         /* spin */
      }
      auto * const buffer( datamanager.get() );
      buffer->setDestKernel( k );
   }

   /**
    * signal_peek - return signal at head of 
    * queue and nothing else
    * @return raft::signal
    */
   virtual raft::signal signal_peek()
   {
      /** 
       * NOTE: normally I'd say we need exclusion here too,
       * however, since this is a copy and we want this to
       * be quick since it'll be used quite often in tight
       * loops I think we'll be okay with getting the current
       * pointer to the head of the queue and returning the
       * value.  Logically copying the queue shouldn't effect
       * this value since the elements all remain in their 
       * location relative to the start of the queue.
       */
      auto * const buff_ptr( datamanager.get() );
      const size_t read_index( Pointer::val( buff_ptr->read_pt ) );
      return( buff_ptr->signal[ read_index ] /* make copy */ ); 
   }
   /**
    * signal_pop - special function fo rthe scheduler to 
    * pop the current signal and associated item.
    */
   virtual void signal_pop()
   {
      (this)->local_pop( nullptr, nullptr );
   }

   virtual void inline_signal_send( const raft::signal sig )
   {
      (this)->local_push( nullptr, sig ); 
   }


  
   template < class iterator_type > 
   void local_insert_helper( iterator_type begin, 
                             iterator_type end,
                             const raft::signal &signal )
   {
      /**
       * TODO, not happy with the performance of the current 
       * solution.  This could easily be much faster with streaming
       * copies.
       */
      auto dist( std::distance( begin, end ) );
      const raft::signal dummy( raft::none );
      while( dist-- )
      {
         /** use global push function **/
         if( dist == 0 )
         {
            /** add signal to last el only **/
            (this)->local_push( (void*) &(*begin), signal );
         }
         else
         {
            (this)->local_push( (void*)&(*begin), dummy );
         }
         ++begin;
      }
      return;
   }
   

   /**
    * insert - inserts the range from begin to end in the queue,
    * blocks until space is available.  If the range is greater than
    * available space on the queue then it'll simply add items as 
    * space becomes available.  There is the implicit assumption that
    * another thread is consuming the data, so eventually there will
    * be room.
    * @param   begin - iterator_type, iterator to begin of range
    * @param   end   - iterator_type, iterator to end of range
    */
   virtual void local_insert(  void *begin_ptr,
                               void *end_ptr,
                               const raft::signal &signal, 
                               const std::size_t iterator_type )
   {
   using it_list = typename std::list< T >::iterator;
   using it_vec  = typename std::vector< T >::iterator; 

   /**
    * FIXME: I suspect this would be faster if the compile time (this)
    * were in fact a pass by reference and the local_insert_helper could be 
    * absorbed into the std::function, I think the capture probably
    * has about as bad of perf as std::bind
    */
   const std::map< std::size_t, 
             std::function< void (void*,void*,const raft::signal&) > > func_map
               = {{ typeid( it_list ).hash_code(), 
                    [&]( void *b_ptr, void *e_ptr, const raft::signal  &sig )
                    {
                        it_list *begin( reinterpret_cast< it_list* >( b_ptr ) );
                        it_list *end  ( reinterpret_cast< it_list* >( e_ptr   ) );
                        local_insert_helper( *begin, *end, sig );
                    } },
                  { typeid( it_vec ).hash_code(),
                    [&]( void *b_ptr, void *e_ptr, const raft::signal  &sig )
                    {
                        it_vec *begin( reinterpret_cast< it_vec* >( b_ptr ) );
                        it_vec *end  ( reinterpret_cast< it_vec* >( e_ptr   ) );
                        local_insert_helper( *begin, *end, sig );

                    } } };
      auto f( func_map.find( iterator_type ) );
      if( f != func_map.end() )
      {
         (*f).second( begin_ptr, end_ptr, signal );
      }
      else
      {
         /** TODO, throw exception **/
         assert( false );
      }
      return;
   }
   
   
   /**
    * pop_range - pops a range and returns it as a std::array.  The
    * exact range to be popped is specified as a template parameter.
    * the static std::array was chosen as its a bit faster, however 
    * this might change in future implementations to a std::vector
    * or some other structure.
    */
   virtual void  local_pop_range( void     *ptr_data,
                                  const std::size_t n_items )
   {
      assert( ptr_data != nullptr );
      if( n_items == 0 )
      {
         return;
      }
      auto *items( 
         reinterpret_cast< 
            std::vector< std::pair< T, raft::signal > >* >( ptr_data ) );
      /** just in case **/
      assert( items->size() == n_items );
      /**
       * TODO: same as with the other range function
       * I'm not too  happy with the performance on this
       * one.  It'd be relatively easy to fix with a little
       * time.
       */
      for( auto &pair : (*items))
      {
         (this)->pop( pair.first, &(pair.second) );
      }
      return;
   }
   
   
   /** 
    * upgraded the *data structure to be a DataManager
    * object to enable easier and more intuitive dynamic
    * lock free buffer resizing and re-alignment.
    */
   DataManager< T, type >       datamanager;
   
   
   
   /**
    * these two should go inside the buffer, they'll
    * be accessed via the monitoring system.
    */
   Blocked                     read_stats;
   Blocked                     write_stats;
   /** 
    * This should be okay outside of the buffer, its local 
    * to the writing thread.  Variable gets set "true" in
    * the allocate function and false when the push with
    * only the signal argument is called.
    */
   /** TODO, this needs to get moved into the buffer for SHM **/
   volatile bool                write_finished = false;
   ptr_map_t                   *in = nullptr;
   ptr_set_t                   *out = nullptr;
   /** these are named with reference to the kernel, in == kernel in **/
   ptr_set_t                   *in_peek  = nullptr;
   ptr_set_t                   *out_peek = nullptr;
   
};

#endif /* END _RINGBUFFERHEAP_ABSTRACT_TCC_ */
