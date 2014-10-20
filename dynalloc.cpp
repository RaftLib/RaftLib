/**
 * dynalloc.cpp - 
 * @author: Jonathan Beard
 * @version: Mon Oct 13 16:36:18 2014
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
#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>
#include <map>
#include <cassert>

#include "graphtools.hpp"
#include "dynalloc.hpp"

dynalloc::dynalloc( Map &map, volatile bool &exit_alloc ) : 
   Allocate( map, exit_alloc )
{
}


dynalloc::~dynalloc()
{
}

std::size_t
dynalloc::hash( PortInfo &a, PortInfo &b )
{
   union{
      std::size_t      all;
      struct{
         std::uint32_t a;
         std::uint32_t b;
      };
   } u;
   const auto ta( (std::uint64_t)(& a ) );
   const auto tb( (std::uint64_t)(& b ) );
   u.a = ta & 0xffff;
   u.b = tb & 0xffff;
   return( u.all );
}

void
dynalloc::run()
{
   auto alloc_func = [&]( PortInfo &a, PortInfo &b )
   {
      assert( a.type == b.type );
      /** assume everyone needs a heap for the moment to get working **/
      instr_map_t *func_map( a.const_map[ Type::Heap ] );
      auto test_func( (*func_map)[ false ] );
      FIFO *fifo( test_func( 2  /* items */, 
                             16 /* align */, 
                             (void*)NULL ) );
      assert( fifo != nullptr );
      (this)->initialize( &a, &b, fifo );
   };
   GraphTools::BFS( (this)->source_kernels,
                    alloc_func );
   
   (this)->setReady();
   std::map< std::size_t, int > size_map;
   
   /** 
    * make this a fixed quantity right now, if size > .75% at
    * montor interval three times or more then increase size.
    */
   auto mon_func = [&]( PortInfo &a, PortInfo &b ) -> void
   {
      const float ratio( .5 );
      const auto hash_val( dynalloc::hash( a, b ) );
      /** TODO, the values might wrap if no monitoring on **/
      const auto realized_ratio( a.getFIFO()->get_frac_write_blocked() );
      if( realized_ratio >= ratio )
      {
         const auto curr_count( size_map[ hash_val ]++ );
         if( curr_count  == 2 )
         {
#ifdef UNITTEST
            fprintf( stderr, "RESIZECALLED\n" );
#endif 
            /** get initializer function **/
            /** TODO, add term signal here **/
            auto * const buff_ptr( a.getFIFO() );
            const auto cap( buff_ptr->capacity() );
            buff_ptr->resize( cap * 2, 16, exit_alloc );
            size_map[ hash_val ] = 0;
         }
      }
      return;
   };
   /** start monitor loop **/
   while( ! exit_alloc )
   {
      /** monitor fifo's **/
      std::chrono::microseconds dura( 1 );
      std::this_thread::sleep_for( dura );
      GraphTools::BFS( (this)->source_kernels ,
                       mon_func );
   }
   return;
}
