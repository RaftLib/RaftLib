/**
 * poolschedule.cpp - 
 * @author: Jonathan Beard
 * @version: Thu Sep 11 15:49:57 2014
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
#include <cassert>
#include <functional>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include "kernel.hpp"
#include "map.hpp"
#include "poolschedule.hpp"
#include "rafttypes.hpp"

#include "partition.tcc"

pool_schedule::pool_schedule( Map &map ) : Schedule( map ),
                                     n_threads( std::thread::hardware_concurrency() ),
                                     pool( n_threads ),
                                     container( n_threads ),
                                     status_flags( n_threads )
{
   for( std::int64_t index( 0 ); index < n_threads; index++ )
   {
      status_flags[index] = false;
      container[index] = new KernelContainer();
      pool[ index ] = new std::thread( poolrun,
                                       container[ index ],
                                       std::ref(  status_flags[index] ) );
   }
}


pool_schedule::~pool_schedule()
{
   for( auto *th : pool )
   {
      delete( th );
   }
   for( auto *kc : container )
   {
      delete( kc );
   }
}

bool
pool_schedule::scheduleKernel( raft::kernel *kernel )
{
   assert( kernel != nullptr );
   kernel_map.emplace_back( kernel );
   return( true );
}

void
pool_schedule::start()
{
   {
      for( auto * const c : container )
      {
         c->lock();
      }     
      
      partition::simple( kernel_map,
                         container );
      
      for( auto * const c : container )
      {
         c->unlock();
      }     
   }

   auto is_done( []( std::vector< KernelContainer* > &containers ) -> bool
   {
      for( auto *c : containers )
      {
         if( c->size() != 0 )
         {
            return( false );
         }
      }
      return( true );
   } );

   while( ! is_done( container ) )
   {
      const std::chrono::milliseconds dura( 100 );
      std::this_thread::sleep_for( dura );
#if 0      
      for( auto * const c : container )
      {
         c->lock();
         c->clear();
      }     
      partition::simple( kernel_map,
                         container );
      for( auto * const c : container )
      {
         c->unlock();
      }
#endif
   }
   /** done **/
   for( auto &flag : status_flags )
   {
      flag = 1;
   }
   for( std::thread *thr : pool )
   {
      thr->join();
   }
}

void 
pool_schedule::poolrun( KernelContainer *container, volatile std::uint8_t &sched_done )
{
   while( sched_done == 0 )
   {
      std::vector< raft::kernel* > unschedule_list;
      container->lock();
      for( auto &kernel : *container )
      {
         bool done( false );
         Schedule::kernelRun( &kernel, done );
         if( done )
         {
            unschedule_list.emplace_back( &kernel );
         }
      }
      for( raft::kernel * const kernel : unschedule_list )
      {
         container->removeKernel( kernel );
         Schedule::inactivate( kernel );
      }
      container->unlock();
   }
}
