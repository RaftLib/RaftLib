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
#ifndef RAFTRINGBUFFERINFINITE_TCC
#define RAFTRINGBUFFERINFINITE_TCC  1
#include "alloc_traits.tcc"

//TODO -> fixme

template < class T >
class RingBufferBase< T, Type::Infinite, 
           typename std::enable_if< inline_nonclass_alloc< T >::value >::type >
: public FIFOAbstract< T, Type::Infinite >
{
public:
   /**
    * RingBuffer - default constructor, initializes basic
    * data structures.
    */
   RingBufferBase() : FIFOAbstract< T, Type::Infinite >()
   {
   }
   
   virtual ~RingBufferBase() = default;


   /**
    * size - as you'd expect it returns the number of 
    * items currently in the queue.
    * @return size_t
    */
   virtual std::size_t   size()
   {
      return( 1 );
   }

   virtual raft::signal get_signal() 
   {
#if 0   
      /** FIXME, reimplement this function **/
#endif      
      return( raft::none );
   }

   virtual bool send_signal( const raft::signal   &signal )
   {
      UNUSED( signal );
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
      return( (this)->datamanager.get()->max_cap );
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
    * send - releases the last item allocated by allocate() to
    * the queue.  Function will imply return if allocate wasn't
    * called prior to calling this function.
    * @param signal - const raft::signal signal, default: raft::signal::none
    */
   virtual void send( const raft::signal signal = raft::none )
   {
      if( ! (this)->allocate_called ) return;
      (this)->datamanager.get()->signal[ 0 ].sig = signal;
      (this)->write_stats.count += (this)->n_allocated;
      (this)->allocate_called = false;
      (this)->n_allocated = 1;
   }

   /** TODO, add send_range func to match fifo.hpp **/

   /**
    * recycle - remove ``range'' items from the head of the
    * queue and discard them.  Can be used in conjunction with
    * the peek operator.
    * @param   range - const size_t, default = 1
    */
   virtual void recycle( const std::size_t range = 1 )
   {
      (this)->read_stats.count += range;
   }

   virtual void get_zero_read_stats( Blocked &copy )
   {
      copy.all       = (this)->read_stats.all;
      (this)->read_stats.all = 0;
   }

   virtual void get_zero_write_stats( Blocked &copy )
   {
      copy.all       = (this)->write_stats.all;
      (this)->write_stats.all = 0;
   }

protected:
   
   virtual void  local_allocate( void **ptr )
   {
      (this)->allocate_called = true;
      *ptr = (void*)&((this)->datamanager.get()->store[ 0 ]);
   }

   virtual std::size_t local_allocate_n( void *ptr, const std::size_t n )
   {
      std::size_t output( n <= (this)->capacity() ? n : (this)->capacity() );
      auto *container( reinterpret_cast< std::vector< std::reference_wrapper< T > >* >( ptr ) );
      for( std::size_t index( 0 ); index < output; index++ )
      {
         container->push_back( (this)->datamanager.get()->store[ 0 ] );
      }
      (this)->allocate_called = true;
      (this)->n_allocated     = output;
      return( output );
   }
   
   virtual void  local_push( void *ptr, const raft::signal &signal )
   {
      T *item (reinterpret_cast< T* >( ptr ) );
      (this)->datamanager.get()->store [ 0 ]  = *item;
      /** a bit awkward since it gives the same behavior as the actual queue **/
      (this)->datamanager.get()->signal[ 0 ].sig  = signal;
      (this)->write_stats.count++;
   }

   template< class iterator_type >
   void local_insert_helper( iterator_type begin, 
                             iterator_type end, 
                             const raft::signal signal )
   {
      while( begin != end )
      {
         (this)->datamanager.get()->store[ 0 ] = (*begin);
         begin++;
         (this)->write_stats.count++;
      }
      (this)->datamanager.get()->signal[ 0 ].sig = signal;
      return;
   }
   

   virtual void local_insert( void *begin_ptr,
                              void *end_ptr,
                              const raft::signal  &signal,
                              const std::size_t iterator_type )
   {
      typedef typename std::list< T >::iterator   it_list;
      typedef typename std::vector< T >::iterator it_vec;
         
      const std::map< std::size_t, 
                std::function< void (void*,void*,const raft::signal&) > > func_map
                  = {{ typeid( it_list ).hash_code(), 
                       [ & ]( void *b_ptr, void *e_ptr, const raft::signal &sig )
                       {
                           it_list *begin( reinterpret_cast< it_list* >( b_ptr ) );
                           it_list *end  ( reinterpret_cast< it_list* >( e_ptr   ) );
                           local_insert_helper( *begin, *end, sig );
                       } },
                     { typeid( it_vec ).hash_code(),
                       [ & ]( void *b_ptr, void *e_ptr, const raft::signal &sig )
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
      }
      return;
   }

   virtual void local_pop( void *ptr, raft::signal *signal )
   {
      T *item( reinterpret_cast< T* >( ptr ) );
      *item  = (this)->datamanager.get()->store[ 0 ];
      if( signal != nullptr )
      {
         *signal = (this)->datamanager.get()->signal[ 0 ].sig;
      }
      (this)->read_stats.count++;
   }
  
   virtual void local_pop_range( void *ptr_data,
                                 const std::size_t n_items )
   {
      UNUSED( ptr_data );
      UNUSED( n_items );
      //assert( ptr_data != nullptr );
      //auto *items( reinterpret_cast< T* >( ptr_data ) );

      //if( signal != nullptr )
      //{
      //   for( size_t i( 0 ); i < n_items; i++ )
      //   {
      //      items [ i ]  = (this)->datamanager.get()->store [ 0 ];
      //   }
      //}
      //else
      //{
      //   for( size_t i( 0 ); i < n_items; i++ )
      //   {
      //      items [ i ]  = (this)->datamanager.get()->store [ 0 ];
      //   }
      //}
   }


   virtual void local_peek( void **ptr, raft::signal  *signal )
   {
      *ptr = (void*)&( (this)->datamanager.get()->store[ 0 ] );
      if( signal != nullptr )
      {
         *signal = (this)->datamanager.get()->signal[  0  ].sig;
      }
   }
};
#endif /* END RAFTRINGBUFFERINFINITE_TCC */
