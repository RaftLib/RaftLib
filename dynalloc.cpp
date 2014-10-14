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
      FIFO *fifo( test_func( 1 /* items */, 
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
   std::cerr.setf( std::ios::fixed );
   std::cerr << std::setprecision( 30 );
   auto mon_func = [&]( PortInfo &a, PortInfo &b ) -> void
   {
      const float ratio( .5 );
      const auto hash_val( dynalloc::hash( a, b ) );
      const auto cap( a.getFIFO()->capacity() );
      const auto size( a.getFIFO()->size() );
      const float realized_ratio( (float) size / (float) cap );
      std::cerr << "realized ratio: " << realized_ratio << "\n";
      if( realized_ratio >= ratio )
      {
         //size_map[ hash_val ]++;
         //if( size_map[ hash_val ] == 3 )
         //{
            std::cerr << "reallocating " << a.my_name << " -> " << a.other_name << 
               ", " << cap << " to " << ( cap * 2 ) << "\n";
            /** get initializer function **/
            instr_map_t *func_map( a.const_map[ Type::Heap ] );
            auto test_func( (*func_map)[ false ] );
            FIFO *newfifo( test_func( cap * 2 /* items */,
                                      16      /* align */,
                                      (void*)NULL ) );
            assert( newfifo != nullptr );
            ///** realloc **/
            (this)->reinitialize( &a, &b, newfifo ); 
            size_map[ hash_val ] = 0;
         //}
      }
      return;
   };
   /** start monitor loop **/
   while( ! exit_alloc )
   {
      /** monitor fifo's **/
      //std::chrono::microseconds dura( 1 );
      //std::this_thread::sleep_for( dura );
      GraphTools::BFS( (this)->source_kernels ,
                       mon_func );
      std::cerr << "end of mon loop\n";
      std::this_thread::yield();
   }
   return;
}
