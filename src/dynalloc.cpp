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

#ifndef INITIAL_ALLOC_SIZE
#warn   "Initial alloc size must be defined in allocate.hpp"
#endif


dynalloc::dynalloc( raft::map &map,
                    volatile bool &exit_alloc ) :
                        Allocate( map, exit_alloc )
{
}


dynalloc::~dynalloc() = default;

std::size_t
dynalloc::hash( PortInfo &a, PortInfo &b )
{
   union{
      std::size_t      all;
      struct a_and_b{
         std::uint32_t a;
         std::uint32_t b;
      } ab;
   } u;
   const auto ta( (std::uint64_t)(& a ) );
   const auto tb( (std::uint64_t)(& b ) );
   u.ab.a = ta & 0xffff;
   u.ab.b = tb & 0xffff;
   return( u.all );
}


void
dynalloc::run()
{
   auto alloc_func = [&]( PortInfo &a, PortInfo &b, void *data )
   {
      /** same alloc for all, inherit from base alloc **/
      (this)->allocate( a, b, data );
   };

   /** acquire source kernels **/
   auto &container( (this)->source_kernels.acquire() );
   GraphTools::BFT( container, alloc_func, GraphTools::output );
   (this)->source_kernels.release();
   (this)->setReady();
   std::map< std::size_t, int > size_map;

   /**
    * make this a fixed quantity right now, if size > .75% at
    * montor interval three times or more then increase size.
    */

   auto mon_func = [&]( PortInfo &a, PortInfo &b, void *data ) -> void
   {
      UNUSED( data );
      /** 
       * return if fixed buffer specified for this link
       * fixed buffer is always taken from the source port
       * info struct or "a" in this case.
       */
      if( a.fixed_buffer_size != 0 )
      {
         /** skip this one....it came with a buffer, not ours to touch **/
         return;
      }
      const auto hash_val( dynalloc::hash( a, b ) );
      /** TODO, the values might wrap if no monitoring on **/
      auto * const buff_ptr( a.getFIFO() );

      const auto realized_ratio( buff_ptr->get_frac_write_blocked() );
      const auto ratio( 0.8 );
      const auto value( buff_ptr->get_suggested_count() );
      const auto cap( buff_ptr->capacity() );
      if( value > 0 && cap != value )
      {
         buff_ptr->resize( value, ALLOC_ALIGN_WIDTH, exit_alloc );
         size_map[ hash_val ] = 0;
      }
      else if( realized_ratio >= ratio )
      {
         const auto curr_count( size_map[ hash_val ]++ );
         if( curr_count  > 2 )
         {
            /** get initializer function **/
            buff_ptr->resize( cap * 2, ALLOC_ALIGN_WIDTH, exit_alloc );
            size_map[ hash_val ] = 0;
         }
      }
      return;
   };
   /** start monitor loop **/
   while( ! exit_alloc )
   {
      /** monitor fifo's **/
      std::chrono::microseconds dura( 3000 );
      std::this_thread::sleep_for( dura );

      auto &container( (this)->source_kernels.acquire() );
      GraphTools::BFT( container, mon_func, GraphTools::output );
      (this)->source_kernels.release();

   }
   return;
}
