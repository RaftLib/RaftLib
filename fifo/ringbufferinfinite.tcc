/**
 * ringbufferinfinite.tcc - 
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
#ifndef _RINGBUFFERINFINITE_TCC_
#define _RINGBUFFERINFINITE_TCC_  1

template < class T > class RingBufferBase< T, Type::Infinite > : 
   public FIFOAbstract< T, Type::Infinite >
{
public:
   /**
    * RingBuffer - default constructor, initializes basic
    * data structures.
    */
   RingBufferBase() : FIFOAbstract< T, Type::Infinite >(),
                      data( nullptr ),
                      allocate_called( false ),
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
      return( 1 );
   }

   virtual RBSignal get_signal() 
   {
#if 0   
      /** 
       * there are two signalling paths, the one 
       * we'll give the highest priority to is the 
       * asynchronous one.
       */
      const auto signal_queue( data->store[ 0 ].signal );
      /**
       * TODO, fix this
       */
      const auto signal_local( (this)->signal_a );
      if( signal_local == RBSignal::NONE )
      {
         return( signal_queue );
      }
      /** there must be something in the local signal **/
      //(this)->signal = RBSignal::NONE;
#endif      
      return( RBSignal::NONE );
   }

   virtual bool send_signal( const RBSignal &signal )
   {
      //(this)->signal = signal;
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
      return( data->max_cap );
   }

  
   /**
    * capacity - returns the capacity of this queue which is 
    * set at compile time by the constructor.
    * @return size_t
    */
   virtual std::size_t   capacity() const
   {
      return( data->max_cap );
   }

   

   /**
    * push - releases the last item allocated by allocate() to
    * the queue.  Function will imply return if allocate wasn't
    * called prior to calling this function.
    * @param signal - const RBSignal signal, default: NONE
    */
   virtual void push( const RBSignal signal = RBSignal::NONE )
   {
      if( ! (this)->allocate_called ) return;
      data->signal[ 0 ].sig = signal;
      write_stats.count++;
      (this)->allocate_called = false;
   }

   
   /**
    * recycle - remove ``range'' items from the head of the
    * queue and discard them.  Can be used in conjunction with
    * the peek operator.
    * @param   range - const size_t, default = 1
    */
   virtual void recycle( const std::size_t range = 1 )
   {
      read_stats.count += range;
   }

   virtual void get_zero_read_stats( Blocked &copy )
   {
      copy.all       = read_stats.all;
      read_stats.all = 0;
   }

   virtual void get_zero_write_stats( Blocked &copy )
   {
      copy.all       = write_stats.all;
      write_stats.all = 0;
   }

protected:
   
   virtual void  local_allocate( void **ptr )
   {
      (this)->allocate_called = true;
      *ptr = (void*)&(data->store[ 0 ].item);
   }
   
   virtual void  local_push( void *ptr, const RBSignal signal )
   {
      T *item (reinterpret_cast< T* >( ptr ) );
      data->store [ 0 ].item  = *item;
      /** a bit awkward since it gives the same behavior as the actual queue **/
      data->signal[ 0 ].sig  = signal;
      write_stats.count++;
   }

   template< class iterator_type >
   void local_insert_helper( iterator_type begin, 
                             iterator_type end, 
                             const RBSignal signal )
   {
      while( begin != end )
      {
         data->store[ 0 ].item = (*begin);
         begin++;
         write_stats.count++;
      }
      data->signal[ 0 ].sig = signal;
      return;
   }
   

   virtual void local_insert( void *begin_ptr,
                              void *end_ptr,
                              const RBSignal &signal,
                              const std::size_t iterator_type )
   {
      typedef typename std::list< T >::iterator   it_list;
      typedef typename std::vector< T >::iterator it_vec;
         
      const std::map< std::size_t, 
                std::function< void (void*,void*,const RBSignal&) > > func_map
                  = {{ typeid( it_list ).hash_code(), 
                       [ & ]( void *b_ptr, void *e_ptr, const RBSignal &sig )
                       {
                           it_list *begin( reinterpret_cast< it_list* >( b_ptr ) );
                           it_list *end  ( reinterpret_cast< it_list* >( e_ptr   ) );
                           local_insert_helper( *begin, *end, signal );
                       } },
                     { typeid( it_vec ).hash_code(),
                       [ & ]( void *b_ptr, void *e_ptr, const RBSignal &sig )
                       {
                           it_vec *begin( reinterpret_cast< it_vec* >( b_ptr ) );
                           it_vec *end  ( reinterpret_cast< it_vec* >( e_ptr   ) );
                           local_insert_helper( *begin, *end, signal );

                       } } };
      auto f( (this)->func_map.find( iterator_type ) );
      if( f != (this)->func_map.end() )
      {
         (*f).second( begin_ptr, end_ptr, signal );
      }
      else
      {
         /** TODO, throw exception **/
      }
      return;
   }

   virtual void local_pop( void *ptr, RBSignal *signal )
   {
      T *item( reinterpret_cast< T* >( ptr ) );
      *item  = data->store[ 0 ].item;
      if( signal != nullptr )
      {
         *signal = data->signal[ 0 ].sig;
      }
      read_stats.count++;
   }
  
   virtual void local_pop_range( void *ptr_data,
                                 RBSignal *signal,
                                 std::size_t n_items )
   {
      assert( ptr_data != nullptr );
      auto *items( reinterpret_cast< T* >( ptr_data ) );

      if( signal != nullptr )
      {
         for( size_t i( 0 ); i < n_items; i++ )
         {
            items [ i ]  = data->store [ 0 ].item;
            signal[ i ]  = data->signal[ 0 ].sig;
         }
      }
      else
      {
         for( size_t i( 0 ); i < n_items; i++ )
         {
            items [ i ]  = data->store [ 0 ].item;
         }
      }
   }


   virtual void local_peek( void **ptr, RBSignal *signal )
   {
      *ptr = (void*)&( data->store[ 0 ].item );
      if( signal != nullptr )
      {
         *signal = data->signal[  0  ].sig;
      }
   }

   /** go ahead and allocate a buffer as a heap, doesn't really matter **/
   Buffer::Data< T, Type::Heap >      *data;
   /** note, these need to get moved into the data struct **/
   volatile Blocked                             read_stats;
   volatile Blocked                             write_stats;
   
   volatile bool                                allocate_called;
   volatile bool                                write_finished;
};
#endif /* END _RINGBUFFERINFINITE_TCC_ */
