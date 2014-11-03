/**
 * write_each.tcc - 
 * @author: Jonathan Beard
 * @version: Sun Oct 26 15:51:46 2014
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
#ifndef _WRITEEACH_TCC_
#define _WRITEEACH_TCC_  1
#include <iterator>
#include <raft>

#include <cstddef>
#include <typeinfo>
#include <functional>

#include <list>
#include <vector>
#include <deque>
#include <forward_list>

/** 
 * TODO: add functor as an option to set signal handlers
 * so the write object can respond to data stream or 
 * asynch signals appropriately
 */
namespace raft{

template < class T > class write_each : public parallel_k
{

typedef typename std::back_insert_iterator< std::list< T > >           it_list;
typedef typename std::back_insert_iterator< std::vector< T > >         it_vect;
typedef typename std::back_insert_iterator< std::deque< T > >          it_deq;

template< class iterator_type > 
   static void inc_helper( iterator_type &insert_position, Port &port_list )
   {
      T temp;
      for( auto &port : port_list )
      {
         if( port.size() > 0 )
         {
            port.pop< T >( temp );
            (*insert_position) = temp;
            /** hope the iterator defined overloaded ++ **/
            insert_position++;
         }
      }
      return;
   }

const std::map< std::size_t,
         std::function< void ( void*, Port& ) > > func_map
            =  {
                  { 
                     typeid( it_list ).hash_code(),
                     [ ](  void *e_ptr, Port &port_list )
                     {
                        it_list *end  ( reinterpret_cast< it_list* >( e_ptr ) );
                        return( inc_helper( *end, port_list ) );
                     }
                  },
                  { 
                     typeid( it_vect ).hash_code(),
                     [ ](  void *e_ptr, Port &port_list )
                     {
                        it_vect *end  ( reinterpret_cast< it_vect* >( e_ptr ) );
                        return( inc_helper( *end, port_list ) );
                     }
                  },
                  { 
                     typeid( it_deq ).hash_code(),
                     [ ](  void *e_ptr, Port &port_list )
                     {
                        it_deq *end  ( reinterpret_cast< it_deq* >( e_ptr ) );
                        return( inc_helper( *end, port_list ) );
                     }
                  }
               };
public:
   template < class iterator_type >
      write_each( iterator_type &&insert_position, 
                  const std::size_t num_ports = 1  ) :
         position( &insert_position )
   {
      /* no output ports, writing to container */
      for( auto index( 0 ); index < num_ports; index++ )
      {
         addPortTo< T >( input ); 
      }
      /** 
       * hacky way of getting the right iterator type for the ptr
       * pehaps change if I can figure out how to do without having
       * to move the constructor template to the class template 
       * param
       */
       const auto ret_val( func_map.find( typeid( iterator_type ).hash_code() ) );
       if( ret_val != func_map.end() )
       {
          inc_func = (*ret_val).second; 
       }
       else
       {
          /** TODO, make exception for this **/
          assert( false );
       }
   }

   virtual raft::kstatus run()
   {
      inc_func( position, input );
      return( raft::proceed );
   }
protected:
   virtual void addPort()
   {
      addPortTo< T >( input );
   }

private:
   void * const position;
   std::function < void( void*, Port& ) > inc_func;

   bool readable = false;
};

} /** end namespace raft **/
#endif /* END _WRITEEACH_TCC_ */
