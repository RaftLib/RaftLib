/**
 * ringbufferheap_lessabstract.tcc - RingBufferBaseHeap contains
 * the functions that don't need specialization based on ringbuffer
 * type, this one actually contains type specific specializations. 
 *
 * @author: Jonathan Beard
 * @version: Sat Jul 25 14:43:36 2020
 * 
 * Copyright 2020 Jonathan Beard
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
#ifndef _RINGBUFFERHEAP_LESSABSTRACT_TCC_
#define _RINGBUFFERHEAP_LESSABSTRACT_TCC_  1

#include "ringbufferheap_abstract.tcc"

/**
 * This "shim" class contains function templates that could
 * at some point depend on the type and might need specializations,
 * however, at this point, it's simpler to keep them in an intermediate
 * class and have the specializations derive from it. 
 */


template < class T,  Type::RingBufferType type > 
class RingBufferBaseHeapShim : public RingBufferBaseHeap< T, type > 
{
public:
    RingBufferBaseHeapShim() : RingBufferBaseHeap<T,type>(){}

    virtual ~RingBufferBaseHeapShim() = default;

protected:
   
   template < class iterator_type > 
   void local_insert_helper( iterator_type begin, 
                             iterator_type end,
                             const raft::signal &signal )
   {
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
   

};


template < Type::RingBufferType type > 
class RingBufferBaseHeapShim < bool, type > : public RingBufferBaseHeap< bool, type > 
{
public:
    RingBufferBaseHeapShim() : RingBufferBaseHeap<bool,type>(){}

    virtual ~RingBufferBaseHeapShim() = default;

protected:
   

   template < class iterator_type > 
   void local_insert_helper( iterator_type begin, 
                             iterator_type end,
                             const raft::signal &signal )
   {
      auto dist( std::distance( begin, end ) );
      const raft::signal dummy( raft::none );
      while( dist-- )
      {
         /** add signal to last el only **/
         const bool val = (*begin);
         /** use global push function **/
         if( dist == 0 )
         {
            (this)->local_push( (void*)&val, signal );
         }
         else
         {
            (this)->local_push( (void*)&val, dummy );
         }
         ++begin;
      }
      return;
   }
};
   
template < class T, Type::RingBufferType type > 
class RingBufferBaseHeapAbstract : protected RingBufferBaseHeapShim< T, type >
{
public:
    RingBufferBaseHeapAbstract() : RingBufferBaseHeapShim< T, type >(){}

    virtual ~RingBufferBaseHeapAbstract() = default;

protected:
   
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
                        (this)->local_insert_helper( *begin, *end, sig );
                    } },
                  { typeid( it_vec ).hash_code(),
                    [&]( void *b_ptr, void *e_ptr, const raft::signal  &sig )
                    {
                        it_vec *begin( reinterpret_cast< it_vec* >( b_ptr ) );
                        it_vec *end  ( reinterpret_cast< it_vec* >( e_ptr   ) );
                        (this)->local_insert_helper( *begin, *end, sig );

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


#endif /* END _RINGBUFFERHEAP_LESSABSTRACT_TCC_ */
