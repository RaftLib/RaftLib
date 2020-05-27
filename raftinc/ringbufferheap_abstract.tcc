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
#ifndef RAFTRINGBUFFERHEAP_ABSTRACT_TCC
#define RAFTRINGBUFFERHEAP_ABSTRACT_TCC  1

#include "portexception.hpp"
#include "defs.hpp"
#include "sysschedutil.hpp"

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
         (this)->datamanager.enterBuffer( dm::size );
         if( (this)->datamanager.notResizing() )
         {
            auto * const buff_ptr( (this)->datamanager.get() );
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
                  (this)->datamanager.exitBuffer( dm::size );
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
                  raft::yield();
                  goto TOP;
               }
               else
               {
                  (this)->datamanager.exitBuffer( dm::size );
                  return( 0 );
               }
            }
            else if( rpt < wpt )
            {
               (this)->datamanager.exitBuffer( dm::size );
               return( wpt - rpt );
            }
            else if( rpt > wpt )
            {
               (this)->datamanager.exitBuffer( dm::size );
               return( buff_ptr->max_cap - rpt + wpt ); 
            }
            (this)->datamanager.exitBuffer( dm::size );
            return( 0 );
         }
         (this)->datamanager.exitBuffer( dm::size );
         raft::yield();
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
      auto * const ptr( (this)->datamanager.get() );
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
      auto * const ptr( (this)->datamanager.get() );
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
      return( (this)->datamanager.get()->max_cap - size() );
   }
  
   /**
    * capacity - returns the capacity of this queue which is 
    * set at compile time by the constructor.
    * @return size_t
    */
   virtual std::size_t   capacity() 
   {
      return( (this)->datamanager.get()->max_cap );
   }

   
   
   /**
    * get_zero_read_stats - sets the param variable
    * to the current blocked stats and then sets the
    * current vars to zero.
    * @param   copy - Blocked&
    */
   virtual void get_zero_read_stats( Blocked &copy )
   {
      
      auto &buff_ptr_stats( (this)->datamanager.get()->read_stats.all );
      
      copy.all          = buff_ptr_stats;;
      buff_ptr_stats    = 0;
   }

   /**
    * get_zero_write_stats - sets the write variable
    * to the current blocked stats and then sets the 
    * current vars to zero.
    * @param   copy - Blocked&
    */
   virtual void get_zero_write_stats( Blocked &copy )
   {
      auto &buff_ptr_stats( (this)->datamanager.get()->write_stats.all );
      copy.all       = buff_ptr_stats;
      buff_ptr_stats = 0;
   }
    
    virtual float get_frac_write_blocked()
    {
        auto &wr_stats( (this)->datamanager.get()->write_stats );
        const auto copy( wr_stats );
        wr_stats.all = 0;
        if( copy.bec.blocked == 0 || copy.bec.count == 0 )
        {
            return( 0.0 );
        }
        /** else **/
        return( (float) copy.bec.blocked / 
                    (float) copy.bec.count );
    }
    
    /**
     * suggested size if the user asks for more than 
     * is available. if that condition never occurs
     * then this will always return zero. if it does
     * then this value can serve as a minimum size
     */
    virtual std::size_t get_suggested_count()
    {
        return( (this)->datamanager.get()->force_resize );
    }
   

protected:
   /**
    * setPtrMap
    */
   virtual void setPtrMap( ptr_map_t * const in )
   {
       assert( in != nullptr );
       (this)->consumer_data.in = in;
   }


   /**
    * setPtrSet
    */
   virtual void setPtrSet( ptr_set_t * const out )
   {
       assert( out != nullptr );
       (this)->producer_data.out = out;
   }

   virtual void setInPeekSet( ptr_set_t * const peekset )
   {
       assert( peekset != nullptr );
       (this)->consumer_data.in_peek = peekset;
   }
   
   virtual void setOutPeekSet( ptr_set_t * const peekset )
   {
       assert( peekset != nullptr );
       (this)->producer_data.out_peek = peekset;
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
      auto * const buff_ptr( (this)->datamanager.get() );
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
};

#endif /* END RAFTRINGBUFFERHEAP_ABSTRACT_TCC */
