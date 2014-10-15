/**
 * ringbufferheap.tcc - 
 * @author: Jonathan Beard
 * @version: Sun Sep  7 07:39:56 2014
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
#ifndef _RINGBUFFERHEAP_TCC_
#define _RINGBUFFERHEAP_TCC_  1

template < class T, 
           Type::RingBufferType type > class RingBufferBase : 
            public FIFOAbstract< T, type > {
public:
   /**
    * RingBuffer - default constructor, initializes basic
    * data structures.
    */
   RingBufferBase() : FIFOAbstract< T, type >(),
                      write_finished( false )
   {
   }
   
   virtual ~RingBufferBase()
   {
   }


   /**
    * size - as you'd expect it returns the number of 
    * items currently in the queue.
    * @return size_t
    */
   virtual std::size_t   size()
   {
      const auto   wrap_write( Pointer::wrapIndicator( dm.get()->write_pt  ) ),
                   wrap_read(  Pointer::wrapIndicator( dm.get()->read_pt   ) );

      const auto   wpt( Pointer::val( dm.get()->write_pt ) ), 
                   rpt( Pointer::val( dm.get()->read_pt  ) );
      if( wpt == rpt )
      {
         if( wrap_read < wrap_write )
         {
            return( dm.get()->max_cap );
         }
         else if( wrap_read > wrap_write )
         {
            /**
             * TODO, this condition is momentary, however there
             * is a better way to fix this with atomic operations...
             * or on second thought benchmarking shows the atomic
             * operations slows the queue down drastically so, perhaps
             * this is in fact the best of all possible returns.
             */
            return( dm.get()->max_cap  );
         }
         else
         {
            return( 0 );
         }
      }
      else if( rpt < wpt )
      {
         return( wpt - rpt );
      }
      else if( rpt > wpt )
      {
         return( dm.get()->max_cap - rpt + wpt ); 
      }
      return( 0 );
   }

   
   /**
    * get_signal - returns a reference to the signal mask for this
    * queue. TODO this function won't necessarily work as advertised
    * as it needs its own FIFO to deliver signals properly.
    * @return raft::signal&
    */
   virtual raft::signal get_signal()
   {
#if 0      
      /** FIXME, reimplement this function **/
#endif
      return( raft::none );
   }
  
   virtual bool  send_signal( const raft::signal &signal )
   {
      //TODO, fixme
      return( true );
   }

   /**
    * space_avail - returns the amount of space currently
    * available in the queue.  This is the amount a user
    * can expect to write without blocking
    * @return  size_t
    */
   virtual std::size_t   space_avail()
   {
      return( dm.get()->max_cap - size() );
   }
  
   /**
    * capacity - returns the capacity of this queue which is 
    * set at compile time by the constructor.
    * @return size_t
    */
   virtual std::size_t   capacity()
   {
      return( dm.get()->max_cap );
   }

   /**
    * push - releases the last item allocated by allocate() to
    * the queue.  Function will imply return if allocate wasn't
    * called prior to calling this function.
    * @param signal - const raft::signal signal, default: NONE
    */
   virtual void push( const raft::signal signal = raft::none )
   {
      if( ! (this)->allocate_called ) return;
      /** should be the end of the write, regardless of which allocate called **/
      const size_t write_index( Pointer::val( dm.get()->write_pt ) );
      dm.get()->signal[ write_index ] = signal;
      Pointer::inc( dm.get()->write_pt );
      write_stats.count += (this)->n_allocated;
      if( signal == raft::eof )
      {
         /**
          * TODO, this is a quick hack, rework when proper signalling
          * is implemented.
          */
         (this)->write_finished = true;
      }
      (this)->allocate_called = false;
      (this)->n_allocated     = 1; /** set to one for convenience **/
   }
   
   /**
    :* recycle - To be used in conjunction with peek().  Simply
    * removes the item at the head of the queue and discards them
    * @param range - const size_t, default range is 1
    */
   virtual void recycle( const std::size_t range = 1 )
   {
      assert( range <= dm.get()->max_cap );
      Pointer::incBy( range, dm.get()->read_pt );
      read_stats.count += range;
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
    * local_allocate - get a reference to an object of type T at the 
    * end of the queue.  Should be released to the queue using
    * the push command once the calling thread is done with the 
    * memory.
    * @return T&, reference to memory location at head of queue
    */
   virtual void local_allocate( void **ptr )
   {
      while( space_avail() == 0 )
      {
#ifdef NICE      
         std::this_thread::yield();
#endif         
         if( write_stats.blocked == 0 )
         {   
            write_stats.blocked = 1;
         }
#if __x86_64
         __asm__ volatile("\
           pause"
           :
           :
           : );
#endif           
      }
      (this)->allocate_called = true;
      const size_t write_index( Pointer::val( dm.get()->write_pt ) );
      *ptr = (void*)&(dm.get()->store[ write_index ].item);
   }

   virtual std::size_t local_allocate_n( void *ptr, const std::size_t n )
   {
      auto *container( reinterpret_cast< std::vector< std::reference_wrapper< T > >* >( ptr ) );
      std::size_t output( n <= (this)->capacity() ? n : (this)->capacity() );
      /** iterate over range, pause if not enough items **/
      for( std::size_t index( 0 ); index < output; index++ )
      {
         while( space_avail() == 0 )
         {
#ifdef NICE
            std::this_thread::yield();
#endif
            if( write_stats.blocked == 0 )
            {
               write_stats.blocked = 1;
            }
#if __x86_64
         __asm__ volatile("\
           pause"
           :
           :
           : );
#endif           
         }
         const std::size_t write_index( Pointer::val( dm.get()->write_pt ) );
         container->push_back( dm.get()->store[ write_index ].item );
         dm.get()->signal[ write_index ] = raft::none;
      }
      (this)->allocate_called = true;
      (this)->n_allocated     = output;
      return( output );
   }
   
   /**
    * local_push - implements the pure virtual function from the 
    * FIFO interface.  Takes a void ptr as the object which is
    * cast into the correct form and an raft::signal signal.
    * @param   item, void ptr
    * @param   signal, const raft::signal&
    */
   virtual void  local_push( void *ptr, const raft::signal &signal )
   {
      assert( ptr != nullptr );
      while( space_avail() == 0 )
      {
#ifdef NICE      
         std::this_thread::yield();
#endif         
         if( write_stats.blocked == 0 )
         {   
            write_stats.blocked = 1;
         }
#if __x86_64
         __asm__ volatile("\
           pause"
           :
           :
           : );
#endif           
      }
      
	   const size_t write_index( Pointer::val( dm.get()->write_pt ) );
      T *item( reinterpret_cast< T* >( ptr ) );
	   dm.get()->store[ write_index ].item     = *item;
	   dm.get()->signal[ write_index ]         = signal;
	   Pointer::inc( dm.get()->write_pt );
	   write_stats.count++;
      if( signal == raft::eof )
      {
         (this)->write_finished = true;
      }
   }
  
   template < class iterator_type > void local_insert_helper( iterator_type begin, 
                                                              iterator_type end,
                                                              const raft::signal &signal )
   {
      auto dist( std::distance( begin, end ) );
      while( dist-- )
      {
         while( space_avail() == 0 )
         {
#ifdef NICE
            std::this_thread::yield();
#endif
            if( write_stats.blocked == 0 )
            {
               write_stats.blocked = 1;
            }
         }
         const size_t write_index( Pointer::val( dm.get()->write_pt ) );
         dm.get()->store[ write_index ].item = (*begin);
         
         /** add signal to last el only **/
         if( dist == 0 )
         {
            dm.get()->signal[ write_index ] = signal;
         }
         else
         {
            dm.get()->signal[ write_index ] = raft::none;
         }
         Pointer::inc( dm.get()->write_pt );
         write_stats.count++;
         ++begin;
      }
      if( signal == raft::eof )
      {
         (this)->write_finished = true;
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
   typedef typename std::list< T >::iterator   it_list;
   typedef typename std::vector< T >::iterator it_vec;
   

      
   const std::map< std::size_t, 
             std::function< void (void*,void*,const raft::signal&) > > func_map
               = {{ typeid( it_list ).hash_code(), 
                    [ & ]( void *b_ptr, void *e_ptr, const raft::signal  &sig )
                    {
                        it_list *begin( reinterpret_cast< it_list* >( b_ptr ) );
                        it_list *end  ( reinterpret_cast< it_list* >( e_ptr   ) );
                        local_insert_helper( *begin, *end, signal );
                    } },
                  { typeid( it_vec ).hash_code(),
                    [ & ]( void *b_ptr, void *e_ptr, const raft::signal  &sig )
                    {
                        it_vec *begin( reinterpret_cast< it_vec* >( b_ptr ) );
                        it_vec *end  ( reinterpret_cast< it_vec* >( e_ptr   ) );
                        local_insert_helper( *begin, *end, signal );

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
    * local_pop - read one item from the ring buffer,
    * will block till there is data to be read
    * @return  T, item read.  It is removed from the
    *          q as soon as it is read
    */
   virtual void 
   local_pop( void *ptr, raft::signal *signal )
   {
      assert( ptr != nullptr );
      while( size() == 0 )
      {
#ifdef NICE      
         std::this_thread::yield();
#endif        
         if( read_stats.blocked == 0 )
         {   
            read_stats.blocked  = 1;
         }
#if __x86_64
         __asm__ volatile("\
           pause"
           :
           :
           : );
#endif           
      }
      const std::size_t read_index( Pointer::val( dm.get()->read_pt ) );
      if( signal != nullptr )
      {
         *signal = dm.get()->signal[ read_index ];
      }
      /** gotta dereference pointer and copy **/
      T *item( reinterpret_cast< T* >( ptr ) );
      *item = dm.get()->store[ read_index ].item;
      Pointer::inc( dm.get()->read_pt );
      read_stats.count++;
   }
   
   /**
    * pop_range - pops a range and returns it as a std::array.  The
    * exact range to be popped is specified as a template parameter.
    * the static std::array was chosen as its a bit faster, however 
    * this might change in future implementations to a std::vector
    * or some other structure.
    */
   virtual void  local_pop_range( void     *ptr_data,
                                  raft::signal  *signal,
                                  std::size_t n_items )
   {
      assert( ptr_data != nullptr );
      
      if( n_items == 0 )
      {
         return;
      }

      auto *items( reinterpret_cast< T* >( ptr_data ) );
      
      while( size() < n_items )
      {
#ifdef NICE
         std::this_thread::yield();
#endif
         if( read_stats.blocked == 0 )
         {
            read_stats.blocked = 1;
         }
      }
     
      size_t read_index;
      

      if( signal != nullptr )
      {
         for( size_t i( 0 ); i < n_items ; i++ )
         {
            read_index = Pointer::val( dm.get()->read_pt );
            items[ i ] = dm.get()->store [ read_index ].item;
            signal  [ i ] = dm.get()->signal[ read_index ];
            Pointer::inc( dm.get()->read_pt );
            read_stats.count++;
         }
      }
      else /** ignore signal **/
      {
         /** TODO, incorporate streaming copy here **/
         for( size_t i( 0 ); i < n_items; i++ )
         {
            read_index = Pointer::val( dm.get()->read_pt );
            items[ i ]    = dm.get()->store[ read_index ].item;
            Pointer::inc( dm.get()->read_pt );
            read_stats.count++;
         }

      }
      return;
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
      while( size() < 1 )
      {
#ifdef NICE      
         std::this_thread::yield();
#endif     
#if  __x86_64   
         __asm__ volatile("\
           pause"
           :
           :
           : );
#endif
      }
      const size_t read_index( Pointer::val( dm.get()->read_pt ) );
      if( signal != nullptr )
      {
         *signal = dm.get()->signal[ read_index ];
      }
      *ptr = (void*) &( dm.get()->store[ read_index ].item );
      return;
   }

   /** 
    * upgraded the *data structure to be a DataManager
    * object to enable easier and more intuitive dynamic
    * lock free buffer resizing and re-alignment.
    */
   DataManager< T, type >       dm;
   /**
    * these two should go inside the buffer, they'll
    * be accessed via the monitoring system.
    */
   volatile Blocked             read_stats;
   volatile Blocked             write_stats;
   /** 
    * This should be okay outside of the buffer, its local 
    * to the writing thread.  Variable gets set "true" in
    * the allocate function and false when the push with
    * only the signal argument is called.
    */
   /** TODO, this needs to get moved into the buffer for SHM **/
   volatile bool                write_finished;
};
#endif /* END _RINGBUFFERHEAP_TCC_ */
